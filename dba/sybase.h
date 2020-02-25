// File: sybase.cpp
// Purpose: Sybase plugin for DBA
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2002
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#ifndef SYBASESYBASE_H
#define SYBASESYBASE_H

#include <vector>
#include "dba/ptr.h"
#include "dba/conversion.h"
#include "dba/database.h"


extern "C" {
  #include <ctpublic.h>
};

/**Sybase driver for dba library
  */

namespace sybase {

const int CTLIB_VERSION = CS_VERSION_100;

struct CtxFree {
  void operator() (CS_CONTEXT* s) {
    cerr << "CtxFree: ct_exit" << endl;
    ct_exit(s,CS_UNUSED);
    cerr << "CtxFree: cs_ctx_drop" << endl;
    cs_ctx_drop(s);   
  };
};

struct ConnFree {
  void operator() (CS_CONNECTION* s) {
    cerr << "ConnFree: ct_con_drop" << endl;
    ct_con_drop(s);   
  };
};

typedef CRes<CS_CONTEXT,CtxFree> Context;
typedef CRes<CS_CONNECTION,ConnFree> Connection;

/**
  Exception thrown by library.
  Because error info is fetched in callback routines, those routines only fills static global 
  instance and then this instance is throws after exit of function that cuased error.
  This prevents population exceptions through C functions stack

  When multiple callback with errors occurs in single ctlib call only first is recorded and thrown 
*/
class SybaseException : public Exception {
    friend class Db;
  public:
    virtual ~SybaseException() throw() {};
  private:
    void setServerity(long pValue);
    void setNumber(long pValue);
    void setMessage(const char* pMsg);
  protected:
    SybaseException();
    int mSeverity;
    int mNumber;
    string mMsg;
};

class SybaseServerException : public SybaseException {
    friend class Db;
  public:
    virtual ~SybaseServerException() throw() {};
  private:
    SybaseServerException();
    void setState(long pValue);
    void setLine(long pValue);
    void setServerName(const char* pName);
    void setProcedureName(const char* pName);
    int mState;
    int mLine;
    string mProcedureName;
    string mServerName;
    void makeDesc();
};

class SybaseClientException : public SybaseException {
    friend class Db;
  public:
    virtual ~SybaseClientException() throw() {};
  private:
    SybaseClientException();  
    void setOrigin(long pValue);
    void setLayer(long pValue);
    void setOsCode(long pValue);
    void setOsInfo(const char* pValue);
    long mOrigin;
    long mLayer;
    long mOsCode;
    string mOsInfo;
    void makeDesc();
};



class SybaseResult;
//void sybase_throw(const string& error);

/**
  Sybase database class.
  First method is to add SybaseConnection->getNextResult, \
  Second (better I think) is to add SybaseResult::next() and some \
  generic api to DbResult to fetch stored procedure params \
  and exit status.
*/

class Db : public Database  {
    //FIXME add API for fetching more than one result from command.
    friend void sybase_throw();
  public:
    Db(const string& dbname);
    Db(const string& dbname,const char* home, const char* locale = NULL);
    virtual DbConnection* getConnection(const char* params);
    virtual ~Db();
    static SybaseServerException Db::sServerException;
    static SybaseClientException Db::sClientException;
  private:
    void init();
    //Open Client static error message callback handlers        
    static CS_RETCODE CS_PUBLIC cs_error(CS_CONTEXT *context, CS_CONNECTION *connection, CS_SERVERMSG *emsgp);
    static CS_RETCODE CS_PUBLIC ct_error(CS_CONTEXT *context, CS_CONNECTION *connection, CS_CLIENTMSG *emsgp);
    static CS_RETCODE CS_PUBLIC libcs_error(CS_CONTEXT* context, CS_CLIENTMSG* emsgp);

    string mvDbName;
    Context mvCtx;
};

class SybaseConnection : public DbConnection {
  public:
    SybaseConnection(const Context& pCtx, CS_CONNECTION* pConn);
    virtual DbResult* sendQuery(const char* pSql);
    virtual int sendUpdate(const char* pSql);
    virtual void begin();
    virtual void commit();
    virtual void rollback();
    virtual void disconnect();
    virtual bool isValid() const;
    ~SybaseConnection();
  private:
    Context mvCtx;
    Connection mvConn;
    CS_COMMAND* sendCommand(const char* pSql);
};

class SybaseColumn;

class SybaseResult : public DbResult {
  public:
    SybaseResult(CS_COMMAND*,const Context& pCtx);
    virtual int columns() const;
    virtual const DbColumn& getColumn(int i) const;
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
    virtual ~SybaseResult();
  private:
    static char tmp[1024];
    typedef map<int,SybaseColumn*> Columns;

    struct ComFree {
      void operator() (CS_COMMAND* s) {
        if (s != NULL) {
          cerr << "ComFree: ct_cmd_drop" << endl;
          ct_cmd_drop(s);
        };
      };
    };

    typedef CRes<CS_COMMAND,ComFree> Command;

    struct BindFree {
      void operator() (char* s) {
        free(s);
      };
    };

    struct IndFree {
      void operator() (CS_SMALLINT* s) {
        free(s);
      };
    };

    typedef CRes<char,BindFree> BindRes;
    typedef CRes<CS_SMALLINT,IndFree> IndRes;

    Command mvCmd;
    Columns mvColumns;
    BindRes mvData;   
    IndRes mvIndicators;    
    Context mvCtx;
    bool mDataFetched;
    
    SybaseColumn* findColumn(int pIndex) const;
    void prepareResult();
};

class SybaseColumn : public DbColumn {
  public:
    SybaseColumn(const CS_DATAFMT&);
    virtual const char* getName() const { return mvDataFmt.name ; };
    void setOffset(int);
    int getOffset() const;
    const CS_DATAFMT* getFormat() const;
    virtual ~SybaseColumn() {};
  private:
    const CS_DATAFMT mvDataFmt;
    int mvOffset;
};

}; //namespace sybase

#endif
