// File: sqlite3.h
// Purpose: Database plugin for sqlite3 database
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2004
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#ifndef SQLITE3SQLITE3_H
#define SQLITE3SQLITE3_H


#include <map>
#include <sqlite3.h>
#include "dba/database.h"
#include "dba/plugininfo.h"
#include "dba/chandle.h"

namespace sqllite3 {

typedef int (*collationFunc)(void* pCaller, int pS1len, const void* pS1, int pS2Len, const void* pS2);

class Db : public dba::Database {
  public:
    Db();
    virtual dba::DbConnection* getConnection(const char* pParams);
    void addCollation(const char* pName, collationFunc pFunc);
    virtual ~Db();
  private:
    std::map<const char*, collationFunc> mCollations;
};

class SLConnFree {
  public:
    void dealloc(sqlite3* db) {
      //std::cerr << "SQL Lite: finishing connection " << db << std::endl;
      sqlite3_close(db);
    };
};


class SLConnection : public dba::DbConnection {
    friend class Db;
  public:
    virtual std::list<std::string> getRelationNames();
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
    SLConnection(Db* pOwner, const char* pParams, const std::map<const char*,collationFunc>& pCols);
    std::string mFileName;
    dba::CHandle<sqlite3*,SLConnFree> mConnHandle;
    bool mAutoCommitFlag;

    /**
    Send query to server.
    @returns table of results
    */
    virtual dba::DbResult* execQuery(const char* pSql);
    /**
    Send update command to server (%SQL UPDATE, %SQL DELETE etc)
    @returns affected rows
    */
    virtual int execUpdate(const char* pSql);
};


class SLColumn : public dba::DbColumn {
  public:
    SLColumn(const char* pName, dba::ConvSpec::charset pDbCharset);
    SLColumn(const SLColumn& pCol);
    SLColumn& operator=(const SLColumn& pCol);
    bool operator==(const SLColumn& pCol);
    virtual const char* getName() const;
    virtual ~SLColumn();
  private:
    char* mName;
};

class SLVMFree {
  public:
    void dealloc(sqlite3_stmt* machine) {
      //should be null. Not null if not all rows were fetched.
      if (machine != NULL) {
//        std::cerr << "SQL Lite: cleaning result" << std::endl;
        sqlite3_finalize(machine);
      };
    };
};

class SLResult : public dba::DbResult {
    friend class SLConnection;
  public:
    virtual int columns() const;
    virtual const dba::DbColumn& getColumn(int i) const;
    virtual bool fetchRow();
    virtual ~SLResult();
  private:
    SLResult(SLConnection* pOwner, sqlite3* pConn, const char* pSql);

    virtual const char* doGetString(int pField) const;
    virtual long doGetInt(int pField) const;
    virtual double doGetDouble(int pField) const;
    virtual struct tm doGetDate(int pField) const;

    virtual int getColumnIndex(const char* pField) const;
    virtual bool doCheckNull(int pField) const;

    sqlite3* mConn;
    dba::CHandle<sqlite3_stmt*,SLVMFree> mRes;
    std::map<int,SLColumn> mColumns;
    bool mLastRow;
    bool mRowFetched;
};

extern "C" {
  dbaDLLEXPORT dba::Database* dbaCreateDb();
  dbaDLLEXPORT void dbaDeleteDb(dba::Database*);
  dbaDLLEXPORT dba::dbaPluginInfo dbaGetPluginInfo();
};

};

#endif
