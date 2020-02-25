// File: sqlite.cpp
// Purpose: Database plugin for sqlite database
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2003
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/sqllite.h"

#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif


using namespace pointers;
using namespace std;
using namespace dba;

namespace sqllite {

void
copy_data(map<int,string>& pDest, map<int,int>& pNulls, const char** pData, int size) {
  for(int i = 0; i < size; i++) {
    if (pData[i] == 0) {
//      cerr << "data in column " << i << " is NULL" << endl;
      pNulls[i] = 1;
    } else {
//      cerr << "data in column " << i << " is " << pData[i]<< endl;
      pNulls[i] = 0;
      pDest[i] = pData[i];
    };
  };
};



Db::Db(){

};

DbConnection*
Db::getConnection(const char* pParams) {
  SLConnection* conn = new SLConnection(pParams);
  conn->setParentErrorHandler(this);
  return conn;
};


Db::~Db(){

};

void
SLConnection::begin() {
//  cerr << "begin" << endl;
  sendUpdate("BEGIN");
};

void
SLConnection::commit() {
//  cerr << "commit" << endl;
  sendUpdate("COMMIT");
};

void
SLConnection::rollback() {
//  cerr << "rollback" << endl;
  sendUpdate("ROLLBACK");
};


void
SLConnection::disconnect() {
  mConnHandle = NULL;
};

bool
SLConnection::isValid() const {
  return (mConnHandle != NULL);
};



SLConnection::SLConnection(const char* pParams) {
  mFileName = pParams;
  char* err = NULL;
  mAutoCommitFlag = true;
  //second argument ignored
  mConnHandle = sqlite_open(mFileName.c_str(),0,&err);
  if (mConnHandle == NULL) {
    handleError(DBA_DB_ERROR,err);
    sqlite_freemem(err);
  };
};

DbResult*
SLConnection::sendQuery(const char* pSql) {
  SLResult* res = new SLResult(this,mConnHandle,pSql);
  return res;
};

SLConnection::~SLConnection() {
//  cerr << "SL destructor" << endl;
  if (!mAutoCommitFlag) {
    try {
      rollback();
    } catch(...) {
      assert(false);
    };
  };
};


int
resultCallback(void* pArg, int argc, char** argv, char**columnNames) {
  return 0;
};

int
SLConnection::sendUpdate(const char* pSql) {
  char* error;
  int code = sqlite_exec(mConnHandle,pSql,&resultCallback,NULL,&error);
  switch(code) {
    case SQLITE_OK:
    break;
    //SQL related error
    case SQLITE_ERROR:
    case SQLITE_SCHEMA:
    case SQLITE_CONSTRAINT:
    case SQLITE_MISMATCH:
    case SQLITE_AUTH: {
      handleError(DBA_SQL_ERROR,error);
      sqlite_freemem(error);
      return -1;
    } break;
    //Database/library related error
    default: {
      handleError(DBA_DB_ERROR,error);
      sqlite_freemem(error);
      return -1;
    } break;
  };
  return sqlite_changes(mConnHandle);
};


SLResult::SLResult(SLConnection* pOwner, sqlite* mConn, const char* pSql) 
{
  const char* tail;
  sqlite_vm* vm;
  char* error;
  setParentErrorHandler(pOwner);
  int code = sqlite_compile(mConn,pSql,&tail,&vm,&error);
  switch(code) {
    case SQLITE_OK:
    break;
    //SQL related error
    case SQLITE_ERROR:
    case SQLITE_SCHEMA:
    case SQLITE_CONSTRAINT:
    case SQLITE_MISMATCH:
    case SQLITE_AUTH: {
      handleError(DBA_SQL_ERROR,error);
      sqlite_freemem(error);
      return;
    } break;
    //Database/library related error
    default: {
      handleError(DBA_DB_ERROR,error);
      sqlite_freemem(error);
      return;
    } break;
  };
  mRes = vm;
  //we must fetch at least one row to obtain column information
  const char** data;
  const char** names;
  code = sqlite_step(mRes,&mColNumber,&data,&names);
  //get column info
  for(int i = 0; i < mColNumber ; i++) {
    SLColumn col(names[i]);
    mCols.insert(make_pair(i,col));
  };
  switch(code) {
    case SQLITE_BUSY: {
      finalize();
    } break;
    case SQLITE_MISUSE: {
      finalize();
    } break;
    case SQLITE_DONE: {
      finalize();
    } break;
    case SQLITE_ROW: {
      copy_data(mNewData,mNewNulls,data,mColNumber);
    } break;
    case SQLITE_ERROR: {
      finalize();
    } break;
  };
};

bool
SLResult::fetchRow() {
  if (mRes == NULL)
    return false;
  int cols;
  const char** data;
  const char** names;
  int code = sqlite_step(mRes,&cols,&data,&names);
//  cerr << "sqlite data:" <<(void*)data << endl;
  switch(code) {
    case SQLITE_BUSY: {
      finalize();
    } break;
    case SQLITE_MISUSE: {
      finalize();
    } break;
    case SQLITE_DONE: {
      mData = mNewData;
      mNulls = mNewNulls;
      finalize();
      return true;
    } break;
    case SQLITE_ROW: {
      mData = mNewData;
      mNulls = mNewNulls;
      copy_data(mNewData,mNewNulls,data,mColNumber);
      return true;      
    };
    case SQLITE_ERROR: {
      finalize();
    };
  };
  return false; //false?
};



void
SLResult::finalize() {
  mRes = NULL;
};

const char*
SLResult::getString(const char* pField) const {
  int col = findColumn(pField);
  return getString(col);
};

const char*
SLResult::getString(int pField) const {
  map<int,string>::const_iterator it = mData.find(pField);
  //row not fetched
  assert(it != mData.end());
  return it -> second.c_str();
};

long
SLResult::getInt(const char* pField) const {
  int col = findColumn(pField);
  return getInt(col);
};

long
SLResult::getInt(int pField) const {
  map<int,string>::const_iterator it = mData.find(pField);
  //row not fetched
  assert(it != mData.end());
  long res;
  convert(it -> second,res);
  return res;
};

double
SLResult::getDouble(const char* pField) const {
  int col = findColumn(pField);
  return getDouble(col);
};

double
SLResult::getDouble(int pField) const {
  map<int,string>::const_iterator it = mData.find(pField);
  //row not fetched
  assert(it != mData.end());
  double res;
  const char* old = setlocale(LC_NUMERIC,"C");
  convert(it -> second,res);
  if (old)
    setlocale(LC_NUMERIC,old);
  return res;
};

struct tm
SLResult::getDate(const char* pField) const {
  int col = findColumn(pField);
  return getDate(col);
};

struct tm
SLResult::getDate(int pField) const {
  //not supported
  assert(false);
  struct tm t;
  t.tm_hour=1;
  return t;
};

bool
SLResult::isNull(const char* pField) const {
  int col = findColumn(pField);
  return isNull(col);
};

bool
SLResult::isNull(int pField) const {
//  cerr << "testing null " << (void*)mData << " field " << pField << endl;
  map<int,int>::const_iterator it = mNulls.find(pField);
  assert(it != mNulls.end());
  return(it -> second == 1);
};

int
SLResult::findColumn(const char* pName) const {
  for(map<int,SLColumn>::const_iterator it = mCols.begin(); it != mCols.end() ; it++) {
    if (!strcmp(it -> second.getName(),pName))
      return it -> first;
  };
  string err(string("column ") + pName + " not found");
  handleError(DBA_DB_ERROR, err.c_str());
  return -1;
};

int
SLResult::columns() const {
  return mColNumber;
};

const DbColumn&
SLResult::getColumn(int i) const {
  assert(i < mColNumber);
  map<int,SLColumn>::const_iterator it = mCols.find(i);
  assert(it != mCols.end());
  return it -> second;
};


SLResult::~SLResult() {
//  assert(mRes == NULL);
};

SLColumn::SLColumn(const char* pName) 
{
  mName = strdup(pName);  
};

const char* 
SLColumn::getName() const {
  return mName;
};

SLColumn::SLColumn(const SLColumn& pCol) {
  mName = strdup(pCol.mName);
};

SLColumn& 
SLColumn::operator=(const SLColumn& pCol) {
  if (this != &pCol) {
    free(mName);
    mName = strdup(pCol.mName);
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

dbaPluginInfo 
dbaGetPluginInfo() {
  dbaPluginInfo info;
  strcpy(info.mName,"SQLite 2.x");
  strcpy(info.mLicense,DBA_LICENSE);
  strcpy(info.mDescription,"SQLlite version 2 driver for DBA library");
  strcpy(info.mVendor,DBA_VENDOR);
  strcpy(info.mUrl,DBA_URL);
  info.mVersionMajor = DBA_VERSION_MAJOR;
  info.mVersionMinor = DBA_VERSION_MINOR;
  info.mVersionRelease = DBA_VERSION_RELEASE;
  info.mPluginType = DBA_DATABASE_DRIVER;
  return info;
};


};//namespace
