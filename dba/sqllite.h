// File: sqlite.h
// Purpose: Database plugin for sqlite database
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2003
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#ifndef SQLLITE_H
#define SQLLITE_H

#include <iostream>

#include <sqlite.h>
#include <assert.h>
#include "dba/database.h"
#include "dba/plugininfo.h"

/**
  sqllite driver for dba library
*/
namespace sqllite {

/**%SQL Lite driver for database class

  Notes on this implementation:

  - When virtual machine in result is about to be finalized (SLVMFree) no error checking is done.
  It is _always_ done in destructor so I don't have idea what to do if it fails.
  Memory leak will occur?

  */

class Db : public dba::Database  {
  public:
    Db();
    virtual dba::DbConnection* getConnection(const char* pParams);
    ~Db();
};

class SLConnFree {
  public:
    void operator() (sqlite* db) {
      std::cerr << "SQL Lite: finishing connection" << std::endl;
      sqlite_close(db);
    };
};


class SLConnection : public dba::DbConnection {
    friend class Db;
  public:
    /**
    Send query to server.
    @returns table of results
    */
    virtual dba::DbResult* sendQuery(const char* pSql);
    /**
    Send update command to server (%SQL UPDATE, %SQL DELETE etc)
    @returns affected rows
    */
    virtual int sendUpdate(const char* pSql);
    /**
    If true transactions will be commited aftera each query
    If false user must commit() changes before destructor is called
    */
    virtual void begin();
    virtual void commit();
    virtual void rollback();
    virtual void disconnect();
    virtual bool isValid() const;
    virtual ~SLConnection();  
  private:
    SLConnection(const char* pParams);
    std::string mFileName;
    pointers::CRes<sqlite,SLConnFree> mConnHandle;    
    bool mAutoCommitFlag;
};

class SLVMFree {
  public:
    void operator() (sqlite_vm* machine) {
      //should be null. Not null if not all rows were fetched.
      if (machine != NULL) {
//        std::cerr << "SQL Lite: cleaning result" << std::endl;
        char* error;
        sqlite_finalize(machine,&error);
      };
    };
};

class SLColumn;

class SLResult : public dba::DbResult {
    friend class SLConnection;
  public:
    virtual int columns() const;
    virtual const dba::DbColumn& getColumn(int i) const;
    virtual const char* getString(const char* pField) const;
    virtual const char* getString(int pField) const;
    virtual long getInt(const char* pField) const;
    virtual long getInt(int pField) const;
    virtual double getDouble(const char* pField) const;
    virtual double getDouble(int pField) const;
    virtual struct tm getDate(const char* pField) const;
    virtual struct tm getDate(int pField) const;
    virtual bool isNull(const char* pField) const;
    virtual bool isNull(int pField) const;
    virtual bool fetchRow();
    virtual ~SLResult();
  private:
    SLResult(const SLResult& pResult);
    SLResult& operator=(const SLResult& pResult);
    typedef enum {NORMAL_FETCH,PREFETCH,LAST_FETCHED} fstatus;
    SLResult(SLConnection* pOwner, sqlite* mConn, const char* pSql);
    void finalize();
    int findColumn(const char* pName) const;
    pointers::CRes<sqlite_vm,SLVMFree> mRes;
    int mColNumber;
    std::map<int,std::string> mData;
    std::map<int,std::string> mNewData;
    std::map<int,int> mNulls;
    std::map<int,int> mNewNulls;
//    char** mData;
//    char** mNames;
//    char** mNewData;
    std::map<int,SLColumn> mCols;
};

class SLColumn : public dba::DbColumn {
  public:
    SLColumn(const char* pName);
    SLColumn(const SLColumn& pCol);
    SLColumn& operator=(const SLColumn& pCol);
    virtual const char* getName() const;
    virtual ~SLColumn();
  private:
    char* mName;
};

extern "C" {
  DLLEXPORT dba::Database* dbaCreateDb();
  DLLEXPORT dba::dbaPluginInfo dbaGetPluginInfo();
};


};//namespace

#endif
