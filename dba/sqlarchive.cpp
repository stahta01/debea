// File: sqlarchive.cpp
// Purpose: Archive for SQL databases
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2003
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#include <iostream>
#include "dba/sqlarchive.h"
#include "dba/dbplugin.h"
#include "dba/exception.h"
#include "dba/database.h"
#include "dba/sqlidfetcher.h"

namespace dba {

using namespace std;

bool
SQLArchive::handleError(void* pOwner, int pType, const char* pMsg) {
  switch(pType) {
    case DBA_DB_ERROR:
      throw DatabaseException(pMsg);
    break;
    case DBA_SQL_ERROR:
      throw SQLException(pMsg);
    break;
  };
  return false;
};

void 
SQLArchive::setConversionSpecs(const ConvSpec& pSpecs) {
  if (mDb == NULL)
    throw APIException("Databse not initialized");
  mDb->setConversionSpecs(pSpecs);
  for(std::list<DbConnection*>::iterator it = mConnections.begin(); it != mConnections.end(); it++) {
    (*it)->setConversionSpecs(pSpecs);
  };
};

const ConvSpec& 
SQLArchive::getConversionSpecs() const {
  if (mDb == NULL)
    throw APIException("Databse not initialized");
  return mDb->getConversionSpecs();
};

void 
SQLArchive::mapType(const std::type_info& pType, StoreableFilterBase* pFilter) {
  mFilterMapper.mapType(pType,pFilter);
}

SQLArchive::SQLArchive(Database* pDb)
  : mDb(pDb),
    mPlugin(NULL)
{
  mFetcher = NULL;
  setDatabase(pDb);
};

void
SQLArchive::open(const char* pConnectStr) {
//  cerr << "SQL archvie: creating connection" << endl;
  mConnectStr = pConnectStr;
  getFreeConnection();
};

void
SQLArchive::open(const char* pPluginName, const char* pConnectString) {
  initPlugin(pPluginName);
  open(pConnectString);
};

void 
SQLArchive::initPlugin(const char* pPluginName) {
  destroyPlugin();
  mPlugin = new DbPlugin(pPluginName);
  mDb = mPlugin->createHandle();
  mDb->setErrorHandler(this,&handleError);
};

bool
SQLArchive::isOpen() const {
  if (mConnections.size() == 0)
    return false;
  return true;
};

void
SQLArchive::setIdFetcher(SQLIdFetcher* pFetcher) {
  mFetcher = pFetcher;
};

SQLIdFetcher* 
SQLArchive::getIdFetcher() const {
  return mFetcher;
};


#ifdef DBA_COMPAT_1_0
SQLIStream*
SQLArchive::getOIStream() {
  DbConnection* conn = getFreeConnection();
  SQLIStream* stream = new SQLIStream(conn,&mFilterMapper);
  return stream;
};

SQLOStream*
SQLArchive::getOOStream() {
  DbConnection* conn = getFreeConnection();
  SQLOStream* stream = new SQLOStream(conn,mFetcher);
  return stream;
};
#endif

IStream*
SQLArchive::getInputStream() {
  DbConnection* conn = getFreeConnection();
  SQLIStream* stream = new SQLIStream(conn,&mFilterMapper);
  return stream;
};

OStream*
SQLArchive::getOutputStream() {
  DbConnection* conn = getFreeConnection();
  SQLOStream* stream = new SQLOStream(conn,mFetcher,&mFilterMapper);
  return stream;
};

SQLIStream
SQLArchive::getIStream() {
  DbConnection* conn = getFreeConnection();
  SQLIStream stream(conn,&mFilterMapper);
  return stream;
};

SQLOStream
SQLArchive::getOStream() {
  DbConnection* conn = getFreeConnection();
  SQLOStream stream(conn,mFetcher,&mFilterMapper);
  return stream;
};

DbConnection*
SQLArchive::getFreeConnection(bool pRelease) {
  for(std::list<DbConnection*>::iterator it = mConnections.begin(); it != mConnections.end(); it++) {
    if (!(*it)->isUsed()) {
      if (!pRelease) {
        return *it;
      } else {
        DbConnection* ret(*it);
        mConnections.erase(it);
        return ret;
      }
      break;
    };
  };
  std::cerr << "creating new connection (params: [" << mConnectStr << "], currently open: " << mConnections.size() << ")" << std::endl;
  if (mDb == NULL) {
    throw APIException("Database not initialized, cannot create connection");
  };
  DbConnection* ret = mDb->getConnection(mConnectStr.c_str());
  if (ret == NULL) {
    throw DatabaseException("Cannot initialize connection to database");
  };
  ret->setErrorHandler(this,&handleError);
  if (!pRelease) {
    mConnections.push_front(ret);
  };
//  cerr << "assigning connection" << endl;
  return ret;
};

int
SQLArchive::getAvailableConnections() const {
  int avail = 0;
  for(std::list<DbConnection*>::const_iterator it = mConnections.begin(); it != mConnections.end(); it++) {
    if (!(*it)->isUsed())
      avail++;
  };
  return avail;
};

int
SQLArchive::getUsedConnections() const {
  int used = 0;
  for(std::list<DbConnection*>::const_iterator it = mConnections.begin(); it != mConnections.end(); it++) {
    if ((*it)->isUsed())
      used++;
  };
  return used;
};

DbConnection*
SQLArchive::getConnection() {
  DbConnection* conn = getFreeConnection(true);
  return conn;
};

std::list<std::string> 
SQLArchive::getTableNames() {
  DbConnection* conn = getFreeConnection(false);
  return conn->getRelationNames();
};

Database* 
SQLArchive::getDatabase() {
  return mDb;
};

void 
SQLArchive::setDatabase(Database* pDb) {
  destroyPlugin();
  mDb = pDb;
  if (mDb != NULL)
    mDb->setErrorHandler(this,&handleError);
};

void
SQLArchive::destroyPlugin() {
  closeAllConnections();
  if (isDatabaseOwner()) {
    mPlugin->destroyHandle(mDb);
    delete mPlugin;
    mDb = NULL;
    mPlugin = NULL;
  };
};

void
SQLArchive::closeAllConnections() {
  for(list<DbConnection*>::iterator it = mConnections.begin(); it != mConnections.end(); it++) {
    delete (*it);
  };
};

SQLArchive::~SQLArchive() {
  //close all connections and delete plugin and database instance here
  setDatabase(NULL);
  delete mFetcher;
};

};//namespace


