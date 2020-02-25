// File: sqlistream.cpp
// Purpose: Object input stream for SQL databases
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2003
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include <string.h>

#include "dba/sqlistream.h"
#include "dba/sqlutils.h"
#include "dba/exception.h"
#include "dba/storeablefilter.h"

namespace dba {

using namespace std;

string
createSelectFields(mt_class* pFirst) {
  string data;
  string table;
  mt_class* current = pFirst;
  if (current->name != NULL)
    data += string(current->name) + ".id as id";
  while(current->name != NULL) {
    table = string(current->name) + ".";
    mt_member* field = current->firstField;
    while(field->name != NULL) {
      data += ",";
      data += table + string(field->name) + " as " + field->name;
      field = field->next;
      if (field == NULL)
        break;
    };
    current = current->next;
    if (current == NULL)
      break;
  };
  //hack for proper comma with empty store table and some binded vars
  return data;
};


string
createJoins(mt_class* pFirst) {
  mt_class* current = pFirst;
  string data;
  if (current->name == NULL)
    return data;
  data = current->name;
  while(current->name != NULL) {
    std::string left_table = current->name;
    current = current->next;
    if (current == NULL)
      break;
    if (current->name == NULL)
      break;
    data += string(" INNER JOIN ") + string(current->name);
    data += " ON " + left_table + ".id = " + string(current->name) + ".id";
  };
  return data;
};


string
createSelect(int id, mt_class* pFirst) {
  string query = "SELECT ";
//  cerr << "creating select fields " << endl;
  query += createSelectFields(pFirst);
  query += " FROM ";
//  cerr << "creating joins " << endl;
  query += createJoins(pFirst);
  query += " WHERE ";
  query += string(pFirst->name) + ".id = " + toStr(id);
  return query;
};

SQLIStream::SQLIStream(DbConnection* pConn, FilterMapper* pMapper)
  : IStream(),
    mConn(pConn),
    mFilterMapper(pMapper)
{
  mConn->incUsed();
  mRowFetched = false;
  mWhereSet = WHERE_NOT_SET;
};

SQLIStream::SQLIStream(const SQLIStream& pStream)
  : IStream(pStream),
    mResult(pStream.mResult),
    mConn(pStream.mConn),
    mFilterMapper(pStream.mFilterMapper),
    mWherePart(pStream.mWherePart),
    mFromPart(pStream.mFromPart),
    mQuery(pStream.mQuery),
    mRowFetched(pStream.mRowFetched),
    mWhereSet(pStream.mWhereSet)
{
  mConn->incUsed();
};

SQLIStream& 
SQLIStream::operator=(const SQLIStream& pStream) {
  IStream::operator=(pStream);
  mResult = pStream.mResult;
  mConn = pStream.mConn;
  mFilterMapper = pStream.mFilterMapper;
  mWherePart = pStream.mWherePart;
  mFromPart = pStream.mFromPart;
  mQuery = pStream.mQuery;
  mRowFetched = pStream.mRowFetched;
  mWhereSet = pStream.mWhereSet;

  mConn->incUsed();
  return *this;
};


void
SQLIStream::open(Storeable& pClass, const char* pMainTable) {
  if (mWhereSet == WHERE_NOT_SET)
    mWherePart = string();
  IStream::open(pClass,pMainTable);
  mResult = NULL;
  createSelect();
  doQuery();
  mWhereSet = WHERE_NOT_SET;
};

void
SQLIStream::openFromQuery(Storeable& pClass, const SQL& pQuery, const char* pMainTable) {
  if (mWhereSet == WHERE_NOT_SET)
    mWherePart = string();
  IStream::open(pClass, pMainTable);
  mResult = NULL;
  mQuery = pQuery.cstring(*mFilterMapper,getConversionSpecs());
  doQuery();
  mWhereSet = WHERE_NOT_SET;
};

void
SQLIStream::close() {
  mResult = NULL;
  mIsOpen = false;
};

void
SQLIStream::setWherePart(const SQL& pQueryPart) {
  setWhere(pQueryPart);
};

void
SQLIStream::setWhereId(int pId) {
  setWhere(SQL("id = :d") << pId);
  mWhereSet = WHERE_ID;
};

void
SQLIStream::setWhere(const SQL& pQueryPart) {
  mWherePart = pQueryPart.cstring(*mFilterMapper,getConversionSpecs());
  mWhereSet = WHERE_CUSTOM;
};

string
SQLIStream::createVarSelectFields() const {
  string query;
  VarMap::const_iterator it = mBindings.begin();
  while(it != mBindings.end()) {
    //check if requested field is was not previously parsed bo createSelectFields()
    //and it is not named "id" which is special
    if ((mMemberList->findMember(it->mTable,it->mField) == NULL)
      && (strcmp(it->mField,"id"))) 
    {
      if (it != mBindings.begin())
        query += ",";
      query = query + it->mTable + "." + it->mField + " as " + it->mField;
    };
    it++;
  };
  return query;
};

void
SQLIStream::createSelect() {
  //No tables!
  if (mMemberList->mFirst->name == NULL)
    throw APIException("No store tables for class");
  mQuery = "SELECT ";
  string sel = createSelectFields(mMemberList->mFirst);
  string var = createVarSelectFields();
  if (sel.length()) {
    mQuery += sel;
    if (var.length())
      mQuery += "," + var;
  };
  
  mQuery += " FROM ";
//  cerr << "creating joins " << endl;
  mQuery += createJoins(mMemberList->mFirst);
  mQuery += mFromPart;
  if (mWhereSet != WHERE_NOT_SET) {
    if (mWhereSet == WHERE_ID) {
      mWherePart = mMemberList->mFirst->name + string(".") + mWherePart;
    };
    mQuery += + " WHERE " + mWherePart;
  };
};

void
SQLIStream::doQuery() {
  mResult = mConn->sendQuery(mQuery);
  mIsOpen = true;
  mRowFetched = false;
};

bool
SQLIStream::updateVars() {
  if (!mResult->fetchRow())
    return false;
  mRowFetched = true;
  fillBindedVars();
  return true;
};

void
SQLIStream::applyFilter(DbResult& pRes, StoreableFilterBase& pFilter, const char* pFieldName, Database::StoreType pFieldType) {
  if (!pRes.isNull(pFieldName)) {
    switch(pFieldType) {
      case Database::STRING:
        pFilter.fromString(getConversionSpecs(), pRes.getString(pFieldName));
      break;
      case Database::INTEGER:
        pFilter.fromInt(getConversionSpecs(), pRes.getInt(pFieldName));
      break;
      case Database::FLOAT:
        pFilter.fromDouble(getConversionSpecs(), pRes.getDouble(pFieldName));
      break;
      case Database::DATE:
        pFilter.fromDate(getConversionSpecs(), pRes.getDate(pFieldName));
      break;
      default:
        throw StoreableFilterException("Internal error: unknown DbResult value type when fetching db record");
      break;
    };      
  } else {
    pFilter.fromNull();
  };
};

bool
SQLIStream::getNext(Storeable* pObject) {
  const StoreTable* tbl = Stream::getTable(*pObject);
  string sdata;
  void *data;
  //object data
  if (!mRowFetched)
    if (!mResult->fetchRow())
      return false;
  mRowFetched = false;
  while(tbl != NULL) {
    StoreTableMember* member = tbl->getMembers();
    while (member != NULL) {
//      cerr << "setting member binded to " << member << endl;
      StoreableFilterBase* filter = member->getFilter();
      filter->updateRef((char*)pObject + (int)(member->getMemberOffset() + tbl->getClassOffset()));
      applyFilter(*mResult,*filter,member->getMemberName(),(Database::StoreType)member->getDatabaseType());
      member = member->getNextMember();
    };
    tbl = tbl->getNextTable();
  };
  fillBindedVars();
  //sometimes id can not exist or can be null - can happen when loading data from views
  if (!strcmp(mResult->getColumn(0).getName(),"id")) {
    if (!mResult->isNull("id")) {
      Stream::alterId(pObject,mResult->getInt("id"));
      Stream::makeOk(pObject);
    };
  };
  //update stored tables count in Storeable class
  Stream::setStoredTables(pObject,Stream::countTables(pObject));
  return true;
};

void
SQLIStream::fillBindedVars() {
  for(VarMap::iterator it = mBindings.begin(); it != mBindings.end(); it++) {
    applyFilter(*mResult,*(it->mFilter),it->mField,it->mType);
  };  
};

void 
SQLIStream::setIdsCondition(const char* pFKeyName, id pRelationId, const std::vector<id>& pIds) {
  SQL where;
  where += pFKeyName;
  where += " IN ";
  where += SQLUtils::createIN(pIds);
  if (pRelationId != Storeable::InvalidId) {
    where += " AND dba_coll_id=";
    where += toStr(pRelationId);
  };
  where += " ORDER BY ";
  where += pFKeyName;
  setWherePart(where);
}

DbResult*
SQLIStream::sendQuery(const SQL& pQuery) const {
  DbResult* res = mConn->sendQuery(pQuery.cstring(*mFilterMapper,getConversionSpecs()));
  return new SQLDbResult(res,*mFilterMapper,pQuery);
};

bool 
SQLIStream::SQLDbResult::fetchRow() {
  bool ret = mResult->fetchRow();
  if (ret) {
    mQuery.updateVars(*mResult,mFilterMapper,mResult->getConversionSpecs());
  }
  return ret;
}


const std::string&
SQLIStream::getQuery() const {
  return mQuery;  
};

void
SQLIStream::destroy() {
  if (isOpen())
    close();
  if (mConn != NULL) {
    mConn->decUsed();
    mConn = NULL;
  };
};

const ConvSpec& 
SQLIStream::getConversionSpecs() const {
  return mConn->getConversionSpecs();
};

SQLIStream::SQLDbResult::SQLDbResult(DbResult* pRes, const FilterMapper& pMapper, const SQL& pSQL)
  : mResult(pRes),
    mFilterMapper(pMapper),
    mQuery(pSQL)
{ 
  setConversionSpecs(mResult->getConversionSpecs()); 
}


SQLIStream::~SQLIStream() {
//  cerr << this << "destructor, IStream: releasing connection" << endl;
  try {
    destroy();
  } catch(...) {
    cerr << "Object Output Stream (SQL) EXCEPTION: cannot close stream" << endl;    
  };
};

};//namespace


