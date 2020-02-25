// File: oracle.h
// Purpose: Oracle plugin for DBA (not maintained, needs update)
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2001
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#ifndef ORACLEORACLE_H
#define ORACLEORACLE_H


#include <exception.h>
#include <pointers.h>
#include <conversion.h>
#include "database.h"
#include <strstream>
#include <set>

extern "C" {
#include <stdlib.h>
#include <oratypes.h>
#include <ocidfn.h>
#include <ociapr.h>
#include <ociap.h>
#include <oci.h>
}

namespace oracle {

class OracleRow;
class OracleColumn;
class OracleResult;
class DbOracle;

/*OCI Error Handle*/
struct OCIErrorFree {
  void operator() (OCIError* err) {
    OCIHandleFree(err,OCI_HTYPE_ERROR);                       
  };
};
typedef Resource<OCIError,OCIErrorFree> ResErr;

/*OCI Context handle - implicitly allocated when OCILogon() is called*/

struct OCISvcCtxFree {
  void operator() (OCISvcCtx* ctx) {
    OCIHandleFree(ctx,OCI_HTYPE_ERROR);                       
  };
};
typedef Resource<OCISvcCtx,OCISvcCtxFree> ResCtx;



class OracleException : public Exception {
  public:
    OracleException(ResErr, sword);
    OracleException(string text) : Exception(1,text) {};
};

class OraTransaction : public DbTransaction {
  public:
    OraTransaction();
    virtual void start();
    virtual void commit();
    virtual void rollback();
    virtual Pointer<DbResult> sendQuery(string sql);
    virtual ~OraTransaction();
};


class OracleTable : public DbTable {
  private:
    OracleTable(string name) : DbTable(name) {};
  friend class DbOracle;
};

class DbOracle :public Database {
  public:
    /*Oracle Database constructor
      name - Oracle database identifier
      home - full path to dir where Oracle was installed

      Constructor creates enviroment and allocates error handle
    */
    DbOracle(string name, string home);
    /*assumes that ORACLE_HOME was set earlier*/
    DbOracle(string name);
    /*Connect to database*/
    virtual void connect(string username, string password);
    /*disconnect from database*/
    virtual void disconnect();
    /*send query to database
      sql - query to send
      
      Return value: Reference to DbResult object. Using this object client can fetch data returned by query.
      Clent recieves only reference and is not allowed to destroy return value
    */
    virtual Pointer<DbResult> sendQuery(string sql);
    /**/
    virtual Pointer<DbTransaction> createTransaction() {};

    virtual Pointer<DbTable> getTable(string name);

    ~DbOracle();

    /*OCI Enviroment Handle*/
    struct OCIEnvFree {
      void operator() (OCIEnv* env) {
        OCIHandleFree(env,OCI_HTYPE_ENV);                       
      };
    };
    typedef Resource<OCIEnv,OCIEnvFree> ResEnv;


  private:
    ResEnv mEnv;
    ResErr mErr;
    ResCtx mCtx;
    /*Database name*/
    string mvName;
    /*Initiates Database Enviroment*/
    void init(string name);
};



/*
OracleColumn description

FIXME offset to data should be known by Result rather than Column
*/

class OracleColumn : public DbColumn {
  public:
    OracleColumn(ub2 type,string name,int size);
    ub2 getOracleType() {return mvOracleType;};
    int getOffset() const {return mvOffset;};
    void setOffset(int offset) {mvOffset = offset;};
    virtual ~OracleColumn() {};
  private:
    /*Oracle External Data Type*/
    ub2 mvOracleType;
    /*Offset to data in OracleResult storage (OracleResult::mvDataSource
      mvDataSource + mvOffset is pointer where data from this column should be stored
    */
    int mvOffset;
};


class OracleResult : public DbResult {
  public:
    OracleResult(OCIStmt *o_stmt,ResErr,ResCtx);
    /*number of columns in result*/
    virtual long modifiedRows() const {throw OracleException("Oracle modifiedRows: TODO!");};
    virtual int columns() const {return mvColumns.size();};
    virtual DbColumn& getColumn(int i) {return mvColumns[i];};
    virtual bool fetchRow();
    virtual DbField operator[] (int);       
    virtual ~OracleResult();
  private:
    rslist <OracleColumn> mvColumns;
    /*OCI Handles are copied from DbOracle class upon creation
      Do not free this handles
    */
    ResCtx mCtx;
    ResErr mErr;

    /*Statement handle related with query/result handle
        freed when result is deleted
    */
    struct OCIStmtFree {
      void operator() (OCIStmt* pStmt) {
        OCIHandleFree(pStmt,OCI_HTYPE_STMT);
      };
    };
    typedef Resource<OCIStmt,OCIStmtFree> ResStmt;
    ResStmt mStmt;

    /*Buffer for data from oracle*/
    char *mvDataSource;
    /*NULL and truncuation indicators*/
    sb2* mvIndicators;
    /*Row (field) error codes*/
    ub2* mvRowErrors;
    /*what to do when FetchRow() is called*/
    bool mvExecuted;
};

}; //namespace oracle

#endif
