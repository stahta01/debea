// File: database.cpp
// Purpose: Base class for SQL driver plugins
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2001
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#include "dba/database.h"
#include "dba/plugininfo.h"
#include "dba/exception.h"

#include <iostream>

namespace dba {

const int Database::sVersionMajor = DBA_VERSION_MAJOR;
const int Database::sVersionMinor = DBA_VERSION_MINOR;
const int Database::sVersionRelease = DBA_VERSION_RELEASE;

DbBase::DbBase()
  : mParent(NULL),
    mErrorHandler(NULL)
{
};

void 
DbBase::setParentErrorHandler(DbBase* pParent) {
  if (mParent != NULL) {
    mParent->mChildren.erase(this);
  };
  mParent = pParent;
  if (mParent != NULL) {
    mParent->mChildren.insert(this);
  };
};  


void
DbBase::setErrorHandler(void* pOwner, errorHandlerFunc pFunc) {
  mErrorHandler = pFunc;
};

DbBase::~DbBase()
{
  if (mParent != NULL) {
    mParent->mChildren.erase(this);
  };
  for(std::set<DbBase*>::iterator it = mChildren.begin(); it != mChildren.end(); it++) {
    (*it)->mParent = mParent;
    if (mParent != NULL) {
      mParent->mChildren.insert(*it);
    };
  };
};


void 
DbBase::handleError(int pErrorType, const char* pMsg) const {
  if (mErrorHandler == NULL) {
    if (mParent != NULL) {
      mParent->handleError(pErrorType, pMsg);
      return;
    };
    switch(pErrorType) {
      case DBA_SQL_ERROR:
        std::cerr << "SQL Error: " << pMsg << std::endl;
      break;
      case DBA_DB_ERROR:
        std::cerr << "Database Error: " << pMsg << std::endl;
      break;
      default:
        std::cerr << "Unknown error in dba module" << std::endl;
      break;
    };
    return;
  } else if ((*mErrorHandler)(mObj,pErrorType,pMsg) == false) {
    if (mParent != NULL) {
      mParent->handleError(pErrorType, pMsg);
    };
  };
};



DbConnection::DbConnection() 
  : mUseCount(0)
{
};

bool 
DbConnection::isUsed() const { 
  return mUseCount != 0; 
};

void 
DbConnection::incUsed() { 
  mUseCount++; 
};

void 
DbConnection::decUsed() { 
  mUseCount--; 
};

DbResult* 
DbConnection::sendQuery(const char* pSql) {
  try {
    #ifdef DBA_QUERY_DEBUG
      std::cerr << this << ":" << pSql << std::endl;
    #endif
    return execQuery(pSql);
  } catch (const SQLException& pEx) {
    SQLException ex(pEx);
    ex.setQuery(pSql);
    throw ex;
  };
};

int 
DbConnection::sendUpdate(const char* pSql) {
  try {
    #ifdef DBA_QUERY_DEBUG
      std::cerr << this << ":" << pSql << std::endl;
    #endif
    return execUpdate(pSql);
  } catch (const SQLException& pEx) {
    SQLException ex(pEx);
    ex.setQuery(pSql);
    throw ex;
  };
};

//static
const struct ::tm DbResult::sInvalidTm = {-1,-1,-1,-1,-1,-1};

const char* 
DbResult::getString(int pField) const {
  static const char* empty = "";
  if (doCheckNull(pField))
    return empty;
  return doGetString(pField);
};

long 
DbResult::getInt(int pField) const {
  if (doCheckNull(pField))
    return 0;
  return doGetInt(pField);
};

double 
DbResult::getDouble(int pField) const {
  if (doCheckNull(pField))
    return 0;
  return doGetDouble(pField);
};

struct tm 
DbResult::getDate(int pField) const {
  if (doCheckNull(pField))
    return sInvalidTm;
  return doGetDate(pField);
};



};//namespace

