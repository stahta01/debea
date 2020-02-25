// File: sqlostream.cpp
// Purpose: Object output stream for SQL databases
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2003
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#include <sstream>
#include <string.h>

#include "dba/sqlostream.h"
#include "dba/sqlutils.h"
#include "dba/exception.h"
#include "dba/storeablefilter.h"
#include "dba/sqlidfetcher.h"

namespace dba {

using namespace std;

std::string
SQLOStream::applyFilter(StoreableFilterBase& pFilter, Database::StoreType pType) {
  if (pFilter.isNull())
    return "NULL";
  std::string strdata(pFilter.toString(getConversionSpecs()));
  switch(pType) {
    case Database::STRING:
      strdata = SQLUtils::setSQLVal(strdata);
    break;
    case Database::FLOAT:
    break;
    case Database::INTEGER:
    break;
    case Database::DATE:
      strdata = SQLUtils::setSQLVal(strdata);
    break;
  };
  return strdata;
};

string
SQLOStream::createInsert(int id, const Storeable& pObject, mt_class* pTable) {
  const char* table = pTable->name;
  if (table == NULL) {
    if (mCurrentTable.size() == 0)
      throw APIException("Table name not set. Pass table name to open() or set it in STORE_TABLE");
    table = mCurrentTable.c_str();
  } else {
    mCurrentTable = table;
  };
  string query = "INSERT INTO " + string(table) + " (id";
  mt_member* current = pTable->firstField;
  //object fields
  while(current->name != NULL) {
//    cerr << current->name << endl;
    query += "," + string(current->name);
    current = current->next;
    if (current == NULL)
      break;
  };
  //binded vars
  for(VarMap::iterator it = mBindings.begin(); it != mBindings.end(); it++) {
    if (it->mTable == string(table)) {
      query += "," + string(it->mField);
    };
  };
  query += ") VALUES (" + toStr(id);
  current = pTable->firstField;
  while(current->name != NULL) {
    StoreableFilterBase* filter = (StoreableFilterBase*)current->func;
    filter->updateRef((char*)&pObject + (int)(current->offset));
    query += "," + applyFilter(*filter,(Database::StoreType)current->type);
    current = current->next;
    if (current == NULL)
      break;
  };
  for(VarMap::iterator it = mBindings.begin(); it != mBindings.end(); it++) {
    if (it->mTable == string(table)) {
      //query += "," + applyFilter(it->mFilter,it->mData,it->mType);
      query += "," + applyFilter(*(it->mFilter),it->mType);
    };
  };
  query += ")";
  return query;
};

std::vector<id>
SQLOStream::loadRefData(const char* pTable, const char* pFkName, id pCollId, id pId) {
  std::vector<id> ret;
  std::string query("SELECT id");
  query += " FROM ";
  query += pTable;
  query += " WHERE ";
  query += pFkName;
  query += "=" + toStr(pId);
  if (pCollId != Storeable::InvalidId) {
    query += " AND dba_coll_id=";
    query += toStr(pCollId);
  };
  std::auto_ptr<DbResult> res(mConn->sendQuery(query.c_str()));
  while(res->fetchRow()) {
    if (res->isNull(0)) {
      std::string error("NULL id of object in table ");
      error += pTable;
      throw APIException(error);
    };
    ret.push_back(res->getInt(0));
  };
  return ret;
};

bool
SQLOStream::deleteRefData(const std::vector<id>& pIds, const char* pTableName) {
  SQL query("DELETE FROM ");
  query += pTableName;
  query += " WHERE id IN ";
  query += SQLUtils::createIN(pIds);
  int recs = sendUpdate(query);
  return recs != 0;
};

void
SQLOStream::begin() {
  mConn->begin();
};

void
SQLOStream::commit() {
  mConn->commit();
};

void
SQLOStream::rollback() {
  mConn->rollback();
};

void
SQLOStream::close() {
  mIsOpen = false;
  mCurrentTable.clear();
};

void
SQLOStream::destroy() {
  if (isOpen())
    close();
  if (mConn != NULL) {
    mConn->decUsed();
    mConn = NULL;
  };
};

bool
SQLOStream::isOpen() const {
  return mIsOpen;
};


SQLOStream::SQLOStream(DbConnection* pConn,SQLIdFetcher* pFetcher, FilterMapper* pFilterMapper)
  : OStream(),
    mConn(pConn),
    mFetcher(pFetcher),
    mFilterMapper(pFilterMapper)
{
  mConn->incUsed();
  mIsOpen = false;
};

SQLOStream::SQLOStream(const SQLOStream& pStream)
  : OStream(pStream),
    mConn(pStream.mConn),
    mFetcher(pStream.mFetcher),
    mFilterMapper(pStream.mFilterMapper),
    mCurrentTable(pStream.mCurrentTable)
{
  mConn->incUsed();
};

SQLOStream&
SQLOStream::operator=(const SQLOStream& pStream) {
  OStream::operator=(pStream);
  mConn = pStream.mConn;
  mFetcher = pStream.mFetcher;
  mFilterMapper = pStream.mFilterMapper;
  mCurrentTable = pStream.mCurrentTable;
  mConn->incUsed();
  return *this;
};

void
SQLOStream::open(const char *const pMainTable) {
  if (pMainTable != NULL)
    mCurrentTable = pMainTable;
  OStream::open(pMainTable);
};

string
SQLOStream::createUpdate(const Storeable& pObject, mt_class* pTable) {
  const char* table = pTable->name;
  if (table == NULL) {
    if (mCurrentTable.size() == 0)
      throw APIException("Table name not set. Pass table name to open() or set it in STORE_TABLE");
    table = mCurrentTable.c_str();
  } else {
    mCurrentTable = table;
  };
  string query = "UPDATE " + string(table) + string(" SET ");
  mt_member* current = pTable->firstField;
  int i = 0;
  while(current->name != NULL) {
    if (i != 0)
      query += ",";
    query += string(current->name) + "=";
    StoreableFilterBase* filter = (StoreableFilterBase*)current->func;
    filter->updateRef((char*)&pObject + (int)(current->offset));
    query += applyFilter(*filter,(Database::StoreType)current->type);
    i++;
    current = current->next;
    if (current == NULL)
      break;
  };

  VarMap bmap(mBindings);
  //binded vars in object tables
  for (VarMap::iterator it = bmap.begin(); it != bmap.end(); it++) {
    if (!strcmp(it->mTable,table)) {
      if (i != 0)
        query += ",";
      query += string(it->mField) + "=";
      query += applyFilter(*(it->mFilter),it->mType);
      i++;
    };
  };
  query += string(" WHERE id = ") + toStr(pObject.getId());
  return query;
};

int
SQLOStream::sendUpdate(const SQL& pCommand) {
  return mConn->sendUpdate(pCommand.cstring(*mFilterMapper,getConversionSpecs()));
};

bool
SQLOStream::update(Storeable* pObject) {
  string query;
  string data;
  VarMap bmap(mBindings);
  createTree(Stream::getTable(*pObject));
  int storedTables = Stream::getStoredTables(pObject);
  int affectedTables = 0;
  mt_class* current = mMemberList->mFirst;
  while(current->name != NULL) {
    if (storedTables != 0)
      query = createUpdate(*pObject,current);
    else
      query = createInsert(pObject->getId(),*pObject,current);
    int affectedRows = mConn->sendUpdate(query);
    //if we are doing update then make sure that
    //object was really updated in database
    //It is possible that sendUpdate return 0 rows
    //if stream was open for different table and stored object
    //when object was inserted into database
    if (storedTables != 0 && affectedRows == 0) {
      std::stringstream error;
      error << "Failed to update object id=" << pObject->getId() <<
      " data in sql table " << current->name << ": SQL UPDATE returned 0 rows affected";
      throw DataException(error.str());
    };
    current = current->next;
    storedTables--;
    affectedTables++;
    if (current == NULL)
      break;
  };
  Stream::setStoredTables(pObject,affectedTables);
  return true;
};


bool
SQLOStream::store(Storeable* pObject) {
  string query;
  //create query tree
  createTree(Stream::getTable(*pObject));
  //nothing is binded
  if (mMemberList->mFirst->name == NULL)
    return false;
  //assign new id
  int id = mFetcher->getNextId(*mConn,Stream::getRootTableName(*pObject));
  //build and execute queries
  int storedTables = 0;
  mt_class* current = mMemberList->mFirst;
  while(current->name != NULL) {
    query = createInsert(id,*pObject,current);
    mConn->sendUpdate(query);
    current = current->next;
    storedTables++;
    if (current == NULL)
      break;
  };
  Stream::alterId(pObject,id);
  Stream::setStoredTables(pObject,storedTables);
  return true;
};

bool
SQLOStream::erase(Storeable* pObject) {
  string query;
  const StoreTable* tbl = Stream::getTable(*pObject);
  if (tbl == NULL)
    return false;

  //first check if we are overriding root table name
  const char* table = tbl->getTableName();
  if (mRootTable != NULL) {
    table = mRootTable;
  };

  //remove all records from all non-NULL tables in loop
  do {
    if (table != NULL) {
      query = string("DELETE FROM " + string(table) + " WHERE id = " + toStr(pObject->getId()));
      mConn->sendUpdate(query);
    };
    tbl = tbl->getNextTable();
    if (tbl != NULL)
      table = tbl->getTableName();
  } while(tbl != NULL);

  return true;
};

void
SQLOStream::assignId(Storeable* pObject) throw (Exception) {
  if (pObject->isNew()) {
    createTree(Stream::getTable(*pObject));
    pObject->setId(mFetcher->getNextId(*mConn,Stream::getRootTableName(*pObject)));
  }
};

const ConvSpec&
SQLOStream::getConversionSpecs() const {
  return mConn->getConversionSpecs();
};

SQLOStream::~SQLOStream() {
  try {
    destroy();
  } catch(...) {
    cerr << "Object Output Stream (SQL) EXCEPTION: cannot close stream" << endl;
  };
};

};//namespace


