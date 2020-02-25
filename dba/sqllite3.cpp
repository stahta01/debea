// File: sqlite3.cpp
// Purpose: Database plugin for sqlite3 database
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2004
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include <algorithm>
#include <memory>
#include <string.h>
#include <cstdio>

#include "dba/sqllite3.h"
#include "dba/connectstringparser.h"
#include "dba/conversion.h"

namespace sqllite3 {

using namespace std;
using namespace dba;

Db::Db()
 : Database()
{
}

DbConnection*
Db::getConnection(const char* pParams) {
  SLConnection* conn = new SLConnection(this,pParams,mCollations);
//  cerr << "created SLConn " << conn << endl;
  return conn;
};

void 
Db::addCollation(const char* pName, collationFunc pFunc) {
  mCollations.insert(make_pair(pName,pFunc));
};

Db::~Db()
{
}

void
SLConnection::begin() {
  sendUpdate("BEGIN");
};

void
SLConnection::commit() {
  sendUpdate("COMMIT");
};

void
SLConnection::rollback() {
  sendUpdate("ROLLBACK");
};


void
SLConnection::disconnect() {
  mConnHandle.reset();
};

bool
SLConnection::isValid() const {
  return (mConnHandle != NULL);
};



SLConnection::SLConnection(Db* pOwner, const char* pParams, const map<const char*,collationFunc>& pCols) {
  setConversionSpecs(pOwner->getConversionSpecs());
  setParentErrorHandler(pOwner);
  if (pParams == NULL) {
    handleError(DBA_DB_ERROR,"database filename is NULL");
    return;
  };
  dba::ConnectStringParser parser(pParams);
  try {
    std::map<std::string,std::string> parsed(parser.parse());
    mFileName = parsed["dbname"];
  } catch (const dba::ConnectStringParserException& pEx) {
    mFileName = pParams;    
  };
  
  if (mFileName.empty()) {
    handleError(DBA_DB_ERROR,"no database filename provided");
    return;
  };

  sqlite3* db;
  int err = sqlite3_open(mFileName.c_str(),&db);
  switch(err) {
    case SQLITE_OK:
    break;
    //SQL related error (user caused)
    case SQLITE_ERROR:
    case SQLITE_SCHEMA:
    case SQLITE_CONSTRAINT:
    case SQLITE_MISMATCH:
    case SQLITE_AUTH: {
      sqlite3_close(db);
      handleError(DBA_SQL_ERROR,sqlite3_errmsg(db));
      return;
    } break;
    //Database/library related error (system caused)
    default: {
      std::string err(sqlite3_errmsg(db));
      sqlite3_close(db);
      handleError(DBA_DB_ERROR,err.c_str());
      return;
    } break;
  };
  
  mConnHandle.reset(db);
  for(map<const char*,collationFunc>::const_iterator it = pCols.begin(); it != pCols.end(); it++) {
    int err = sqlite3_create_collation(db,it->first,SQLITE_UTF8,NULL,it->second);
    if (err != SQLITE_OK) {
      handleError(DBA_DB_ERROR,(string("Cannot create collation ") + it->first).c_str());
      return;
    };
  };
};

DbResult*
SLConnection::execQuery(const char* pSql) {
  SLResult* res = new SLResult(this,mConnHandle,pSql);
  return res;
};

int
SLConnection::execUpdate(const char* pSql) {
  char* error;
  int code = sqlite3_exec(mConnHandle,pSql,NULL,NULL,&error);
  //move error to ptr because checkError can throw exception
  switch(code) {
    case SQLITE_OK:
    break;
    //SQL related error (user caused)
    case SQLITE_ERROR:
    case SQLITE_SCHEMA:
    case SQLITE_CONSTRAINT:
    case SQLITE_MISMATCH:
    case SQLITE_AUTH: {
      try {
        handleError(DBA_SQL_ERROR,error);
      } catch (...) {
        sqlite3_free(error);
        throw;
      };
      return -1;
    } break;
    //Database/library related error (system caused)
    default: {
      try {
        handleError(DBA_DB_ERROR,error);
      } catch (...) {
        sqlite3_free(error);
        throw;
      };
      return -1;
    } break;
  };
  return sqlite3_changes(mConnHandle);
};

std::list<std::string> 
SLConnection::getRelationNames() {
  std::list<std::string> ret;
  std::auto_ptr<dba::DbResult> res(sendQuery("SELECT name FROM sqlite_master WHERE type='table' ORDER BY name"));
  while(res->fetchRow()) {
    ret.push_back(res->getString(0));
  };
  return ret;
};

SLConnection::~SLConnection() {
};


SLResult::SLResult(SLConnection* pOwner, sqlite3* pConn, const char* pSql) 
  : mConn(pConn)
{
  setConversionSpecs(pOwner->getConversionSpecs());
  setParentErrorHandler(pOwner);
  sqlite3_stmt* stmt = NULL;
  const char* tail;
  int error = sqlite3_prepare(mConn,pSql,strlen(pSql),&stmt,&tail);
  switch(error) {
    case SQLITE_OK:
    break;
    //SQL related error (user caused)
    case SQLITE_ERROR:
    case SQLITE_SCHEMA:
    case SQLITE_CONSTRAINT:
    case SQLITE_MISMATCH:
    case SQLITE_AUTH: {
      handleError(DBA_SQL_ERROR,sqlite3_errmsg(mConn));
      return;
    } break;
    //Database/library related error (system caused)
    default: {
      handleError(DBA_DB_ERROR,sqlite3_errmsg(mConn));
      return;
    } break;
  };
  mRes.reset(stmt);
  for(int i = 0; i < columns(); i++) {
    SLColumn col(sqlite3_column_name(mRes,i),mConvSpecs.mDbCharset);
    mColumns.insert(make_pair(i,col));
  };
  mLastRow = false;
  mRowFetched = false;
};

int 
SLResult::columns() const {
  return sqlite3_column_count(mRes);
};

const DbColumn& 
SLResult::getColumn(int i) const {
  map<int,SLColumn>::const_iterator it = mColumns.find(i);
  if (it == mColumns.end()) {
    handleError(DBA_DB_ERROR,"cannot get column - index out of range");
  };
  return it->second;
};

bool 
SLResult::fetchRow() {
  int err = sqlite3_step(mRes);
  switch(err) {
    case SQLITE_ROW:
      mRowFetched = true;
    break;
    case SQLITE_DONE:
      return false;
    break;
    //SQL related error (user caused)
    case SQLITE_ERROR:
    case SQLITE_SCHEMA:
    case SQLITE_CONSTRAINT:
    case SQLITE_MISMATCH:
    case SQLITE_AUTH: {
      handleError(DBA_SQL_ERROR,sqlite3_errmsg(mConn));
      return false;
    } break;
    //Database/library related error (system caused)
    default: {
      handleError(DBA_DB_ERROR,sqlite3_errmsg(mConn));
      return false;
    } break;
  };
  return true;
};

const char* 
SLResult::doGetString(int pField) const {
  if (!mRowFetched) {
    handleError(DBA_DB_ERROR,"row not fetched. Call fetchRow() first.");
  };
  return (const char*)sqlite3_column_text(mRes,pField);
};

long 
SLResult::doGetInt(int pField) const {
  if (!mRowFetched) {
    handleError(DBA_DB_ERROR,"row not fetched. Call fetchRow() first.");
  };
  return sqlite3_column_int(mRes,pField);
};

double 
SLResult::doGetDouble(int pField) const {
  if (!mRowFetched) {
    handleError(DBA_DB_ERROR,"row not fetched. Call fetchRow() first.");
  };
  return sqlite3_column_double(mRes,pField);
};

struct tm 
SLResult::doGetDate(int pField) const {
  if (!mRowFetched) {
    handleError(DBA_DB_ERROR,"row not fetched. Call fetchRow() first.");
  };
  return parseISODateTime(getString(pField));
};

bool 
SLResult::doCheckNull(int pField) const {
  if (!mRowFetched) {
    handleError(DBA_DB_ERROR,"row not fetched. Call fetchRow() first.");
  };
  return sqlite3_column_type(mRes,pField) == SQLITE_NULL;
};


class colMatcher {
  public:
    colMatcher(const char* pName) : mName(pName) {};
    bool operator() (const pair<int,SLColumn>& pPair) {
      return strcmp(pPair.second.getName(),mName) == 0;
    };
  private:
    const char* mName;
};

int
SLResult::getColumnIndex(const char* pField) const {
  map<int,SLColumn>::const_iterator it = find_if(mColumns.begin(), mColumns.end(), colMatcher(pField)); 
  if (it == mColumns.end()) {
    char* err = new char[13 + strlen(pField)];
    sprintf(err,"No such field %s",pField);
    handleError(DBA_DB_ERROR,err);
    delete [] err;
    return -1;
  };
  return it->first;
};

SLResult::~SLResult() {
};

SLColumn::SLColumn(const char* pName, dba::ConvSpec::charset pDbCharset) 
  : DbColumn(pDbCharset)
{
  mName = strdup(pName);  
};

const char* 
SLColumn::getName() const {
  return mName;
};

SLColumn::SLColumn(const SLColumn& pCol) 
  : DbColumn(pCol)
{
  mName = strdup(pCol.mName);
};

SLColumn& 
SLColumn::operator=(const SLColumn& pCol) {
  if (this != &pCol) {
    free(mName);
    mName = strdup(pCol.mName);
    DbColumn::operator=(pCol);
  };
  return *this;
};

SLColumn::~SLColumn() {
  free(mName);
};

Database* 
dbaCreateDb() {
  return new Db();
};

void
dbaDeleteDb(Database* pDb) {
  delete pDb;
};

dbaPluginInfo 
dbaGetPluginInfo() {
  dbaPluginInfo info;
  strcpy(info.mName,"SQLite 3.x");
  strcpy(info.mLicense,DBA_LICENSE);
  strcpy(info.mDescription,"SQLlite version 3 driver for DBA library");
  strcpy(info.mVendor,DBA_VENDOR);
  strcpy(info.mUrl,DBA_URL);
  info.mVersionMajor = DBA_VERSION_MAJOR;
  info.mVersionMinor = DBA_VERSION_MINOR;
  info.mVersionRelease = DBA_VERSION_RELEASE;
  info.mPluginType = DBA_DATABASE_DRIVER;
  return info;
};

};
