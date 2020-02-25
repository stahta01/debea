// File: csv.cpp
// Purpose: Archive implementation for csv file format
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2004
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include <algorithm>
#include <map>
#include <assert.h>
#include <string.h>

#include "dba/csv.h"
#include "dba/exception.h"
#include "dba/conversion.h"

namespace dba {

using namespace std;
using namespace dba;

/*============================ CSV Parser =========================*/

CSVParser::CSVParser(ifstream& pStream) 
  : mStream(pStream),
    mLine(0),
    mChar(0),
    mFieldSep(',')
{
};

char 
CSVParser::getFieldSeparator() const {
  return mFieldSep;
};

void
CSVParser::setFieldSeparator(char pSep) {
  mFieldSep = pSep;
};

const list<string>&
CSVParser::getValues() const {
  return mValues;
};

void 
CSVParser::onField(int pNum, string* pData) {
//  cerr << "adding " << *pData << " (" << pNum << ")" << endl;
  mValues.push_back(*pData);
  pData -> clear();
};

void 
CSVParser::onLine() {

};

void
CSVParser::prepareList() {
  mCurrent = mValues.begin();
  for(list<string>::iterator it = mValues.begin(); it != mValues.end(); it++) {
    it -> clear();
  };
};

bool
CSVParser::eof() {
  return mStream.eof();
};

bool
CSVParser::parseLine() throw (CSVParseException) {
  string value;
  
  mValues.clear();
  mState.clear();
  mState.push_back(SCAN);
  mState.push_back(FIELD);
  prepareList();
  int field = 0;
      
  char c;
  bool stop = false;
  mChar = 0;
  while(!stop) {
    mStream.get(c);
    if (mStream.bad())
      throw CSVFileException("error reading file");
    if (mStream.eof()) {
      //end of file
      if (mChar == 0)
        return false;
      switch(mState.back()) {
        case SCAN:
          onLine();
          return true;
        break;
        case FIELD:
          mState.pop_back();
          onField(field++,&value);
          return true;
        break;
        case QUOTED_FIELD:
          throw CSVParseException("unterminated quoted field");
        break;
        case QUOTE_TEST:
          mState.pop_back();
          mState.pop_back();
          mState.pop_back();
          onField(field++,&value);
          return true;
        break;
      };
    };
    switch (c) {
      case '"':
        switch (mState.back()) {
          case SCAN:
            throw CSVParseException("got quote char, expecting separator or newline");
          break;
          case FIELD:
            mState.push_back(QUOTED_FIELD);
          break;
          case QUOTED_FIELD:
            mState.push_back(QUOTE_TEST);
          break;
          case QUOTE_TEST:
            value += c;
            mState.pop_back();
          break;
        };
      break;
      case '\r':
        mChar--;
        switch(mState.back()) {
          case SCAN:
          case FIELD:
            //end of line is comming, wait for \n
          break;
          case QUOTED_FIELD:
            value += c;
          break;
          case QUOTE_TEST:
            //empty quoted field, end of line is comming
            mState.pop_back();
            mState.pop_back();
            mState.pop_back();
            onField(field++,&value);
          break;
        };    
      break;
      case '\n':
        switch(mState.back()) {
          case SCAN:
            //end of record
            onLine();
            stop = true;
          break;
          case FIELD:
            mState.pop_back();
            //empty line is perfectly valid
            if (mChar == 0) {
              //no fields here
            } else {
              onField(field++,&value);
            };
            onLine();
            stop = true;
          break;
          case QUOTED_FIELD:
            value += c;
          break;
          case QUOTE_TEST:
            //empty field, end of line
            mState.pop_back();
            mState.pop_back();
            mState.pop_back();
            onField(field++,&value);
            onLine();
            stop = true;
          break;
        };    
        mChar = 0;
        mLine++;
      break;
      case ' ':
        switch (mState.back()) {
          case SCAN:
          case FIELD:
          /*
            If space is a separator, then do not treat it as part of field  
          */
            if (c == mFieldSep) {
              processSeparator(field,value,c);
              break;
            };
          /*
            for compability with broken MS Excel CSV exproter - if 
            separation chars are not used then string with whitespaces are not in double quotes
          */
          case QUOTED_FIELD:
            value += c;
          break;
          case QUOTE_TEST:
            mState.pop_back();
            mState.pop_back();
            //if space is a separator then we
            //assume that next char (if not space) 
            //is a part of next field 
            //otherwise we ignore this space seeting state to SCAN
            //and wait for next character
            if (c != mFieldSep)
              mState.pop_back();
            onField(field++,&value);
          break;
        };
      break;
      default:
        if (c == mFieldSep) {
          processSeparator(field,value,c);
        } else {
          switch (mState.back()) {
            case SCAN:
              throw CSVParseException("got char, expecting separator or newline");       
            break;
            case FIELD:
            case QUOTED_FIELD:
              value += c;
            break;
            case QUOTE_TEST:
              mState.pop_back();
              mState.pop_back();
              mState.pop_back();
              onField(field++,&value);
            break;
          };
        };
      break;
    };
    mChar++;
  };
//  for(list<string>::const_iterator it = mValues.begin(); it != mValues.end(); it++) cerr << "pval:" << *it << endl;
  return true;
};

void
CSVParser::processSeparator(int& field, std::string& value, char c) {
  switch (mState.back()) {
    case SCAN:
      mState.push_back(FIELD);
    break;
    case FIELD:
      //end field
      onField(field++,&value);
    break;
    case QUOTED_FIELD:
      value += c;
    break;
    case QUOTE_TEST:
      //empty quoted field
      mState.pop_back();
      mState.pop_back();
      onField(field++,&value);
    break;
  };    
};

CSVParser::~CSVParser() {
};

/*============================ CSV Archive ========================*/

CSVArchive::CSVArchive() 
  : mHasColNames(true),
    mIgnoreEmptyLines(true),
    mFieldSeparator(',')
{

};

void 
CSVArchive::hasColumnNames(bool pFlag) {
  mHasColNames = pFlag;
};

bool 
CSVArchive::hasColumnNames() const {
  return mHasColNames;
};

void
CSVArchive::ignoreEmptyLines(bool pFlag) {
  mIgnoreEmptyLines = pFlag;
};

void 
CSVArchive::open(const char* pOpenStr)  {
  mFilename = pOpenStr;
  ifstream rs;
  rs.open(pOpenStr);
  if (!rs.is_open()) {
    ofstream ws;
    ws.open(pOpenStr);
    if (!ws.is_open()) {
      std::string err("Cannot create file ");
      err += pOpenStr;
      throw CSVFileException(err.c_str());	
    };
  };
};

void 
CSVArchive::setFieldSeparator(char pSep) {
  mFieldSeparator = pSep;
};

#ifdef DBA_COMPAT_1_0
CSVIStream* 
CSVArchive::getOIStream() {
  CSVIStream* stream = new CSVIStream(mFilename.c_str(), mMappings, mHasColNames, mIgnoreEmptyLines, mConvSpecs);
  stream->setFieldSeparator(mFieldSeparator);
  return stream;
};

CSVOStream* 
CSVArchive::getOOStream() {
  CSVOStream* stream = new CSVOStream(mFilename.c_str(), mMappings, mHasColNames, mConvSpecs);
  stream->setFieldSeparator(mFieldSeparator);
  return stream;
};

std::list<std::string> 
CSVArchive::getColumns() {
  CSVIStream stream(getIStream());
  return stream.getColumns();
};
#endif

IStream*
CSVArchive::getInputStream() {
  CSVIStream* stream = new CSVIStream(mFilename.c_str(), mMappings, mHasColNames, mIgnoreEmptyLines, mConvSpecs);
  stream->setFieldSeparator(mFieldSeparator);
  return stream;
};


CSVIStream 
CSVArchive::getIStream() {
  CSVIStream stream(mFilename.c_str(), mMappings, mHasColNames, mIgnoreEmptyLines, mConvSpecs);
  stream.setFieldSeparator(mFieldSeparator);
  return stream;
};

CSVOStream 
CSVArchive::getOStream() {
  CSVOStream stream(mFilename.c_str(), mMappings, mHasColNames, mConvSpecs);
  stream.setFieldSeparator(mFieldSeparator);
  return stream;
};


OStream*
CSVArchive::getOutputStream() {
  CSVOStream* stream = new CSVOStream(mFilename.c_str(), mMappings, mHasColNames, mConvSpecs);
  stream->setFieldSeparator(mFieldSeparator);
  return stream;
};

bool 
CSVArchive::isOpen() const {
  return true;
};

CSVArchive::~CSVArchive() {

};

/*========================= mapping helper utils ======================*/

class mappingMatcher {
  public:
    mappingMatcher(const string& pName) : mName(pName) {};
    bool operator() (const mapping& pMapping) const {
      if (pMapping.fname == NULL)
        return false;
      return (pMapping.fname == mName);
    };
  private:
    string mName;
};

class mappingTableMatcher {
  public:
    mappingTableMatcher(const char* pTable, const char* pField) : mTable(pTable), mField(pField) {};
    bool operator() (const mapping& pMapping) const {
      if (!strcmp(pMapping.table,mTable))
        if (!strcmp(pMapping.field,mField))
          return true;
      return false;
    };
  private:
    const char* mTable;
    const char* mField;
};

class mappingIndexMatcher {
  public:
    mappingIndexMatcher(int pIndex) : mIndex(pIndex) {}
    bool operator() (const mapping& pMapping) {
      return pMapping.fnumber == mIndex;
    };
  private:
    int mIndex;
};

int
getMappingPosByName(const list<mapping>& pMappings, const char* pTable, const char* pField) {
  list<mapping>::const_iterator it = find_if(pMappings.begin(), pMappings.end(), mappingTableMatcher(pTable, pField));
  if (it == pMappings.end())
    return -1;
//  if ((unsigned)it->fnumber > pMappings.size())
//    return -1;
  return it -> fnumber;
};

void
removeMappingByIndex(int pIndex,list<mapping>* pMappings) {
  list<mapping>::iterator it = find_if(pMappings->begin(), pMappings->end(), mappingIndexMatcher(pIndex));
  if (it == pMappings->end()) {
//    cerr << "removeMapping: mapping with index " << pIndex << " not found" << endl;
    return;
  };
  pMappings->erase(it); 
};

void
updateMappings(const list<string>& pColNames, list<mapping>* pMappings) {
  int col_index = 0;
  for(list<string>::const_iterator it = pColNames.begin(); it != pColNames.end(); it++) {
    for(list<mapping>::iterator mit = pMappings->begin(); mit != pMappings->end(); mit++) {
      if (mit->fnumber == -1) {
        if (mit->fname == *it) {
          mit->fnumber = col_index;
//          cerr << "updating mapping for member " << mit->fname << " table " << mit->table << " setting index "<< mit->fnumber <<  endl;
          break;
        };
      };
    };
    col_index++;
  };
};

/*============================ CSV IStream ===========================*/

CSVIStream::CSVIStream(const char* pFilename, list<mapping>& pMappings, bool pHasColNames, bool pIgnoreEmptyLines, const ConvSpec& pSpecs)
  : IStream(),
    ConvSpecContainer(pSpecs),
    mMappings(pMappings),
    mRowFetched(false),
    mIgnoreEmptyLines(pIgnoreEmptyLines),
    mFilename(pFilename),
    mParser(NULL),
    mStream(new std::ifstream()),
    mHasColNames(pHasColNames)
{
};

mapping::mapping(int pCSVField, const char* pTable, const char* pField) 
  : fnumber(pCSVField),
    fname(NULL),
    table(pTable == NULL ? NULL : strdup(pTable)),
    field(pField == NULL ? NULL : strdup(pField))
{
};

mapping::mapping(const char* pCSVField, const char* pTable, const char* pField) 
  : fnumber(-1),
    fname(pCSVField == NULL ? NULL : strdup(pCSVField)),
    table(pTable == NULL ? NULL : strdup(pTable)),
    field(pField == NULL ? NULL : strdup(pField))
{
};

mapping::mapping(const mapping& pMapping) 
  : fnumber(pMapping.fnumber)
{
  fname = pMapping.fname == NULL ? NULL : strdup(pMapping.fname);
  table = pMapping.table == NULL ? NULL : strdup(pMapping.table);
  field = pMapping.field == NULL ? NULL : strdup(pMapping.field);
};
    
mapping& 
mapping::operator=(const mapping& pMapping) {
  if (this != &pMapping) {
    if (fname != NULL) free(fname);
    if (table != NULL) free(table);
    if (field != NULL) free(field);
    fname = pMapping.fname == NULL ? NULL : strdup(pMapping.fname);
    table = pMapping.table == NULL ? NULL : strdup(pMapping.table);
    field = pMapping.field == NULL ? NULL : strdup(pMapping.field);
    fnumber = pMapping.fnumber;
  };
  return *this;
};


mapping::~mapping() {
  free(fname);  
  free(table);
  free(field);
};

bool 
mapping::operator<(const mapping& pMapping) const {
  return fnumber < pMapping.fnumber;
};


void
CSVArchive::addMapping(int pCSVField, const char* pTable, const char* pField) {
  mapping m(pCSVField,pTable,pField);
  mMappings.push_back(m);
};

void 
CSVArchive::clearMappings() {
  mMappings.clear();
};


void
CSVArchive::addMapping(const char* pCSVField, const char* pTable, const char* pField) {
  if (!mHasColNames)
    throw APIException("Cannot add named mapping to archive without column names");
  mapping m(pCSVField,pTable,pField);
  mMappings.push_back(m);
};

void
CSVIStream::createMappings(Storeable& pObject) {
  const StoreTable* tbl = Stream::getTable(pObject);
  const char* cur_table_name = tbl->getTableName();
  if (cur_table_name == NULL)
    throw APIException("root table has no name");
  while(tbl != NULL) {
    if (tbl->getTableName() != NULL)
      cur_table_name = tbl->getTableName();
    StoreTableMember* member = tbl->getMembers();
    while (member != NULL) {
      mapping m(member->getMemberName(),cur_table_name,member->getMemberName());
      mMappings.push_back(m);
      member = member->getNextMember();
    };
    tbl = tbl->getNextTable();
  };
  for(VarMap::iterator it = mBindings.begin(); it != mBindings.end(); it++) {
    mapping m(it->mField,it->mTable,it->mField);  
//    cerr << "creating named mapping for binded var " << it->mField << " table " << it->mTable << endl;
    mMappings.push_back(m);
  };  
};

const list<string>& 
CSVIStream::getColumns() {
  //parse first line of csv file if file is not already open
  if (!mStream->is_open()) {
    openFile();
    parseColumns();
  };
  return mColNames;
};

void
CSVIStream::open(Storeable& pObject, const char* pTable) {
  IStream::open(pObject,pTable);
  
  if (!mStream->is_open()) {
    openFile();
  };
  //discover column names
  if (mHasColNames && mColNames.empty()) {
    parseColumns();
  };
  //map column names (or index values) to object store table
  if (mMappings.size() == 0) {
    createMappings(pObject);
  };
  updateMappings(mColNames,&mMappings);
};

void
CSVIStream::openFile() {
  mStream->open(mFilename.c_str());
  if (!mStream->is_open()) {
    std::string exdata("cannot open file ");
    exdata += mFilename;
    throw CSVFileException(exdata.c_str());
  };
  mParser = new CSVParser(*mStream);
  mParser->setFieldSeparator(mFieldSep);
};

void
CSVIStream::parseColumns() {
  if (!mParser->parseLine()) {
    throw DataException("No column data in file");
  } else {
    mColNames = mParser->getValues();
  };
};

void 
CSVIStream::setFieldSeparator(char pSep) {
  mFieldSep = pSep;
};


bool
CSVIStream::fetchRow() { 
  if (!mParser->parseLine())
    return false;
  if (mIgnoreEmptyLines) {
    while(mParser->getValues().size() == 0) {
//      cerr << "ignoring empty line" << endl;
      if (!mParser->parseLine())
        return false;
    };
  };
  mValues = mParser->getValues();
  return true;
};

bool 
CSVIStream::getNext(Storeable* pObject) {
  if (!isOpen()) {
    throw APIException("csv istream not open");
  };
  if (!mRowFetched)
    if (!fetchRow())
      return false;
  mRowFetched = false;
//  for(list<string>::const_iterator it = mValues.begin(); it != mValues.end(); it++) cerr << "val:" << *it << endl;
  const StoreTable* tbl = Stream::getTable(*pObject);
  if (tbl != NULL) {
    std::string data;
    const char* current_table_name = tbl->getTableName();
    if (current_table_name == NULL)
      throw APIException("Root table has no name");
    while(tbl != NULL) {
      if (tbl->getTableName() != NULL)
        current_table_name = tbl->getTableName();
      StoreTableMember* member = tbl->getMembers();
      while (member != NULL) {
  //      cerr << "setting member " << member -> name << " binded to " << member << endl;
        unsigned int pos = getMappingPosByName(mMappings, current_table_name, member->getMemberName());
        dba::StoreableFilterBase* filter(member->getFilter());
        filter->updateRef((char*)pObject + (int)(member->getMemberOffset() + tbl->getClassOffset()));
        if (pos != (unsigned)-1) {
          //we got mapping for column but we cannot find 
          //that column data in csv file
          if (mValues.size() <= pos) {
            string s;
            s += "Column " + dba::toStr(pos) + " for mapping " +  current_table_name + "." + member->getMemberName() + " not found ";
            throw DataException(s);
          };
          data = getValueByIndex(mValues, pos);
          filter->fromString(mConvSpecs, data);
        } else {
          data.erase();
          filter->fromNull();
        };
        member = member->getNextMember();
      };
      tbl = tbl->getNextTable();
    };
    list<mapping>::iterator mit = find_if(mMappings.begin(), mMappings.end(), mappingMatcher("id"));
    if (mit != mMappings.end()) {
      if (mit -> fnumber != -1) {
        data = getValueByIndex(mValues, mit -> fnumber);
        int id;
        dba::convert(data,id);
        Stream::alterId(pObject,id);
        Stream::makeOk(pObject);
      };
    };
  };
  fillBindedVars();
  //update stored tables count in Storeable class
  Stream::setStoredTables(pObject,Stream::countTables(pObject));
  return true;
};

bool
CSVIStream::updateVars() {
  if (!mRowFetched)
    if (!fetchRow())
      return false;
  mRowFetched = true;
  fillBindedVars();
  return true;
};

void
CSVIStream::fillBindedVars() {
  std::string data;
  list<mapping> m(mMappings);
  for(VarMap::iterator it = mBindings.begin(); it != mBindings.end(); it++) {
    int pos = getMappingPosByName(m, it->mTable, it->mField);
    if (pos != -1) {
      data = getValueByIndex(mValues, pos);
//      cerr << "removing mapping for member " << it->mField << " table " << it->mTable << " index "<< pos << " from local mapping (size " << m.size() << ")" <<  endl;
      removeMappingByIndex(pos,&m);
//        cerr << *data << endl;
    } else {
      data.erase();
    };
    dba::StoreableFilterBase& filter(*(it->mFilter));
    filter.fromString(mConvSpecs, data);
  };  
};

void
CSVIStream::close() {
  mParser = NULL;
  mStream->close();
};

std::string
CSVIStream::getValueByIndex(const list<std::string>& pValue, int pPos) {
  list<string>::const_iterator it = pValue.begin();
  int i = 0;
  while(it != pValue.end()) {
    if (i == pPos)
      return *it;
    i++;
    it++;
  };
  std::string err("Field index");
  err += pPos;
  err += " is out of bounds";
  throw DataException(err.c_str());
  //unreachable code
  return std::string();
};

void
CSVIStream::destroy() {
  mStream = NULL;
  mParser = NULL;
};

CSVIStream::~CSVIStream() {
};

/*========================= csv object output stream ======================*/
    
CSVOStream::CSVOStream(const char* pFilename, list<mapping>& pMappings, bool pHasColNames, const ConvSpec& pSpecs)
  : OStream(),
    ConvSpecContainer(pSpecs),
    mMappings(pMappings),
    mHasColNames(pHasColNames),
    mStream(new std::ofstream)
{
  mStream->open(pFilename);
  mNoWriteYet = true;
};

void 
CSVOStream::open(const char* pRootTable) {
  OStream::open(pRootTable);
};

void 
CSVOStream::close() {
  mStream->close();
  mIsOpen = false;
};

bool 
CSVOStream::erase(Storeable* pObject) {
  //not supported
  assert(false);
  return false;
};

void 
CSVOStream::setFieldSeparator(char pSep) {
  mFieldSep = pSep;
};

bool 
CSVOStream::update(Storeable* pObject) {
  //not supported
  assert(false);
  return false;
};

void
CSVOStream::createMappings() {
  int index = 0;
  mt_class* current = mMemberList->mFirst;
  while(current->name != NULL) {
    mt_member* member = current->firstField;
    while(member->name != NULL) {
      mapping m(index++,current->name,member->name);
      m.fname = strdup(member->name);
      mMappings.push_back(m);
      member = member->next;
      if (member == NULL)
        break;
    };
    
    VarMap bmap(mBindings);
    //binded vars in object tables
    VarMap::iterator it = bmap.begin();
    while(it != bmap.end()) {
      if (!strcmp(it->mTable,current->name)) {
        mapping m(index++,it->mTable,it->mField);
        m.fname = strdup(it->mField);
        mMappings.push_back(m);     
      };
      it++;
    };
    
    current = current->next;
    if (current == NULL)
      break;
  };  
};

void
CSVOStream::writeColumns() {
  int index = 0;
  string csvline;
  mMappings.sort();
  for(list<mapping>::iterator it = mMappings.begin(); it != mMappings.end(); it++) {
    if (it != mMappings.begin())
      csvline += mFieldSep;
    string data(it->fname);
    data = escapeString(data);
    csvline += data;
    if (it->fnumber == -1) {
      it->fnumber = index;
    } else {
      if (it->fnumber != index) {
        string s("missing mapping for column ");
        s += toStr(index);
        throw APIException(s);
      };      
    };
    index++;
  };
  csvline += '\n';
  mStream->write(csvline.c_str(),csvline.size());
  if (!mStream->good())
    throw CSVFileException("write to file failed");
};

bool 
CSVOStream::store(Storeable* pObject) {
  std::map<int,string> outmap;
  createTree(Stream::getTable(*pObject));
  if (mNoWriteYet) {
    if (mHasColNames) {
      if (mMappings.size() == 0)
        createMappings();
      writeColumns();
    };
    mNoWriteYet = false;
  };
  mt_class* current = mMemberList->mFirst;
  while(current -> name != NULL) {
    mt_member* member = current -> firstField;
    while(member -> name != NULL) {
//        (*((st_to_db)member -> func))((char*)pObject + (int)(member -> offset),&data,Database::STRING,true);
      //std::string data(applyFilter((st_to_db)member->func,(char*)pObject + (int)(member -> offset),(Database::StoreType)member->type));
      dba::StoreableFilterBase* filter = (StoreableFilterBase*)member->func;
      filter->updateRef((char*)pObject + (int)(member -> offset));
      std::string data;
      if (!filter->isNull())
        data = filter->toString(mConvSpecs);
      int pos = getMappingPosByName(mMappings,current->name, member->name);
      if (pos != -1) {
        outmap[pos] = data;
      };
    
      member = member -> next;
      if (member == NULL)
        break;
    };

    VarMap bmap(mBindings);
    list<mapping> m(mMappings);
    //binded vars in object tables
    for (VarMap::iterator it = bmap.begin(); it != bmap.end(); it++) {
      std::string data;
      if (!strcmp(it->mTable,current -> name)) {
        dba::StoreableFilterBase& filter = *(it->mFilter);
        data = filter.toString(mConvSpecs);
      };

      int pos = getMappingPosByName(m,it->mTable,it->mField);
      if (pos != -1) {
        removeMappingByIndex(pos,&m);
        outmap[pos] = data;
      };
    };
  
    current = current -> next;
    if (current == NULL)
      break;
  };
  string csvline;
  int index = 0;
  //got data on outlist, make line and save it
  for(map<int,string>::iterator it = outmap.begin(); it != outmap.end(); it++) {
    if (it != outmap.begin())
      csvline += mFieldSep;
    csvline += escapeString(it->second);
    //fill holes in mappings
    while(index < it->first) {
      csvline += mFieldSep;
      index++;
    };
    index++;
  };
  csvline += '\n';
  mStream->write(csvline.c_str(),csvline.size());
  if (!mStream->good())
    throw CSVFileException("write to file failed");
  return true;
};

string
CSVOStream::escapeString(const string& pInput) {
  if (pInput.empty())
    return pInput;
  string s(pInput);
  int i = replaceAll(s,"\"","\"\"");
  if (i == 0)
    if (!isspace((int)(unsigned char)pInput[0]))
      if (!isspace((int)(unsigned char)pInput[pInput.size()-1]))
        if (pInput.find("\n") == string::npos)
          if (pInput.find("\r") == string::npos)
            if (pInput.find(mFieldSep) == string::npos)
              return s;
  s.insert(0,"\"");
  s.append("\"");
  return s;
};

void
CSVOStream::destroy() {
  mStream = NULL;
};


};
