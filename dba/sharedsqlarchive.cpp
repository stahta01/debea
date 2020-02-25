// File: sharedsqlarchive.h
// Purpose: SQLArchive with connection management and transaction support
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2003
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/sharedsqlarchive.h"
#include "dba/shared_ptr.h"
#include <iostream>

namespace dba {

Transaction::Transaction(DbConnection* pConn, SQLIdFetcher* pFetcher, FilterMapper* pMapper, const shared_ptr<bool>& pRollbackFlag) 
  : mConn(pConn),
    mFetcher(pFetcher),
    mFilterMapper(pMapper),
    mRollbackFlag(pRollbackFlag)
{
  mConn->incUsed();
};

Transaction::Transaction(const Transaction& pTransaction)
  : mConn(pTransaction.mConn),
    mFetcher(pTransaction.mFetcher),
    mFilterMapper(pTransaction.mFilterMapper),
    mRollbackFlag(pTransaction.mRollbackFlag)
{
  mConn->incUsed();
};

Transaction& 
Transaction::operator=(const Transaction& pTransaction) {
  mConn->decUsed();
  mConn = pTransaction.mConn;
  mFetcher = pTransaction.mFetcher;
  mFilterMapper = pTransaction.mFilterMapper;
  mRollbackFlag = pTransaction.mRollbackFlag;
  mConn->incUsed();
  return *this;
};

#ifdef DBA_COMPAT_1_0    
shared_ptr<SQLOStream> 
Transaction::getOOStream() {
  SQLOStream* ret = new SQLOStream(mConn,mFetcher,mFilterMapper);
  ret->open();
  return ret;
};

shared_ptr<SQLIStream> 
Transaction::getOIStream() {
  SQLIStream* ret = new SQLIStream(mConn,mFilterMapper);
  return ret;
};
#endif

SQLIStream
Transaction::getIStream() {
  SQLIStream s(mConn,mFilterMapper);
  return s;
};

SQLOStream
Transaction::getOStream() {
  SQLOStream ret(mConn,mFetcher,mFilterMapper);
  ret.open();
  return ret;
};

void
Transaction::commit() {
  SQLOStream stream(getOStream());
  stream.commit();
  stream.begin();
};

void
Transaction::rollback() {
  *mRollbackFlag = true;
};

bool 
Transaction::isRolledback() const {
  return *mRollbackFlag == true;
};

void
Transaction::cleanStreams() {
  SQLOStream stream(getOStream());
  if (*mRollbackFlag == false)
    stream.commit();
  else
    stream.rollback();
};

Transaction::~Transaction() throw() {
  try {
    //If we are only one reference to connection 
    //then we are last one. But if we were last Transaction
    //created from SharedSQLArchive then stream are referenced from
    //archive itself. In that case we cleanup out references 
    //and archive references returning connection to connection pool
    mConn->decUsed();
    if (!mConn->isUsed()) {
      cleanStreams();
    };
  } catch (const SQLException& ex) {
    std::cerr << "DBA FATAL: error commiting transaction:" << ex.what() << std::endl;
    std::cerr << "Next transaction on this connection (" << this << ") will fail because of this" << std::endl;
  } catch (...) {
    std::cerr << "DBA FATAL: unknwon exception when commiting transaction" << std::endl;
    std::cerr << "Next transaction on this connection (" << this << ") propably will fail because of this" << std::endl;
  };
};


SharedSQLArchive::SharedSQLArchive(Database* pDatabase)
  : SQLArchive(pDatabase),
    mLastTransConnection(NULL)
{
}

Transaction
SharedSQLArchive::createTransaction(Transaction::creationType pType) {
  if (pType == Transaction::TRANS_USE_LAST) {
    //if there is no rollback set connections are NULL too.
    if (mLastTransConnection == NULL) {
      mLastTransConnection = getFreeConnection();
      mRollbackFlag = new bool;
      *mRollbackFlag = false;
    };
  } else {
    //create always new transaction using new connection no matter what.
    //old references to last Transaction will live only in Transaction objects
    //from now.
    mLastTransConnection = getFreeConnection();
    mRollbackFlag = new bool;
    *mRollbackFlag = false;
  };
  //we use stream begin() method to start transaction
  //using stream from Transaction instance that will be returned
  //need_begin have to be before transaction constructor
  //because this constructor will increment connection usage
  bool need_begin = !mLastTransConnection->isUsed();
  Transaction t(mLastTransConnection,mFetcher,&mFilterMapper,mRollbackFlag);
  if (need_begin) {
    *mRollbackFlag = false;
    SQLOStream stream(t.getOStream());
    stream.begin();
  };
  return t; 
};


SharedSQLArchive::~SharedSQLArchive() throw()
{
  try {
    mLastTransConnection = NULL;
  } catch (...) {
  
  };
}


};//namespace


