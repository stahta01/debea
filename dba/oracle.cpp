// File: oracle.cpp
// Purpose: Oracle plugin for DBA (not maintained, needs update)
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2001
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#include "dba/oracle.h"

namespace oracle {

/*DbOracle*/
DbOracle::DbOracle(string name, string home) {
  if (!home.empty()) {
    setenv("ORACLE_HOME",home.c_str(),1);   
  }
  init(name);
};

DbOracle::DbOracle(string name) {
  init(name);
};

void
DbOracle::init(string name) {
  mvName = name;
  OCIError* err = NULL;
  OCIEnv* env = NULL;
  sword ret = OCIEnvCreate(&env,OCI_OBJECT,0,0,0,0,0,0);
  if (ret != OCI_SUCCESS)
    throw OracleException("Cannot allocate enviroment handle. OCIHandleAlloc returns " + toString(ret));

  ret = OCIHandleAlloc((dvoid *)env,(dvoid **)&err,OCI_HTYPE_ERROR,0,0);
  if (ret != OCI_SUCCESS)
    throw OracleException("Cannot allocate error handle. OCIHandleAlloc returns " + toString(ret));
  mEnv = env;
  mErr = err;
};

Pointer<DbTable>
DbOracle::getTable(string name) {
  if (mvState != DB_CONNECTED)
    throw OracleException("Cannot get table description when not connected to database");
  
  int ret;
  OCIDescribe* hDesc;
  
  ret = OCIHandleAlloc(mEnv,&(void *)hDesc,OCI_HTYPE_DESCRIBE,0,0);
  if (ret != OCI_SUCCESS)
    throw OracleException(mErr,ret);

  ret = OCIDescribeAny(mCtx,mErr,(void*)name.c_str(),name.length(),OCI_OTYPE_NAME,0,OCI_PTYPE_TABLE,hDesc);
  if (ret != OCI_SUCCESS)
    throw OracleException(mErr,ret);

  OCIParam* paramh;
  ret = OCIAttrGet(hDesc,OCI_HTYPE_DESCRIBE,&paramh,0,OCI_ATTR_PARAM,mErr);
  if (ret != OCI_SUCCESS)
    throw OracleException(mErr,ret);

  ub2 numcols = 0;
  ret = OCIAttrGet(paramh,OCI_DTYPE_PARAM,&numcols,0,OCI_ATTR_NUM_COLS,mErr);
  if (ret != OCI_SUCCESS)
    throw OracleException(mErr,ret);

  OCIParam* collisth;
  ret = OCIAttrGet(paramh,OCI_DTYPE_PARAM,&collisth,0,OCI_ATTR_LIST_COLUMNS,mErr);
  if (ret != OCI_SUCCESS)
    throw OracleException(mErr,ret);

  /*I don't know how to fetch primary key information using OCI calls :( */
  /*so here we send query that fetch it for us*/
  Pointer<DbResult> res = sendQuery("SELECT position FROM"
                                    " user_cons_columns cols, user_constraints c"
                                    " WHERE c.table_name = '"+name+"' AND c.constraint_type = 'P'"
                                    " AND cols.constraint_name = c.constraint_name");
  if(!res -> fetchRow())
    throw OracleException("cannot fetch primary key info for table " + name);

  OracleTable* table = new OracleTable(name);

  do {
    table -> addKey((*res)[0]);
  } while (res -> fetchRow());


  for (int i = 0; i < numcols; i++) {
    OCIParam* colh;
    text* colName;
    int colSize;
    ret = OCIParamGet(collisth,OCI_DTYPE_PARAM,mErr,&(void*)colh,i+1);
    if (ret != OCI_SUCCESS)
      throw OracleException(mErr,ret);

    ret = OCIAttrGet(colh,OCI_DTYPE_PARAM,&colName,0,OCI_ATTR_NAME,mErr);
    if (ret != OCI_SUCCESS)
      throw OracleException(mErr,ret);
    ret = OCIAttrGet(colh,OCI_DTYPE_PARAM,&colSize,0,OCI_ATTR_DATA_SIZE,mErr);
    if (ret != OCI_SUCCESS)
      throw OracleException(mErr,ret);
      
    table -> addColumn(i,new DbColumn(string((char*)colName),colSize));
  };
  return Pointer<DbTable>(table);
};

void
DbOracle::connect(string username, string password) {
  Database::connect(username,password);
  sword ret;
  OCISvcCtx* o_ctx = NULL;    
  ret = OCILogon(mEnv,mErr,&o_ctx,
    (CONST text *)username.c_str(),
    username.length(),
    (CONST text *)password.c_str(),
    password.length(),
    (CONST text *)mvName.c_str(),
    mvName.length()
  );

  if (ret != OCI_SUCCESS)
    throw OracleException(mErr,ret);
  mCtx = o_ctx;
};

Pointer<DbResult>
DbOracle::sendQuery(string sql) {
  sword ret;
  OCIStmt *o_stmt = NULL;

  cerr << sql << endl;

  /*Allocate statement handle*/
  ret = OCIHandleAlloc(mEnv,&(void *)o_stmt,OCI_HTYPE_STMT,0,0);
  if (ret != OCI_SUCCESS) {
    throw OracleException (mErr,ret);
  };
  ret =  OCIStmtPrepare(o_stmt,mErr,(text *)sql.c_str(),sql.length(),OCI_NTV_SYNTAX,0);
  if (ret != OCI_SUCCESS) {
    OCIHandleFree(o_stmt,OCI_HTYPE_STMT);
    throw OracleException (mErr,ret);
  }
  //this gets column info from database
  ret = OCIStmtExecute(mCtx,o_stmt,mErr,0,0,0,0,OCI_DESCRIBE_ONLY);
  if (ret != OCI_SUCCESS) {
    OCIHandleFree(o_stmt,OCI_HTYPE_STMT);
    throw OracleException (mErr,ret);
  }
  //create result and bind DbField data members to statement handle
  return Pointer<DbResult>(new OracleResult(o_stmt,mErr,mCtx));
};

void
DbOracle::disconnect() {
  if (mvState = DB_DISCONNECTED)
  
  cerr << "disconnecting from database" << endl;
  sword ret;
  ret = OCILogoff(mCtx,mErr);
  if (ret != OCI_SUCCESS) {
    OracleException e(mErr,ret);
    throw e;
  }
};

DbOracle::~DbOracle() {
  cerr << " DbOracle destructor" << endl;
  if (mvState == DB_CONNECTED) {
    try {
      disconnect();
    } catch (...) {
      cerr << "OracleDb destructor: exception in Oracle::disconnect()" << endl;
    };
  }
}

/*OracleResult*/

/*
 - get all info about columns from database
 - allocate structures for data, indicators and row errors
*/
OracleResult::OracleResult(OCIStmt *o_stmt,ResErr pErr, ResCtx pCtx)
  :   mStmt(o_stmt), mErr(pErr),mCtx(pCtx) {
  ub4 counter = 0;
  int ret;
  OCIParam *col = NULL;
  int data_type = 0;
  char *column_name = NULL;
  ub4 column_name_len = 0;
  ub4 column_data_size = 0;

  mvDataSource = NULL;
  mvIndicators = NULL;
  mvExecuted = false;
  
  ret = OCIAttrGet(mStmt, OCI_HTYPE_STMT,&counter,0,OCI_ATTR_PARAM_COUNT,mErr);
  if (ret != OCI_SUCCESS)
    throw OracleException(mErr,ret);

  for(int i = 1;i <= counter;i++) {
    ret = OCIParamGet(mStmt,OCI_HTYPE_STMT,mErr,&(void *)col,i);
      if (ret != OCI_NO_DATA) {
          if (ret != OCI_SUCCESS)
            throw OracleException(mErr,ret);

          ret = OCIAttrGet(col,OCI_DTYPE_PARAM,&data_type,0,OCI_ATTR_DATA_TYPE,mErr);
          if (ret != OCI_SUCCESS)
            throw OracleException(mErr,ret);

          ret = OCIAttrGet(col,OCI_DTYPE_PARAM,&column_name,&column_name_len,OCI_ATTR_NAME,mErr);
          if (ret != OCI_SUCCESS)
            throw OracleException(mErr,ret);

          ret = OCIAttrGet(col,OCI_DTYPE_PARAM,&column_data_size,0,OCI_ATTR_DATA_SIZE,mErr);
          if (ret != OCI_SUCCESS)
            throw OracleException(mErr,ret);

      };

    //convert some of internal Oracle datatypes to usable external representation
    switch(data_type) {
      case SQLT_NUM:
        data_type = SQLT_VNU;
        column_data_size++;
      break;      
    };

    string cname(column_name,column_name_len);
    try {
      OracleColumn *column = new OracleColumn(data_type,cname,column_data_size);
      mvColumns.push_back(*column);
    } catch (OracleException &e) {
      cerr << "OracleException: " << e.getInfo() << endl;
      for(int i = 0;i < mvColumns.size();i++) {
        delete(&(mvColumns[i]));
      };
      throw OracleException("Failed to create OracleColumn object");
    };
  };

  //Now we got all colums defined, allocate enoug memory to hold data from that columns;
  int datasize = 0; //amount of space for data;
  for(int i = 0; i < mvColumns.size();i++) {
    datasize += mvColumns[i].getSize();
  };

  cerr << "allocating " << datasize << " for data source" << endl;
  mvDataSource = new char[datasize];  
  cerr << "allocating " << counter*sizeof(sb2) << " for indicators" << endl;
  mvIndicators = new sb2[counter];
  cerr << "allocating " << counter*sizeof(ub2) << " for error codes" << endl;
  mvRowErrors = new ub2[counter];

  int offset = 0;
  for(int i = 0; i < mvColumns.size();i++) {
    OCIDefine *tmp = NULL;
    ret = OCIDefineByPos(mStmt,&tmp,mErr,i+1,mvDataSource+offset,mvColumns[i].getSize(),mvColumns[i].getOracleType(),mvIndicators + i,0,mvRowErrors + i,OCI_DEFAULT);
    if (ret != OCI_SUCCESS) {
      for(int i = 0;i < mvColumns.size();i++) {
        delete(&(mvColumns[i]));
      };
      throw OracleException(mErr,ret);
    };
    mvColumns[i].setOffset(offset);
    offset += mvColumns[i].getSize();
  };
};

OracleResult::~OracleResult() {
  cerr << "DbResult destructor" << endl;
  for(int i = 0;i < mvColumns.size();i++) {
    delete(&(mvColumns[i]));
  };
  cerr << "release data source" << endl;
  delete []mvIndicators;
  delete []mvRowErrors;
  delete []mvDataSource;
};

/*OracleColumn*/
OracleColumn::OracleColumn(ub2 type,string name,int size) : DbColumn(name,size), mvOracleType(type) {
  mvOffset = 0;
};


/*OracleRow*/
DbField
OracleResult::operator[] (int index) {
  OracleColumn &col = mvColumns[index];
  char *data = mvDataSource + col.getOffset();
  if (mvIndicators[index] == -1)  
    //this field is NULL
    return DbField((char *)NULL,true);
  else {
    //convert internal format into C++ format
    switch(col.getOracleType()) {
      //dont touch binary and char * representations
      case SQLT_CHR:
      case SQLT_STR:
      case SQLT_VCS:
      case SQLT_VBI:
      case SQLT_BIN:
      case SQLT_AFC:
      case SQLT_AVC:
        return DbField(data);
      break;
      //Oracle Number - conver to double.
      case SQLT_VNU: {
        double r;
        sword ret = OCINumberToReal(mErr,(OCINumber *)data,sizeof(r),&r);
        if (ret != OCI_SUCCESS)
          throw OracleException(mErr,ret);        
        return DbField(r);
      } break;
      case SQLT_INT: {
        int a = *data;
      } break;
      case SQLT_DAT: {
        typedef struct _date {
          unsigned char century;
          unsigned char year;
          unsigned char month;
          unsigned char day;
          unsigned char hour;
          unsigned char minute;
          unsigned char second;
        };
        union ora_int {
          char oracle[7];
          struct _date date;
        } *ora_internal;
        struct tm t;
             uword v;
        OCIDateCheck(mErr,(OCIDate *)data,&v);
        if (!v) {
          throw OracleException(mErr,OCI_ERROR);
        }
        ora_internal = (ora_int *)data;
        int century = (ora_internal -> date.century-100);
        int year = ora_internal -> date.year - 100;
        t.tm_hour = ora_internal -> date.hour-1;
        t.tm_min = ora_internal -> date.minute-1;
        t.tm_sec = ora_internal -> date.second-1;
        t.tm_year = ((century - 19)*100) + year;
        t.tm_mon = ora_internal -> date.month;
        t.tm_mday = ora_internal -> date.day;
        time_t tmp = mktime(&t);
        return DbField(&t);
      } break;
      default:
        throw OracleException("Cannot create DbField from Oracle datatype " + toString(col.getOracleType()));       
      break;
    };                        
  };
};


bool
OracleResult::fetchRow() {
  sword ret;
  if (!mvExecuted) {
    ret = OCIStmtExecute(mCtx,mStmt,mErr,1,0,0,0,OCI_DEFAULT);
    mvExecuted = true;
  } else {
    ret = OCIStmtFetch(mStmt,mErr,1,OCI_FETCH_NEXT,OCI_DEFAULT);
  };
  switch(ret) {
    case OCI_NO_DATA:
      return false;
    break;
    case OCI_SUCCESS:
      return true;
    break;
    case OCI_SUCCESS_WITH_INFO: {
      for(int i = 0;i < columns();i++) {
        if (mvRowErrors[i] == 1406)
          throw OracleException("data truncuated at column " + toString(i) + " by " +toString(mvIndicators[i]) + " bytes");
      }
      return true;
    } break;
    default: {
      throw OracleException(mErr,ret);
    } break;
  };
};

OracleException::OracleException(ResErr pErr, sword status) {
  text errbuf[512];
  ub4 buflen;
  sb4 errcode;
  switch (status) {
    case OCI_SUCCESS_WITH_INFO:
      mvDesc = "ErrorOCI_SUCCESS_WITH_INFO";
    break;
    case OCI_NEED_DATA:
      mvDesc = "ErrorOCI_NEED_DATA";
    break;
    case OCI_NO_DATA:
      mvDesc = "ErrorOCI_NO_DATA";
    break;
    case OCI_ERROR:
      OCIErrorGet ((dvoid *) pErr, (ub4) 1, NULL, &errcode,
      errbuf, sizeof(errbuf), OCI_HTYPE_ERROR);
      mvDesc = (string)((char *)&errbuf);
    break;
    case OCI_INVALID_HANDLE:
      mvDesc = "Error OCI_INVALID_HANDLE";
    break;
    case OCI_STILL_EXECUTING:
      mvDesc = "Error OCI_STILL_EXECUTE";
    break;
    case OCI_CONTINUE:
      mvDesc = "Error OCI_CONTINUE";
    break;
    default:
      mvDesc = "OCI unknown error";
    break;
  };
};

}; //namespace oracle

