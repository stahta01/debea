// File: postgres.h
// Purpose: Database plugin for PostgresSQL RDBMS
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2001
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#ifndef POSTGRESPOSTGRES_H
#define POSTGRESPOSTGRES_H

#include <iostream>
#include "dba/database.h"
#include "dba/plugininfo.h"
#include "dba/chandle.h"
#include <map>

extern "C" {
  #include <libpq-fe.h>
}

/**
  PosgtreSQL driver for dba library
*/

namespace postgres {

/**
  Postgres extension to DBA library
  */


class PgResult;
typedef std::set<PgResult*> PgResType;


struct PgConnFree {
    void dealloc(PGconn* c) {
//        std::cerr << "Postgres: finishing connection" << std::endl;
      PQfinish(c);
    }
};

class PgColumn : public dba::DbColumn {
  public:
    PgColumn(const char* name,int type, int type_attr, dba::ConvSpec::charset pDbCharset);
    virtual const char* getName() const { return mName; };
    virtual ~PgColumn() {};
  private:
    int mvType;
    int mvTypeAttr;
    const char* mName;
};


class Db : public dba::Database  {
  public:
    virtual dba::DbConnection* getConnection(const char* pParams);
  private:
};

class PgConn : public dba::DbConnection {
  friend class Db;
  public:
    virtual std::list<std::string> getRelationNames();
    /**
    If true transactions will be commited aftera each query
    If false user must commit() changes before destructor is called
    */
    virtual void setAutoCommit(bool pFlag = true);
    virtual void begin();
    virtual void commit();
    virtual void rollback();
    virtual void disconnect();
    virtual bool isValid() const;
    virtual ~PgConn();
  private:
    PgConn(PGconn* pConn);
    dba::CHandle<PGconn*,PgConnFree> connHandle;
    PgResult* sendPgQuery(PGconn* conn,const char* sql);

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


struct PgResFree {
    void dealloc(PGresult* r) {
//        std::cerr << "Postgres: clear result" << r << std::endl;
      PQclear(r);
    }
};

class PgResult : public dba::DbResult {
    friend class PgConn;
  public:
    enum res_type {
      SELECT,
      COMMAND
    };
    virtual int columns() const;
    virtual const dba::DbColumn& getColumn(int i) const;
    virtual bool fetchRow();
    virtual ~PgResult();
  private:
    PgResult(PgConn* pOwner, PGresult*);

    virtual const char* doGetString(int pField) const;
    virtual long doGetInt(int pField) const;
    virtual double doGetDouble(int pField) const;
    virtual struct tm doGetDate(int pField) const;

    virtual int getColumnIndex(const char* pField) const;
    virtual bool doCheckNull(int pField) const;

    typedef std::map<int,PgColumn*> PgColsType;
    dba::CHandle<PGresult*,PgResFree> mvRes;
    int currentRow;
    int mvRows;
    PgColsType PgColumns;
    int discover_size(PGresult* res, int col);
};

extern "C" {
  dbaDLLEXPORT dba::Database* dbaCreateDb();
  dbaDLLEXPORT void dbaDeleteDb(dba::Database*);
  dbaDLLEXPORT dba::dbaPluginInfo dbaGetPluginInfo();
};

} //namespace

#endif
