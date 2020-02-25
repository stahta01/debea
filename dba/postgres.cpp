// File: postgres.cpp
// Purpose: Database plugin for PostgresSQL RDBMS
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2001
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#include <assert.h>
#include <string.h>
#include <memory>

#include "dba/postgres.h"
#include "dba/conversion.h"

using namespace std;
using namespace dba;

namespace postgres {

struct tm
parseDate(const char* pDate) {
    return parseISODateTime(pDate);
};

/*-----------------------------------------------------------*/

const char*
getError(PGconn* pConn) {
  const char *ret = "Postgres connection handle is NULL";
  if (pConn != NULL) {
    return PQerrorMessage(pConn);   
  }
  return ret;
};

PgResult*
PgConn::sendPgQuery(PGconn* conn,const char* sql) {
  PGresult* res = PQexec(conn,sql);
  if (!res) {
    handleError(DBA_DB_ERROR,"Failed to create query result object");
    return NULL;
  };

  switch(PQresultStatus(res)) {
    case PGRES_COMMAND_OK:
      PQclear(res);
      handleError(DBA_DB_ERROR,"Not a query");
    case PGRES_TUPLES_OK: {
      PgResult* r = new PgResult(this,res);
      return r;
    } break;
    case PGRES_BAD_RESPONSE:
    case PGRES_FATAL_ERROR:
    case PGRES_NONFATAL_ERROR: {
      string s(PQresultErrorMessage(res));
      PQclear(res);
      if (PQstatus(connHandle) == CONNECTION_OK)
        handleError(DBA_SQL_ERROR,s.c_str());
      handleError(DBA_DB_ERROR,s.c_str());
    } break;
    default:
      PQclear(res);
      handleError(DBA_DB_ERROR,"Unknown Query execution error");
    break;
  }
  return NULL; 
}

int
PgConn::execUpdate(const char* sql) {
  PGresult* res = PQexec(connHandle,sql);
  if (!res) {
    handleError(DBA_DB_ERROR,"Failed to create query result object");
    return -1;
  };

  switch(PQresultStatus(res)) {
    case PGRES_COMMAND_OK: {
      int n;
      const char* num = PQcmdTuples(res);
      convert(num,n);
      PQclear(res);
      return n;
    } break;
    case PGRES_TUPLES_OK: {
      handleError(DBA_DB_ERROR,"query was not a command");
    } break;
    case PGRES_BAD_RESPONSE:
    case PGRES_FATAL_ERROR:
    case PGRES_NONFATAL_ERROR: {
      string s(PQresultErrorMessage(res));
      PQclear(res);
      if (PQstatus(connHandle) == CONNECTION_OK)
        handleError(DBA_SQL_ERROR,s.c_str());
      handleError(DBA_DB_ERROR,s.c_str());
    } break;
    default:
      PQclear(res);
      handleError(DBA_DB_ERROR,"Unknown Query execution error");    
    break;
  }
  return -1;
}


DbConnection*
Db::getConnection(const char* pParams) {
  if (pParams == NULL) {
    handleError(DBA_DB_ERROR,"NULL connection params");
    return NULL;
  };
  PGconn* conn = PQconnectdb(pParams);
  switch(PQstatus(conn)) {
    case CONNECTION_BAD:
    {
      std::string error = getError(conn);
      PQfinish(conn);
      handleError(DBA_DB_ERROR,error.c_str());
    }
    break;
    case CONNECTION_OK: {
      PgConn* c = new PgConn(conn);
      c->setParentErrorHandler(this);
      c->setConversionSpecs(getConversionSpecs());
      return c;
    } break;
    default:
      PQfinish(conn);
      handleError(DBA_DB_ERROR,"(connect) Unknown error");    
    break;
  }
  return NULL;
};

PgConn::PgConn(PGconn* pConn) 
  : connHandle(pConn) 
{
};

DbResult*
PgConn::execQuery(const char* sql) {
  PgResult* r = sendPgQuery(connHandle,sql);
  return r;
};

void
PgConn::disconnect() {
  connHandle.reset();
};

bool
PgConn::isValid() const {
  if (PQstatus(connHandle) == CONNECTION_OK)
    return true;
  return false;
};


PgConn::~PgConn() {
  disconnect();
};


int
PgResult::discover_size(PGresult* res, int col) {
  int size = 0;
  for(int i = 0; i < mvRows; i++) {
    int len = PQgetlength(res,i,col);
    if (len > size)
      size = len;
  };
  return size;
};

PgResult::PgResult(PgConn* pOwner, PGresult* res) 
  : mvRes(res) 
{
  setConversionSpecs(pOwner->getConversionSpecs());
  setParentErrorHandler(pOwner);
  //first fetch() will set this to 0
  currentRow = -1;
  mvRows = PQntuples(res);
  int fields = PQnfields(res);
  for(int i = 0; i < fields;i++) {
    PgColumn* col = new PgColumn(PQfname(res,i),PQftype(res,i),PQfmod(res,i),mConvSpecs.mDbCharset);
    PgColumns.insert(make_pair(i,col));
  };
};

const char*
PgResult::doGetString(int pField) const {
  const char* ptr = PQgetvalue(mvRes,currentRow,pField);
  return ptr;
};

long
PgResult::doGetInt(int pField) const {
  long val;
  try {
    const char* ptr = PQgetvalue(mvRes,currentRow,pField);
    struct lconv* l = localeconv();
    char* lptr = l->decimal_point;
    char* pos = strchr((char*)ptr,*lptr);
    if (pos != NULL) {
      *pos = '\0';    
    };
    convert(ptr,val);
  } catch (const ConversionException& pEx) {
    handleError(DBA_DB_ERROR,pEx.what());
  };
  return val;
};

double
PgResult::doGetDouble(int pField) const {
  char* ptr = NULL;
  double val;
  try {
    ptr = PQgetvalue(mvRes,currentRow,pField);
    convert(ptr,val,&mConvSpecs.mDecimalPoint);
  } catch (const ConversionException& pEx) {
    handleError(DBA_DB_ERROR,pEx.what());
  };
  return val;
};

struct tm
PgResult::doGetDate(int pField) const {
  try {
    return parseDate(PQgetvalue(mvRes,currentRow,pField));
  } catch (const ConversionException& pEx) {
    handleError(DBA_DB_ERROR,pEx.what());
    //cannot use static DbResult::sInvalidTm in dll
    struct ::tm invalid = {-1,-1,-1,-1,-1,-1};
    return invalid;
  };
};

bool
PgResult::doCheckNull(int pField) const {
  return PQgetisnull(mvRes,currentRow,pField) == 1;
};


int
PgResult::getColumnIndex(const char* pName) const {
  return PQfnumber(mvRes,pName);
};

void
PgConn::setAutoCommit(bool pFlag) {
};

std::list<std::string> 
PgConn::getRelationNames() {
  std::list<std::string> ret;
  std::auto_ptr<dba::DbResult> res(
    sendQuery("select c.relname FROM pg_catalog.pg_class c "
              "LEFT JOIN pg_catalog.pg_namespace n ON n.oid = c.relnamespace "
              "WHERE c.relkind IN ('r','') AND n.nspname NOT IN ('pg_catalog', 'pg_toast') "
              "AND pg_catalog.pg_table_is_visible(c.oid) ORDER BY c.relname"
             )
  );
  while(res->fetchRow()) {
    ret.push_back(res->getString(0));
  };
  return ret;
};

void
PgConn::commit() {
  sendUpdate("COMMIT");
};

void
PgConn::begin() {
  sendUpdate("BEGIN");
};

void
PgConn::rollback() {
  sendUpdate("ROLLBACK");
};

int
PgResult::columns() const {
  return PgColumns.size();
};


const DbColumn&
PgResult::getColumn(int i) const {
  PgColsType::const_iterator it = PgColumns.find(i);
  assert(it != PgColumns.end());
  return *(it -> second);
};

bool
PgResult::fetchRow() {
  currentRow++;
  if (currentRow >= mvRows)
    return false;
  return true;
};

PgResult::~PgResult() {
  for(PgColsType::iterator it = PgColumns.begin(); it != PgColumns.end(); it++)
    delete (*it).second;
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
  strcpy(info.mName,"PostgreSQL 7.x");
  strcpy(info.mLicense,DBA_LICENSE);
  strcpy(info.mDescription,"PostgreSQL version 7 driver for DBA library");
  strcpy(info.mVendor,DBA_VENDOR);
  strcpy(info.mUrl,DBA_URL);
  info.mVersionMajor = DBA_VERSION_MAJOR;
  info.mVersionMinor = DBA_VERSION_MINOR;
  info.mVersionRelease = DBA_VERSION_RELEASE;
  info.mPluginType = DBA_DATABASE_DRIVER;
  return info;
};

PgColumn::PgColumn(const char* name,int type, int type_attr, dba::ConvSpec::charset pDbCharset) 
  : DbColumn(pDbCharset),
    mName(name) 
{
  mvType = type;
  mvTypeAttr = type_attr;
};


} // namespace
