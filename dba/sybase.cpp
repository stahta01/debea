// File: sybase.cpp
// Purpose: Sybase plugin for DBA
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2002
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#include "dba/sybase.h"
#include "dba/connectstring.h"

#include <algorithm>
#include <stdlib.h>

#ifdef WIN32
  #define localtime_r(buf,val) localtime(buf)
#endif

namespace sybase {

SybaseServerException Db::sServerException;
SybaseClientException Db::sClientException;

/**
  Type of last exception recorded by callback routines. @sa error routines.
*/
typedef enum {
  SYBASE_EX_NONE = 0,
  SYBASE_EX_CLIENT,
  SYBASE_EX_SERVER
} tExType;

/**
  Type of error. @sa tExType.
*/
tExType sErrorOccured = SYBASE_EX_NONE;

char SybaseResult::tmp[1024];

void
sybase_throw(const string& error) {
  /*
    Reset global error flag to record next error
  */
  tExType tmp = sErrorOccured;
  sErrorOccured = SYBASE_EX_NONE;
  switch (tmp) {
    case SYBASE_EX_NONE:
      throw Exception(error.c_str());
    break;
    case SYBASE_EX_SERVER:
      throw Db::sServerException;
    break;
    case SYBASE_EX_CLIENT:
      throw Db::sClientException;
    break;
  };
};


class ColNameFinder {
  public:
    ColNameFinder(const char* pColName) : mName(pColName) {};
    bool operator() (const pair<int,SybaseColumn*>& pCol) {
      return (0 == strcmp(mName, pCol.second -> getName()));
    };
  private:
    const char* mName;
};

Db::Db(const string& dbname) : mvDbName(dbname) {
  init();
};

#ifndef WIN32
Db::Db(const string& dbname,const char *home,const char *locale) 
  : mvDbName(dbname) 
{
  setenv("SYBASE",home,1);
  if (locale)
    setenv("LC_ALL",locale,1);
  init();
};
#endif

CS_RETCODE
Db::cs_error(CS_CONTEXT *context, CS_CONNECTION *connection,CS_SERVERMSG *emsgp) {
  cerr << "cs_error call: " << emsgp->text << endl;
  sServerException.setServerity((long)(emsgp->severity));
  sServerException.setNumber((long)(emsgp->msgnumber));
  sServerException.setState((long)(emsgp->state));
  sServerException.setState((long)(emsgp->line));
  if (emsgp->svrnlen > 0)
    sServerException.setServerName(emsgp->svrname);
  else
    sServerException.setServerName("");

  if (emsgp->proclen > 0)
    sServerException.setProcedureName(emsgp->proc);
  else
    sServerException.setProcedureName("");

  sServerException.setDesc(emsgp->text);

  sErrorOccured = SYBASE_EX_SERVER;
  return CS_SUCCEED;
};

CS_RETCODE
Db::ct_error(CS_CONTEXT *context, CS_CONNECTION *connection, CS_CLIENTMSG *emsgp) {
  cerr << "ct_error call: " << emsgp->msgstring << endl;
  sClientException.setServerity((long)CS_SEVERITY(emsgp->severity));
  sClientException.setNumber((long)CS_NUMBER(emsgp->severity));
  sClientException.setOrigin((long)CS_ORIGIN(emsgp->severity));
  sClientException.setLayer((long)CS_LAYER(emsgp->severity));
  sClientException.setDesc(emsgp->msgstring);
  if (emsgp->osstringlen > 0) {
    sClientException.setOsCode(emsgp->osnumber);
    sClientException.setOsInfo(emsgp->osstring);
  } else {
    sClientException.setOsCode(0);
    sClientException.setOsInfo("");
  };
  sErrorOccured = SYBASE_EX_CLIENT;
  return CS_SUCCEED;
};

CS_RETCODE
Db::libcs_error(CS_CONTEXT* context, CS_CLIENTMSG* emsgp) {
  cerr << "cs_error" << endl;
  ct_error(context,NULL,emsgp);
  return CS_SUCCEED;
};

#if 0
Pointer<DbTable>
Db::getTable(string name) {
  /*any better method instead of sending dummy query to database?*/
  Pointer<DbResult> res = sendQuery("SELECT c.column_name,c.width,c.pkey FROM syscolumn c, systable t"
                  " WHERE c.table_id = t.table_id AND t.table_name = '" + name +
                  "' ORDER BY c.width");

  SybaseTable* table = new SybaseTable(name);
  int index = 0;
  while (res -> fetchRow()) {
    DbColumn* col = new DbColumn((*res)[0],(*res)[1]);
    table -> addColumn(index,col);
    if ((string)((*res)[2]) == "Y")
      table -> addKey(index);
    index++;
  };
  return Pointer<DbTable>(table);
};
#endif


void
Db::init() {
#ifndef WIN32
  setenv("DSQUERY",mvDbName.c_str(),1);
#else
  string s("DSQUERY=");
  s += mvDbName;
  putenv(s.c_str());
#endif
  //initialize context
  CS_CONTEXT* ctx = NULL;
  cerr << "init(): cs_ctx_alloc()" << endl;
  int retcode = cs_ctx_alloc(CTLIB_VERSION,&ctx);
  if (retcode != CS_SUCCEED)
    sybase_throw("Cannot create database object, cs_ctx_alloc() returned "+ toStr(retcode));

  retcode = ct_init(ctx,CTLIB_VERSION);
  if (retcode != CS_SUCCEED) {
    cs_ctx_drop(ctx);   
    sybase_throw("Cannot create database object, ct_init() returned "+ toStr(retcode));
  }

#ifdef SYBASE_API_DEBUG
  retcode = ct_debug(ctx,NULL,CS_SET_FLAG,CS_DEBUG_API_STATES,NULL,CS_UNUSED);
  if (retcode != CS_SUCCEED) {
    cs_ctx_drop(ctx);   
    sybase_throw("Cannot create database object, ct_debug() returned "+ toStr(retcode));
  }
#endif

  //set error handlers  

if (cs_config(ctx, CS_SET, CS_MESSAGE_CB, (CS_VOID*)libcs_error,  CS_UNUSED, NULL) != CS_SUCCEED) {
    cs_ctx_drop(ctx);   
    sybase_throw("Cannot create database object, ct_config() returned "+ toStr(retcode));
  };


  cerr << "init(): ct_callback(CLIENTMSG)" << endl;
  retcode = ct_callback(ctx,NULL,CS_SET,CS_CLIENTMSG_CB,(CS_VOID *)ct_error);
  if (retcode != CS_SUCCEED) {
    cs_ctx_drop(ctx);   
    sybase_throw("Cannot create database object, ct_callback() returned "+ toStr(retcode));
  }
        
  cerr << "init(): ct_callback(SERVERMSG)" << endl;
  retcode = ct_callback(ctx,NULL,CS_SET,CS_SERVERMSG_CB,(CS_VOID *)cs_error);
  if (retcode != CS_SUCCEED) {
    cs_ctx_drop(ctx);   
    sybase_throw("Cannot create database object, ct_callback() returned "+ toStr(retcode));
  }
  
  //set connection timeout
  int timeout = 60;
//  cerr << "init(): ct_cconfig(TIMEOUT)" << endl;
  retcode = ct_config(ctx,CS_SET,CS_TIMEOUT,(CS_VOID*)&timeout,CS_UNUSED,NULL);
  if (retcode != CS_SUCCEED) {
    cs_ctx_drop(ctx);   
    sybase_throw("Cannot set timeout. ct_config returned "+ toStr(retcode));
  }

//  cerr << "init(): ct_cconfig(LOGIN_TIMEOUT)" << endl;
  retcode = ct_config(ctx,CS_SET,CS_LOGIN_TIMEOUT,(CS_VOID*)&timeout,CS_UNUSED,NULL);
  if (retcode != CS_SUCCEED) {
    cs_ctx_drop(ctx);   
    sybase_throw("Cannot set login timeout, ct_config() returned "+ toStr(retcode));
  }
  //store ctx pointer into copy-proof resource class
  mvCtx = ctx;  
};

DbConnection*
Db::getConnection(const char* pParams) {
  CS_CONNECTION* conn;
  
  ConnectString str(pParams);
  //callback error routine will throw execption, return is only for safety
//  cerr << "connect(): ct_con_alloc" << endl;
  if (ct_con_alloc(mvCtx,&conn) != CS_SUCCEED)
    sybase_throw("ct_conn_alloc() failed");

  cerr << "connect(): ct_con_props(USERNAME)" << endl;
  if (ct_con_props(conn,CS_SET,CS_USERNAME,(CS_CHAR*)str.username().c_str(),CS_NULLTERM,NULL) != CS_SUCCEED)
    sybase_throw("setting username conection proprtity failed");
          
  cerr << "connect(): ct_con_props(PASSWORD)" << endl;
  if (ct_con_props(conn,CS_SET,CS_PASSWORD,(CS_CHAR*)str.password().c_str(),CS_NULLTERM,NULL) != CS_SUCCEED)
    sybase_throw("setting password proprtity failed");
  
  cerr << "connect(): ct_con_props(APPNAME)" << endl;
  if (ct_con_props(conn,CS_SET,CS_APPNAME,(CS_VOID*)"DBA C++ library",CS_NULLTERM,NULL) != CS_SUCCEED)
    sybase_throw("settion appilcation name properity failed");
  
  cerr << "connect(): ct_connect" << endl;
  if (ct_connect(conn,(CS_CHAR*)str.dbname().c_str(),0) != CS_SUCCEED)
    sybase_throw("connection to database failed");
  return new SybaseConnection(mvCtx, conn); 
};

SybaseConnection::SybaseConnection(const Context& pCtx, CS_CONNECTION* pConn)
  : mvCtx(pCtx),
    mvConn(pConn)
{
  
};

int
SybaseConnection::sendUpdate(const char* pSql) {
  CS_COMMAND* cmd = sendCommand(pSql);
  
  CS_INT result_type;
  typedef enum error_code {
    NONE,
    RES_INFO_FAILED,
    DESCRIBE_FAILED,
    RESULTS_FAILED,
    NOMEM,
    CMD_FAIL
  };
  error_code error = NONE;
  int ret;
  CS_INT rows = 0;
  while((ret = ct_results(cmd,&result_type)) == CS_SUCCEED) {
//    cerr << "SybaseResult(): ct_results" << endl;
    switch(result_type) {
      case CS_ROW_RESULT: 
        cerr << "CS_ROW_RESULT after update?" << endl;
      break;
      case CS_CMD_FAIL:
        error = CMD_FAIL;
      break;
      case CS_CMD_DONE:
        cerr << " all results done " << endl;
        //all results done.   
      case CS_CMD_SUCCEED: {
        int code = ct_res_info(cmd,CS_ROW_COUNT,&rows,CS_UNUSED,NULL);
        if (code != CS_SUCCEED)
          error = RES_INFO_FAILED;
      } break;
      default:
        cerr << " unknown result type "+ toStr(result_type) +" fetched " << endl;
      break;
    };  
  };

  if (ret != CS_END_RESULTS)
    error = RESULTS_FAILED;

  switch(error) {
    case NONE: {
    } break;
    case    RES_INFO_FAILED:
      sybase_throw("ct_res_info() failed");
    break;
    case    DESCRIBE_FAILED:
      sybase_throw("ct_describe() failed"); 
    break;
    case    NOMEM:
      sybase_throw("Memory allocation error in Result constructor");
    break;
    case CMD_FAIL:
      sybase_throw("Update failed");
    break;
    case RESULTS_FAILED:
      sybase_throw("Results failed");
    break;
    default:
      sybase_throw("Unknown error");
    break;
  };
  return rows;
};

//FIXME   implementation
void 
SybaseConnection::begin() {

};

//FIXME implementation
void 
SybaseConnection::commit() {

};

//FIXME implementation
void 
SybaseConnection::rollback() {

};

//FIXME implementation
void 
SybaseConnection::disconnect() {

};

//FIXME implementation
bool 
SybaseConnection::isValid() const {
  return true;
};


SybaseConnection::~SybaseConnection() {
  cerr << "disconnect(): ct_disconnect" << endl;
  ct_close(mvConn,CS_UNUSED);
};


CS_COMMAND*
SybaseConnection::sendCommand(const char* pSql) {
  CS_COMMAND* cmd;

  cerr << "Sybase::sendQuery: sending " << pSql << endl;
//  cerr << "sendQuery(): ct_cmd_alloc" << endl;
  if (ct_cmd_alloc(mvConn,&cmd) != CS_SUCCEED)
    sybase_throw("Cannot allocate ctlib command structure");
    
//  cerr << "sendQuery(): ct_command" << endl;
  if (ct_command(cmd,CS_LANG_CMD,(CS_CHAR*)pSql,CS_NULLTERM,CS_UNUSED) != CS_SUCCEED)
    sybase_throw("Cannot create command, ct_command() failed");
  
//  cerr << "sendQuery(): ct_send" << endl;
  if (ct_send(cmd) != CS_SUCCEED)
    sybase_throw("ct_send() failed");
  return cmd;
};

Ptr<DbResult>
SybaseConnection::sendQuery(const char* pSql) {
  return new SybaseResult(sendCommand(pSql),mvCtx);
}

#if 0
Pointer<DbTransaction>
Db::createTransaction() {
  sybase_throw("Sybase createTransaction: TODO");
};
#endif


void
SybaseResult::prepareResult() {
  CS_INT result_type;
  char *databuf = NULL;
  CS_SMALLINT *ind = NULL;
  mDataFetched = false;
  typedef enum error_code {
    NONE,
    RES_INFO_FAILED,
    DESCRIBE_FAILED,
    RESULTS_FAILED,
    NOMEM,
    CMD_FAIL
  };
  error_code error = NONE;
  int ret;
  ret = ct_results(mvCmd,&result_type);
  if (ret == CS_SUCCEED) {
//    cerr << "SybaseResult(): ct_results" << endl;
    switch(result_type) {
      case CS_ROW_RESULT: {
        int columns;
        CS_DATAFMT data_format;
//        cerr << "SybaseResult(): ct_resinfo" << endl;
        if (ct_res_info(mvCmd,CS_NUMDATA,&columns,CS_UNUSED,NULL) != CS_SUCCEED)
          error = RES_INFO_FAILED;

        int offset = 0;
        for(int i = 0; i < columns; i++) {
//          cerr << "SybaseResult(): ct_describe" << endl;
          if (ct_describe(mvCmd,i+1,&data_format) != CS_SUCCEED)
            error = DESCRIBE_FAILED;
//          cerr << "column " << i << " type "<< data_format.datatype << " datalen " << data_format.maxlength << endl;
          switch(data_format.datatype) {
            case CS_CHAR_TYPE:
              data_format.datatype = CS_LONGCHAR_TYPE;
            case CS_LONGCHAR_TYPE:
              data_format.format = CS_FMT_NULLTERM;
              data_format.maxlength += 1;
            break;  
            case CS_BIT_TYPE:
            break;
            case CS_DATETIME_TYPE:
            case CS_DATETIME4_TYPE:
            break;
            case CS_TINYINT_TYPE:
            break;
            case CS_SMALLINT_TYPE:
            break;
            case CS_INT_TYPE:
            break;
            case CS_REAL_TYPE:
            break;
            case CS_FLOAT_TYPE:
            break;
            case CS_NUMERIC_TYPE:
            case CS_DECIMAL_TYPE:
              data_format.datatype = CS_CHAR_TYPE;
              data_format.format = CS_FMT_NULLTERM;
              data_format.maxlength = data_format.precision + 1;
            break;
            default:
            break;
          };
          SybaseColumn* s = new SybaseColumn(data_format);
          s->setOffset(offset);
//          cerr << "setting offset of column " << i << " to " << offset << endl;
          offset += data_format.maxlength;
          mvColumns.insert(make_pair(i,s));
        };
        databuf = (char *)malloc(offset);
        void *pos = databuf;
//        cerr << "allocated " << offset << " bytes for bind data at " << pos << endl;
        ind = (CS_SMALLINT*)malloc(sizeof(CS_SMALLINT)*columns);
        pos = ind;
//        cerr << "allocated " << sizeof(CS_SMALLINT)*columns << " bytes for indicators at " << pos << endl;
        if ((!databuf)||(!ind)) {
          error = NOMEM;
        } else {
          mvData = databuf;
          mvIndicators = ind;

          //bind memory locations for fetch
          for(int i = 0; i < columns; i++) {
//            void *bpos = databuf+mvColumns[i]->getOffset();
//            void *ipos = mvIndicators+i;
//            cerr << "binding column " << i << " at pos " << bpos << " ind " << ipos <<endl;
            ct_bind(mvCmd,i+1,(CS_DATAFMT*)(mvColumns[i]->getFormat()),databuf+mvColumns[i]->getOffset(),NULL,mvIndicators+i);
          };
        };
      } break;
      case CS_CMD_SUCCEED:
        //no rows
      break;
      case CS_CMD_FAIL:
        error = CMD_FAIL;
      break;
      case CS_CMD_DONE:
        //error?
      break;
      default:
        cerr << " unknown result type "+ toStr(result_type) +" fetched " << endl;
      break;
    };  
  };

//  if (ret != CS_END_RESULTS)
//    error = RESULTS_FAILED;

  switch(error) {
    case NONE: {
    } break;
    case    RES_INFO_FAILED:
      sybase_throw("ct_res_info() failed");
    break;
    case    DESCRIBE_FAILED:
      sybase_throw("ct_describe() failed"); 
    break;
    case    NOMEM:
      sybase_throw("Memory allocation error in Result constructor");
    break;
    case CMD_FAIL:
      sybase_throw("Command failed");
    break;
    case RESULTS_FAILED:
      sybase_throw("Results failed");
    break;
    default:
      sybase_throw("Unknown error");
    break;
  };
};

SybaseResult::SybaseResult(CS_COMMAND* cmd,const Context& ctx) 
  : mvCmd(cmd), 
    mvCtx(ctx),
    mvIndicators(NULL),
    mvData(NULL)
{
  prepareResult();
};

int
SybaseResult::columns() const {
  return mvColumns.size();
}

const DbColumn&
SybaseResult::getColumn(int i) const {
  Columns::const_iterator it = mvColumns.find(i);
  if (it == mvColumns.end())
    throw DatabaseException("Column not found");
  return *(it -> second);
};

bool
SybaseResult::fetchRow() {
  CS_INT count;
//  cerr << "fetchRow(): ct_fetch" << endl;
  switch(ct_fetch(mvCmd,CS_UNUSED,CS_UNUSED,CS_UNUSED,&count)) {
    case CS_SUCCEED:
      return true;    
    break;
    case CS_END_DATA: {
      mDataFetched = true;
      return false;
    } break;
  };
  throw Exception("Error fetching rows");   
};

SybaseColumn*
SybaseResult::findColumn(int pIndex) const {
  Columns::const_iterator it = mvColumns.find(pIndex);
  if (it == mvColumns.end())
    throw DatabaseException("Column not found");
  return it -> second;
};

const char* 
SybaseResult::getString(const char* pField) const {
  Columns::const_iterator it = find_if(mvColumns.begin(), mvColumns.end(), ColNameFinder(pField));
  if (it == mvColumns.end())
    throw DatabaseException("Column not found (by name)");
  return getString(it -> first);
};

const char* 
SybaseResult::getString(int pField) const {
  SybaseColumn* col = findColumn(pField);
  void* ptr = mvData + col -> getOffset();
  const CS_DATAFMT& fmt = *(col -> getFormat());
  
  switch(fmt.datatype) {
    case CS_BINARY_TYPE:
    case CS_LONGCHAR_TYPE:
    case CS_CHAR_TYPE:
      return (const char *)ptr;
    break;  
    /*
      Unsinged char with 0 for false and 1 for true - just return it
    */
    case CS_BIT_TYPE: {
      return (const char *)ptr;
    } break;
    case CS_DATETIME_TYPE:
    case CS_DATETIME4_TYPE: {
      struct tm out;
      CS_DATEREC mid;
      cs_dt_crack(mvCtx,fmt.datatype,ptr,&mid);
      out.tm_sec = mid.datesecond;
      out.tm_min = mid.dateminute;
      out.tm_hour = mid.datehour;
      out.tm_mday = mid.datedmonth;
      out.tm_mon = mid.datemonth;
      out.tm_year = mid.dateyear - 1900;
      out.tm_wday = mid.datedweek;
      out.tm_yday = mid.datedyear - 1;
      int ret = strftime(tmp,1024,"%Y-%m-%d %H:%M:%S",&out);
      if (ret == 0)
        throw DatabaseException("strftime failed");
      return tmp;
    } break;
    case CS_TINYINT_TYPE: {
      CS_TINYINT* val = (CS_TINYINT*)ptr;
      convert(*val,tmp);
      return tmp;
    } break;
    case CS_SMALLINT_TYPE: {
      CS_SMALLINT* val = (CS_SMALLINT*)ptr;
      convert(*val,tmp);
      return tmp;
    } break;
    case CS_INT_TYPE: {
      CS_INT* val = (CS_INT*)ptr;
      convert(*val,tmp);
      return tmp;
    } break;
    case CS_REAL_TYPE: {
      CS_REAL* val = (CS_REAL*)ptr;
      convert(*val,tmp);
      return tmp;
    } break;
    case CS_FLOAT_TYPE: {
      CS_FLOAT* val = (CS_FLOAT*)ptr;
      convert(*val,tmp);
      return tmp;
    } break;
    case CS_NUMERIC_TYPE:
    case CS_DECIMAL_TYPE: {
      CS_NUMERIC* val = (CS_NUMERIC*)ptr;
//      snprintf(tmp,1024,"%d.%d",
      return (const char*)val -> array;
    } break;
  };
  throw Exception(("Datatype " + toStr(fmt.datatype) + " not supported").c_str());
};

long 
SybaseResult::getInt(const char* pField) const {
  Columns::const_iterator it = find_if(mvColumns.begin(), mvColumns.end(), ColNameFinder(pField));
  if (it == mvColumns.end())
    throw DatabaseException("Column not found (by name)");
  return getInt(it -> first);
};

long 
SybaseResult::getInt(int pField) const {
  SybaseColumn* col = findColumn(pField);
  void* ptr = mvData + col -> getOffset();
  const CS_DATAFMT& fmt = *(col -> getFormat());
  
  switch(fmt.datatype) {
    case CS_LONGCHAR_TYPE:
    case CS_CHAR_TYPE:
      long l;
      convert((const char *)ptr,l);
      return l;
    break;  
    /*
      Unsinged char with 0 for false and 1 for true - just return it
    */
    case CS_BIT_TYPE: {
      if (*(unsigned char*)ptr == 0)
        return 0;     
      return 1;
    } break;
    case CS_DATETIME_TYPE:
    case CS_DATETIME4_TYPE: {
      struct tm out;
      CS_DATEREC mid;
      cs_dt_crack(mvCtx,fmt.datatype,ptr,&mid);
      out.tm_sec = mid.datesecond;
      out.tm_min = mid.dateminute;
      out.tm_hour = mid.datehour;
      out.tm_mday = mid.datedmonth;
      out.tm_mon = mid.datemonth;
      out.tm_year = mid.dateyear - 1900;
      out.tm_wday = mid.datedweek;
      out.tm_yday = mid.datedyear - 1;
      return mktime(&out);
    } break;
    case CS_TINYINT_TYPE: {
      CS_TINYINT* val = (CS_TINYINT*)ptr;
      return *val;
    } break;
    case CS_SMALLINT_TYPE: {
      CS_SMALLINT* val = (CS_SMALLINT*)ptr;
      return *val;
    } break;
    case CS_INT_TYPE: {
      CS_INT* val = (CS_INT*)ptr;
      return *val;
    } break;
    case CS_REAL_TYPE: {
      CS_REAL* val = (CS_REAL*)ptr;
      return (long)*val;
    } break;
    case CS_FLOAT_TYPE: {
      CS_FLOAT* val = (CS_FLOAT*)ptr;
      return (long)*val;
    } break;
    case CS_NUMERIC_TYPE:
    case CS_DECIMAL_TYPE: {
      CS_NUMERIC* val = (CS_NUMERIC*)ptr;
      long l;
      convert((const char*)val -> array,l);
      return l;
    } break;
  };
  throw Exception(("Datatype " + toStr(fmt.datatype) + " not supported").c_str());
};

double 
SybaseResult::getDouble(const char* pField) const {
  Columns::const_iterator it = find_if(mvColumns.begin(), mvColumns.end(), ColNameFinder(pField));
  if (it == mvColumns.end())
    throw DatabaseException("Column not found (by name)");
  return getDouble(it -> first);
};

double 
SybaseResult::getDouble(int pField) const {
  SybaseColumn* col = findColumn(pField);
  void* ptr = mvData + col -> getOffset();
  const CS_DATAFMT& fmt = *(col -> getFormat());
  
  switch(fmt.datatype) {
    case CS_LONGCHAR_TYPE:
    case CS_CHAR_TYPE:
      double l;
      convert((const char *)ptr,l);
      return l;
    break;  
    /*
      Unsinged char with 0 for false and 1 for true - just return it
    */
    case CS_BIT_TYPE: {
      if (*(unsigned char*)ptr == 0)
        return 0;     
      return 1;
    } break;
    case CS_DATETIME_TYPE:
    case CS_DATETIME4_TYPE: {
      struct tm out;
      CS_DATEREC mid;
      cs_dt_crack(mvCtx,fmt.datatype,ptr,&mid);
      out.tm_sec = mid.datesecond;
      out.tm_min = mid.dateminute;
      out.tm_hour = mid.datehour;
      out.tm_mday = mid.datedmonth;
      out.tm_mon = mid.datemonth;
      out.tm_year = mid.dateyear - 1900;
      out.tm_wday = mid.datedweek;
      out.tm_yday = mid.datedyear - 1;
      return mktime(&out);
    } break;
    case CS_TINYINT_TYPE: {
      CS_TINYINT* val = (CS_TINYINT*)ptr;
      return *val;
    } break;
    case CS_SMALLINT_TYPE: {
      CS_SMALLINT* val = (CS_SMALLINT*)ptr;
      return *val;
    } break;
    case CS_INT_TYPE: {
      CS_INT* val = (CS_INT*)ptr;
      return *val;
    } break;
    case CS_REAL_TYPE: {
      CS_REAL* val = (CS_REAL*)ptr;
      return (long)*val;
    } break;
    case CS_FLOAT_TYPE: {
      CS_FLOAT* val = (CS_FLOAT*)ptr;
      return (long)*val;
    } break;
    case CS_NUMERIC_TYPE:
    case CS_DECIMAL_TYPE: {
      CS_NUMERIC* val = (CS_NUMERIC*)ptr;
      long l;
      convert((const char*)val -> array,l);
      return l;
    } break;
  };
  throw Exception(("Datatype " + toStr(fmt.datatype) + " not supported").c_str());
};

struct tm 
SybaseResult::getDate(const char* pField) const {
  Columns::const_iterator it = find_if(mvColumns.begin(), mvColumns.end(), ColNameFinder(pField));
  if (it == mvColumns.end())
    throw DatabaseException("Column not found (by name)");
  return getDate(it -> first);
};
    
struct tm 
SybaseResult::getDate(int pField) const {
  SybaseColumn* col = findColumn(pField);
  void* ptr = mvData + col -> getOffset();
  const CS_DATAFMT& fmt = *(col -> getFormat());
  
  struct tm out;
  switch(fmt.datatype) {
    case CS_LONGCHAR_TYPE:
    case CS_CHAR_TYPE:
      return parseISODateTime((const char *)ptr);
    break;  
    /*
      Unsinged char with 0 for false and 1 for true - just return it
    */
    case CS_BIT_TYPE: {
      throw DatabaseException("Conversion from CS_BIT_TYPE to struct tm unsupported");
    } break;
    case CS_DATETIME_TYPE:
    case CS_DATETIME4_TYPE: {
      CS_DATEREC mid;
      cs_dt_crack(mvCtx,fmt.datatype,ptr,&mid);
      out.tm_sec = mid.datesecond;
      out.tm_min = mid.dateminute;
      out.tm_hour = mid.datehour;
      out.tm_mday = mid.datedmonth;
      out.tm_mon = mid.datemonth;
      out.tm_year = mid.dateyear - 1900;
      out.tm_wday = mid.datedweek;
      out.tm_yday = mid.datedyear - 1;
      return out;
    } break;
    case CS_TINYINT_TYPE: {
      CS_TINYINT* val = (CS_TINYINT*)ptr;
      localtime_r((const time_t*)val,&out);
      return out;
    } break;
    case CS_SMALLINT_TYPE: {
      CS_SMALLINT* val = (CS_SMALLINT*)ptr;
      localtime_r((const time_t*)val,&out);
      return out;
    } break;
    case CS_INT_TYPE: {
      CS_INT* val = (CS_INT*)ptr;
      localtime_r((const time_t*)val,&out);
      return out;
    } break;
    case CS_REAL_TYPE: {
      CS_REAL* val = (CS_REAL*)ptr;
      localtime_r((long*)val,&out);
      return out;
    } break;
    case CS_FLOAT_TYPE: {
      CS_FLOAT* val = (CS_FLOAT*)ptr;
      localtime_r((long*)val,&out);
      return out;
    } break;
    case CS_NUMERIC_TYPE:
    case CS_DECIMAL_TYPE: {
      CS_NUMERIC* val = (CS_NUMERIC*)ptr;
      long l;
      convert((const char*)val -> array,l);
      localtime_r((long*)val,&out);
      return out;
    } break;
  };
  throw Exception(("Datatype " + toStr(fmt.datatype) + " not supported").c_str());
};
    
bool 
SybaseResult::isNull(const char* pField) const {
  Columns::const_iterator it = find_if(mvColumns.begin(), mvColumns.end(), ColNameFinder(pField));
  if (it == mvColumns.end())
    throw DatabaseException("Column not found (by name)");
  return isNull(it -> first);
};
    
bool 
SybaseResult::isNull(int pField) const {
  if (*(mvIndicators + pField) == 0)
    return false;
  return true;
};


SybaseResult::~SybaseResult() {
  int ret;
  CS_INT result_type;
  if (!mDataFetched) {
    ct_cancel(NULL,mvCmd,CS_CANCEL_CURRENT);
  };
  while((ret = ct_results(mvCmd,&result_type)) == CS_SUCCEED) {
//  cerr << "fetchRow(): ct_results" << endl;
    switch(result_type) {
      case CS_ROW_RESULT:
        cerr << "~SybaseResult: got rows" << endl;
        while (fetchRow());
          ;
      break;
      case CS_CMD_SUCCEED:
        //no rows?
      break;
      case CS_CMD_FAIL:
        sybase_throw("Result failed after fetchRow()");
      break;
      case CS_CMD_DONE:
        cerr << " result done " << endl;
        //all results done.   
      break;
      case CS_PARAM_RESULT:
        cerr << "param result" << endl;
        while (fetchRow());
          ;
      break;
      case CS_STATUS_RESULT:
        cerr << "status result" << endl;
        while (fetchRow())
          ;
      break;
      default:
        cerr << " unknown result type "+ toStr(result_type) +" fetched " << endl;
      break;
    };  
  };
};

#if 0
DbField
SybaseResult::operator[] (int i) {
  bool null;
  if (*(mvIndicators + i) == 0)
    null = false;
  else
    null = true;

  void* ptr = mvData + mvColumns[i]->getOffset();
  double dbl;
  long intval;
  CS_DATAFMT dstfmt;
  
  struct tm date;       
  const CS_DATAFMT& fmt = *(mvColumns[i]->getFormat());
  switch(fmt.datatype) {
    case CS_LONGCHAR_TYPE:
    case CS_CHAR_TYPE:
      return DbField((char *)ptr,null);
    break;  
    case CS_BIT_TYPE: {
      unsigned char i = *(unsigned char *)ptr;      
      return DbField((long)i,null);   
    } break;
    case CS_DATETIME_TYPE:
    case CS_DATETIME4_TYPE: {
      struct tm out;
      CS_DATEREC mid;
      cs_dt_crack(mvCtx,fmt.datatype,ptr,&mid);
      out.tm_sec = mid.datesecond;
      out.tm_min = mid.dateminute;
      out.tm_hour = mid.datehour;
      out.tm_mday = mid.datedmonth;
      out.tm_mon = mid.datemonth;
      out.tm_year = mid.dateyear - 1900;
      out.tm_wday = mid.datedweek;
      out.tm_yday = mid.datedyear - 1;
      return DbField(&out,null);
    } break;
    case CS_TINYINT_TYPE: {
      CS_TINYINT val = *(CS_TINYINT*)ptr;
      return DbField((long)val,null);
    } break;
    case CS_SMALLINT_TYPE: {
      CS_SMALLINT val = *(CS_SMALLINT*)ptr;
      return DbField((long)val,null);
    } break;
    case CS_INT_TYPE: {
      CS_INT val = *(CS_INT*)ptr;
      return DbField((long)val,null);
    } break;
    case CS_REAL_TYPE: {
      CS_REAL val = *(CS_REAL*)ptr;
      return DbField((double)val,null);
    } break;
    case CS_FLOAT_TYPE: {
      CS_FLOAT val = *(CS_FLOAT*)ptr;
      return DbField((double)val,null);
    } break;
    case CS_NUMERIC_TYPE:
    case CS_DECIMAL_TYPE:
      sybase_throw("Fatal: Numeric datatype spotted, should be converted to char");
    break;
    default:
      sybase_throw("Datatype " + toString(fmt.datatype) + " not supported");
    break;
  };
};
#endif



SybaseColumn::SybaseColumn(const CS_DATAFMT& data) 
  : mvDataFmt(data)
{
}

int
SybaseColumn::getOffset() const {
  return mvOffset;
};

void
SybaseColumn::setOffset(int offset) {
  mvOffset = offset;
};

const CS_DATAFMT*
SybaseColumn::getFormat() const {
  return &mvDataFmt;
};

Db::~Db() {
//FIXME: redesign remove.
//  if (mvState == DB_CONNECTED)
//    disconnect();
}

SybaseException::SybaseException() 
{
  mSeverity = CS_SV_API_FAIL;
//  mvOrigin = 0;
//  mvLayer = 99;
  mNumber = 0;
};

void 
SybaseException::setServerity(long pValue) {
  mSeverity = pValue;
};

void 
SybaseException::setNumber(long pValue) {
  mNumber = pValue;
};

void 
SybaseException::setMessage(const char* pMsg) {
  mMsg = pMsg;
};


SybaseServerException::SybaseServerException() 
{
};

void
SybaseServerException::makeDesc() {
  string s("Server error:\n");
  s += "Server name: " + mServerName + "\n";
  s += "Severity: " + toStr(mSeverity) + " Number: " + toStr(mNumber) + " State: " + toStr(mState) + " Line: " + toStr(mLine) + "\n";
  if (mProcedureName.size())
    s += "Procedure name: " + mProcedureName + "\n";
  s += "Description:\n" + mMsg + "\n";
  setDesc(s.c_str());
};

void 
SybaseServerException::setState(long pValue) {
  mState = pValue;
  makeDesc();
};

void 
SybaseServerException::setLine(long pValue) {
  mLine = pValue;
  makeDesc();
};

void 
SybaseServerException::setServerName(const char* pName) {
  mServerName = pName;
  makeDesc();
};

void 
SybaseServerException::setProcedureName(const char* pName) {
  mProcedureName = pName;
  makeDesc();
};

SybaseClientException::SybaseClientException()
{
};

void
SybaseClientException::makeDesc() {
  string s("CtLib error:\n");
  s += "Severity: " + toStr(mSeverity) + " Number: " + toStr(mNumber) + " Origin: " + toStr(mOrigin) + " Layer: " + toStr(mLayer) + "\n";
  s += "Description:\n" + mMsg + "\n";
  if (mOsInfo.size()) {
    s += "OsCode: " + toStr(mOsCode) + ", info:\n";
    s += mOsInfo;
  };
  setDesc(s.c_str());
};

void 
SybaseClientException::setOrigin(long pValue) {
  mOrigin = pValue;
  makeDesc();
};

void 
SybaseClientException::setLayer(long pValue) {
  mLayer = pValue;
  makeDesc();
};

void 
SybaseClientException::setOsCode(long pValue) {
  mOsCode = pValue;
  makeDesc();
};

void 
SybaseClientException::setOsInfo(const char* pValue) {
  mOsInfo = pValue;
  makeDesc();
};



}; //namespace sybase
