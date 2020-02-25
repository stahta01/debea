// File: istream.h
// Purpose: ODBC driver for DBA library
// Author: Aleksander Jaromin, Lukasz Michalski <lm at zork.pl>, Copyright 2006
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


//http://msdn.microsoft.com/library/default.asp?url=/library/en-us/odbc/htm/odbcabout_this_manual.asp
//http://msdn.microsoft.com/library/default.asp?url=/library/en-us/odbc/htm/odbcsqlallochandle.asp

#include <algorithm>
#include <sstream>
#include <assert.h>
extern "C" {
  #include <string.h>
}

#include "dba/conversion.h"
#include "dba/odbc.h"
#include "dba/localechanger.h"
#include "dba/connectstring.h"

#ifdef _MSC_VER
  #ifdef _DEBUG
    #include <crtdbg.h>
      #define new new(_NORMAL_BLOCK ,__FILE__, __LINE__)
  #endif
#endif

#if (ODBCVER < 0x0300)
  #error "Must be compiled with ODBC 3.0 or higher"
#endif

using namespace dba;

namespace odbc {

/**
  Get errors and warnings from ODBC handle
*/
std::string
getDiagnosticMessages(SQLSMALLINT pHandleType, SQLHANDLE pHandlePtr, const char* pWhere) {
  SQLTCHAR buf[SQL_MAX_MESSAGE_LENGTH];
  SQLTCHAR sqlstate[15];
  SQLINTEGER nativeError = 0;
  SQLSMALLINT buflen;
  
  std::stringstream stream;
  stream << ": ";

  SQLINTEGER msgs = 5;
  //this code hangs under iODBC
  //SQLRETURN dr = SQLGetDiagField(pHandleType, pHandlePtr, 0, SQL_DIAG_NUMBER, &msgs, SQL_IS_INTEGER, &buflen);
  //if (dr == SQL_SUCCESS) {
    for(int i = 0; i < msgs; i++) {
      SQLRETURN ret = SQLGetDiagRec (pHandleType, pHandlePtr, i+1, sqlstate, &nativeError, buf, sizeof(buf)/sizeof (SQLTCHAR), &buflen);
      if (ret == SQL_SUCCESS) {
        stream << i << ": " << pWhere << " = " << buf << " (" << (long)nativeError  << ") SQLSTATE: " << sqlstate << std::endl;
      } else {
        //no messges were retrieved
        if (i ==0) return std::string();
      };
    };
  //} else {
    //stream << "<unknown error>" << std::endl;
  //};
  std::string err(pWhere);
  return err + stream.str();
};

/**
  Helper function for getting error messages from HSTMT handle
*/
void
handleStatementError(HSTMT pHstmt, const char* pWhere, const DbBase* pErrorHandler) {
  std::string err(getDiagnosticMessages(SQL_HANDLE_STMT,pHstmt,pWhere));
  pErrorHandler->handleError(dba::DBA_SQL_ERROR, err.c_str());
};

/**
  Helper function for getting error messages from HDBC handle
*/
void
handleConnectionError(HDBC pHandle, const char* pWhere, DbBase* pErrorHandler) {
  std::string err(getDiagnosticMessages(SQL_HANDLE_DBC,pHandle,pWhere));
  pErrorHandler->handleError(dba::DBA_DB_ERROR, err.c_str());
};

/**
  Print errors and warnings from ODBC handle to std::cerr
*/
void
dumpDiagnostics(SQLSMALLINT pType, SQLHANDLE pHandle, const char* pWhere) {
  std::string err(getDiagnosticMessages(pType,pHandle,pWhere));
  if (err.empty())
    return;
  std::cerr << err << std::endl;
};

void 
HDBCDealloc::dealloc(HDBC pHandle) {
  SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_DBC, pHandle);
  if (!SQL_SUCCEEDED(ret)) {
    dumpDiagnostics(SQL_HANDLE_DBC,pHandle,"HDBCDealloc");
  };
  //std::cerr << "deleting HDBC " << pHandle << ":" << ret << std::endl; 
};


OdbcDb::OdbcDb() 
{
  HENV env;
  if (SQLAllocHandle (SQL_HANDLE_ENV, 0, &env) != SQL_SUCCESS)  {
    handleError(dba::DBA_DB_ERROR,"(connect) ENV ERROR: Cannot allocate ODBC env handle");
  } else {
    mHenv.reset(env);
    SQLSetEnvAttr (mHenv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER) SQL_OV_ODBC3, SQL_IS_UINTEGER);
  };
};

dba::DbConnection*
OdbcDb::getConnection(const char* pParams) {
  HDBC hdbc;
  if (SQLAllocHandle (SQL_HANDLE_DBC, mHenv, &hdbc) != SQL_SUCCESS) {
    handleError(dba::DBA_DB_ERROR,"(connect) CONNECTION ERROR - Cannot get DB connection");
    return NULL;
  }
  
  SQLCHAR outstr[1024];
  SQLSMALLINT outstrlen;
  
  std::string dsn;
  dba::DbConnection::unicode_flag_t unicode_flag = dba::DbConnection::DEBEA_UNICODE_DEFAULT;
  try {
    dba::ConnectString conn(pParams);
    //search for DSN in dbname, then in odbcparams 
    //and if not found treat connect string as odbcparams
    dsn = conn.dbname();
    if (dsn.empty()) {
      dsn = conn.getParam("odbcparams");
      if (dsn.empty())
        dsn = pParams;
    };
    std::string unicode_param = conn.getParam("unicode");
    if (unicode_param.size()) {
      try {
        int uf;
        convert(unicode_param,uf);
        if (uf == 0)
          unicode_flag = dba::DbConnection::DEBEA_UNICODE_OFF;
        else
          unicode_flag = dba::DbConnection::DEBEA_UNICODE_ON;
      } catch (ConversionException&) {
        handleError(dba::DBA_DB_ERROR,"'unicode' flag should be set to 1 or 0");
      };
    };
  } catch (const dba::ConnectStringParserException&) {
    dsn = pParams;
  };
  RETCODE status = SQLDriverConnect(hdbc, 0, (SQLCHAR *) dsn.c_str(), SQL_NTS, outstr, sizeof(outstr), &outstrlen, SQL_DRIVER_COMPLETE);
  if (SQL_SUCCEEDED(status)) {
    //std::cerr<< "Connection string:" << outstr << std::endl;
    if (status == SQL_SUCCESS_WITH_INFO) {
      dumpDiagnostics(SQL_HANDLE_DBC, hdbc, "connect");
    };
  } else {
    handleConnectionError(hdbc,"SQLDriverConnect",this);
    return NULL;
  };
  return new OdbcConnection(this,hdbc,unicode_flag);
}

OdbcDb::~OdbcDb() {
}


OdbcColumn::OdbcColumn(dba::ConvSpec::charset pDbCharset)
  : DbColumn(pDbCharset), 
    mType(SQL_UNKNOWN_TYPE) 
{};

OdbcColumn::OdbcColumn(const char* pName, dba::ConvSpec::charset pDbCharset) 
  : DbColumn(pDbCharset),
    mName(pName),
    mType(SQL_UNKNOWN_TYPE)
{
}

void 
OdbcColumn::setName(const char* pName) {
  mName.assign(pName);
};

const char* 
OdbcColumn::getName() const {
  return mName.c_str();
}

OdbcColumn::~OdbcColumn() {
}

/*================== Connection =======================*/

OdbcConnection::OdbcConnection(OdbcDb* pDb, HDBC pHandle, unicode_flag_t pUseUnicode) 
  : DbConnection(),
    mHdbc(pHandle),
    mUseUnicode(pUseUnicode)
{
  setParentErrorHandler(pDb);
  setConversionSpecs(pDb->getConversionSpecs());
}

HSTMT 
OdbcConnection::createHstmt() {
  HSTMT hstmt;
  if (SQLAllocHandle (SQL_HANDLE_STMT, mHdbc, &hstmt) != SQL_SUCCESS) {
    handleError(dba::DBA_DB_ERROR,"(connect) CONNECTION ERROR - Cannot allocate statement handle");
    return SQL_NULL_HSTMT;
  }
  //std::cerr << "new hstmt " << hstmt << std::endl; 
  return hstmt;
}

dba::DbResult*
OdbcConnection::execQuery(const char* pSql) {
  return new OdbcResult(this, doSQLExec(pSql));
}

int 
OdbcConnection::execUpdate(const char* pSql) {
  SQLINTEGER rowcnt = -1;
  dba::CHandle<HSTMT,HSTMTDealloc> hstmt(doSQLExec(pSql));
  // error reporting is done in doSQLExec
  // we just avoid calling SQLRowCount here
  // if error handler does not throw exceptions.
  if (hstmt.ptr() == SQL_NULL_HSTMT)
    return -1;
  SQLRETURN ret = SQLRowCount(hstmt,&rowcnt);
  if (!SQL_SUCCEEDED(ret)) {
    handleStatementError(hstmt.ptr(),"execUpdate (get affected rows)",this);
  };
  return rowcnt;
}


HSTMT 
OdbcConnection::doSQLExec(const char* pSql) {
  dba::CHandle<HSTMT,HSTMTDealloc> hstmt(createHstmt());
  SQLRETURN ret;
  wchar_t* wquery = NULL;
  switch(mUseUnicode) {
    case DEBEA_UNICODE_DEFAULT:
      if (mConvSpecs.mDbCharset == dba::ConvSpec::UTF8) {
        wchar_t* wquery = CPToWideChar(pSql);
        ret = SQLExecDirectW(hstmt, wquery, SQL_NTS);
        delete [] wquery;
      } else {
        //it is always better to use unicode version of 
        //SQLExcecDirect if we are able to convert 
        //multibyte data ourselfs
        DbBase* parent = getParentErrorHandler();
        setParentErrorHandler(NULL);
        wchar_t* wquery = CPToWideChar(pSql);
        setParentErrorHandler(parent);
        if (wquery != NULL) {
          ret = SQLExecDirectW(hstmt, wquery, SQL_NTS);
        } else  {
          ret = SQLExecDirect(hstmt, (SQLCHAR*)pSql, SQL_NTS);
        };
        delete [] wquery;
      };
    break;
    case DEBEA_UNICODE_ON: {
      wchar_t* wquery = CPToWideChar(pSql);
      ret = SQLExecDirectW(hstmt, wquery, SQL_NTS);
      delete [] wquery;
    } break;
    case DEBEA_UNICODE_OFF:
      ret = SQLExecDirect(hstmt, (SQLCHAR*)pSql, SQL_NTS);
    break;
  };
  if (ret != SQL_SUCCESS && ret != SQL_NO_DATA) {
    if (ret != SQL_SUCCESS_WITH_INFO) {
      handleStatementError(hstmt.ptr(), "SQLExec", this);
      return SQL_NULL_HSTMT;
    };
  }
  return hstmt.release();
}

wchar_t* 
OdbcConnection::CPToWideChar(const char* pStr) {
#ifdef _WIN32
  int len = MultiByteToWideChar(dba::ConvSpec::getMSWCodePage(mConvSpecs.mDbCharset), 0, pStr, -1, 0, 0) + 1;
  if (len == 0) {
    std::stringstream err;
    err << "Error getting size of multibyte string '" << pStr << "'";
    handleError(DBA_DB_ERROR, err.str().c_str());
    return NULL;
  };
  wchar_t* buff = new wchar_t[len];
  len = MultiByteToWideChar(dba::ConvSpec::getMSWCodePage(mConvSpecs.mDbCharset), 0, pStr, -1, buff, len);
  if (len == 0) {
    std::stringstream err;
    err << "Error converting multibyte string '" << pStr << "'";
    handleError(DBA_DB_ERROR, err.str().c_str());
    delete [] buff;
    return NULL;
  };
#else
  LocaleChanger changer(LC_CTYPE,dba::ConvSpec::getUnixLCType(mConvSpecs.mDbCharset));
  if (!changer.isOk()) {
    std::stringstream err;
    err << "Error converting string";
    err << "to wide string using ConvSpec::mDbCharset " << mConvSpecs.mDbCharset;
    err << ": suitable locale not found";
    handleError(DBA_DB_ERROR, err.str().c_str());
    return NULL;
  };
  errno = 0;
  const char* c = pStr;
  size_t len = mbsrtowcs(NULL,&c,0,NULL); 
  if (errno == EILSEQ && len == -1) {
    std::stringstream err;
    err << "Error getting size of utf8 string '" << pStr << "'";
    err << ": check failed at character ";
    err << (c - pStr);
    handleError(DBA_DB_ERROR, err.str().c_str());
  };
  len++; //make place for terminating char
  wchar_t* buff = new wchar_t[len];
  c = pStr;
  errno = 0;
  len = mbsrtowcs(buff,&c,len,NULL);
  if (errno == EILSEQ && len == -1) {
    std::stringstream err;
    err << "Error converting utf8 string '" << pStr << "'";
    err << ": to wide char string: failed at character ";
    err << (c - pStr);
    delete buff;
    handleError(DBA_DB_ERROR, err.str().c_str());
  } else {
    //std::wcerr << buff << std::endl;
  };
#endif
  return buff;
};

void
OdbcResult::wideCharToCP(std::string& pDest, const wchar_t* pWideCharStr) const {
#ifdef _WIN32
  size_t len = WideCharToMultiByte(dba::ConvSpec::getMSWCodePage(mConvSpecs.mDbCharset), 0, pWideCharStr, -1, 0, 0, 0, 0);
  if (len == 0) {
    std::string err("Error getting size of wide string");
    handleError(DBA_DB_ERROR, err.c_str());
  };
  pDest.reserve(len);
  WideCharToMultiByte(dba::ConvSpec::getMSWCodePage(mConvSpecs.mDbCharset), 0, pWideCharStr, -1, (LPSTR)pDest.data(), len, 0, 0);
  if (len == 0) {
    std::string err("Error converting wide string to utf8 string");
    handleError(DBA_DB_ERROR, err.c_str());
  };
#else
  LocaleChanger c(LC_CTYPE,dba::ConvSpec::getUnixLCType(mConvSpecs.mDbCharset));
  if (!c.isOk()) {
    std::stringstream err;
    err << "Error converting wide string";
    err << "to string using ConvSpec::mDbCharset " << mConvSpecs.mDbCharset;
    err << ": suitable locale not found";
    handleError(DBA_DB_ERROR, err.str().c_str());
    return;
  };
  
  const wchar_t* w = pWideCharStr;
  errno = 0;
  size_t len = wcsrtombs(NULL,&w,0,NULL);
  if (errno == EILSEQ && len == -1) {
    std::string err("Error getting size of wide string");
    handleError(DBA_DB_ERROR, err.c_str());
  };
  len++; //make place for terminating char
  w = pWideCharStr;
  char* ret = new char[len];
  errno = 0;
  len = wcsrtombs(ret,&w,len,NULL);
  if (errno == EILSEQ && len == -1) {
    std::string err("Error converting wide string to utf8 string");
    delete ret;
    handleError(DBA_DB_ERROR, err.c_str());
  };
  pDest.assign(ret);
#endif
};


void
OdbcResult::cancel() {
  SQLRETURN ret = SQLCancel(mHstmt.ptr());
  if (ret != SQL_SUCCESS && ret != SQL_SUCCESS_WITH_INFO)
    handleStatementError(mHstmt.ptr(), "SQLCancel",this);
}

std::list<std::string> 
OdbcConnection::getRelationNames() {
  const int size = 512;
  SQLCHAR tablename[size+1];
  SQLINTEGER cb_tablename;
  SQLHSTMT result;
  
  // data to return
  std::list<std::string> ret;
  
  //prepare handle
  SQLRETURN retcode = SQLAllocHandle(SQL_HANDLE_STMT, mHdbc, &result);
  if (!SQL_SUCCEEDED(retcode)) {
    handleConnectionError(mHdbc,"relation names",this);
    return ret;
  };
  
  retcode = SQLTables(result, NULL, 0, NULL, 0, NULL, 0, NULL, 0);
  if (!SQL_SUCCEEDED(retcode)) {
    handleStatementError(result,"relation names (fetch)",this);  
    return ret;
  };
  
  //bind table name to c array
  retcode = SQLBindCol(result,3, SQL_C_CHAR, tablename, size, &cb_tablename);
  if (!SQL_SUCCEEDED(retcode)) {
    handleConnectionError(mHdbc,"relation names (bindcol)",this);
    return ret;
  };  
  //get data
  while (SQL_SUCCESS == retcode) {
    retcode = SQLFetch(result);
    if (retcode == SQL_NO_DATA) {
      break;
    };
    if (!SQL_SUCCEEDED(retcode)) {
      handleStatementError(result,"relation names (fetch)",this);
      return ret;
    } else {
      ret.push_back(std::string((const char*)tablename));
    };
  };
  SQLFreeHandle(SQL_HANDLE_STMT,result);
  return ret; 
};

void 
OdbcConnection::begin()  {
  setAutoCommit(false);
}


void 
OdbcConnection::commit() {
  SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, mHdbc, SQL_COMMIT);
  if (!SQL_SUCCEEDED(ret))
    handleConnectionError(mHdbc, "rollback",this);  
  setAutoCommit(true);
}

void 
OdbcConnection::rollback() {
  SQLRETURN ret = SQLEndTran(SQL_HANDLE_DBC, mHdbc, SQL_ROLLBACK);
  if (!SQL_SUCCEEDED(ret))
    handleConnectionError(mHdbc, "rollback",this);
  setAutoCommit(false);
}

void
OdbcConnection::setAutoCommit(bool pFlag) {
  SQLRETURN ret;
  if (pFlag)
    ret = SQLSetConnectAttr(mHdbc,SQL_ATTR_AUTOCOMMIT,(SQLPOINTER)SQL_AUTOCOMMIT_ON,0);
  else
    ret = SQLSetConnectAttr(mHdbc,SQL_ATTR_AUTOCOMMIT,SQL_AUTOCOMMIT_OFF,0);
  if (!SQL_SUCCEEDED(ret))
    handleConnectionError(mHdbc,"autocommit",this);
  else if (ret == SQL_SUCCESS_WITH_INFO) {
    dumpDiagnostics(SQL_HANDLE_DBC,mHdbc,"autocommit");
  };
};


void 
OdbcConnection::disconnect() {
  //do nothing if already disconnected
  if (!isValid())
    return;
  
  SQLRETURN ret;
  ret = SQLDisconnect(mHdbc);
  if (SQL_SUCCEEDED(ret)) {
    if (ret == SQL_SUCCESS_WITH_INFO)
      dumpDiagnostics(SQL_HANDLE_DBC, mHdbc, "disconnect");
  } else {
    dumpDiagnostics(SQL_HANDLE_DBC, mHdbc, "disconnect");
  };
}


bool 
OdbcConnection::isValid() const {
  SQLUINTEGER result;
  SQLRETURN ret;
  ret = SQLGetConnectAttr(mHdbc,SQL_ATTR_CONNECTION_DEAD,&result,0,NULL);
  if (!SQL_SUCCEEDED(ret)) {
    dumpDiagnostics(SQL_HANDLE_DBC, mHdbc,"isValid");
    return false;
  };
  return result == SQL_CD_FALSE;
}


OdbcConnection::~OdbcConnection() {
  disconnect();
}

int 
OdbcResult::columns() const {
  return mNumCols;
}

const OdbcResult::ColumnData&
OdbcResult::findColumn(int pField) const {
  if (pField >= mNumCols) {
    std::stringstream err;
    err << "Column index " << pField << " out of range (" << mNumCols << ")";
    handleError(DBA_DB_ERROR,err.str().c_str());
  };
  return mColumns[pField];
};

const char* 
OdbcResult::doGetString(int pField) const {
  try {
    const ColumnData& data = findColumn(pField);
    switch(data.getCType()) {
      case SQL_C_CHAR:
        return data.getStringBuf();
      break;
      case SQL_C_WCHAR:
        wideCharToCP(mStrBuf,data.getWStringBuf());    
        return mStrBuf.c_str();
      break;
      case SQL_C_SLONG:
        convert(data.getIntValue(),mStrBuf);
        return mStrBuf.c_str();    
      break;
      case SQL_C_DOUBLE:
        convert(data.getDoubleValue(),mStrBuf);
        return mStrBuf.c_str();    
      break;
      case SQL_C_TYPE_TIMESTAMP:
        const tm& tmp = data.getDateValue();          
        char tmpstr[128];
        //for some locales/format configuration strftime can return emtyp string
        //thats why we check first char of converted data
        tmpstr[0] = '\1';
        size_t len = strftime(tmpstr,128,mConvSpecs.mTimestampFormat,&tmp);
        if (len == 0 && (tmpstr[0] != '\0')) {
          std::stringstream err;
          err << "Failed to convert date to string using format '" << mConvSpecs.mTimestampFormat << "'";
          handleError(DBA_DB_ERROR,err.str().c_str());
        };
        mStrBuf.assign(tmpstr);
        return mStrBuf.c_str();
      break;
    };
  } catch (const dba::ConversionException& pEx) {
    handleError(DBA_DB_ERROR,pEx.what());
  };
  return NULL;
}

long 
OdbcResult::doGetInt(int pField) const {
  const ColumnData& data = findColumn(pField);
  int ret;
  try {
    switch(data.getCType()) {
      case SQL_C_CHAR:
        convert(data.getStringBuf(),ret);
      break;
      case SQL_C_WCHAR:
        wideCharToCP(mStrBuf,data.getWStringBuf());    
        convert(mStrBuf,ret);
      break;
      case SQL_C_SLONG:
        ret = data.getIntValue();
      break;
      case SQL_C_DOUBLE:
        ret = (int)data.getDoubleValue();
      break;
      case SQL_C_TYPE_TIMESTAMP:
        const tm& tmp = data.getDateValue();          
        ret = mktime(&(tm&)tmp);
      break;
    };
  } catch (const dba::ConversionException& pEx) {
    handleError(DBA_DB_ERROR,pEx.what());
  };
  return ret;
}

double 
OdbcResult::doGetDouble(int pField) const {
  const ColumnData& data = findColumn(pField);
  double ret;
  try {
    switch(data.getCType()) {
      case SQL_C_CHAR:
        convert(data.getStringBuf(),ret,&mConvSpecs.mDecimalPoint);
      break;
      case SQL_C_WCHAR:
        wideCharToCP(mStrBuf,data.getWStringBuf());    
        convert(mStrBuf,ret,&mConvSpecs.mDecimalPoint);
      break;
      case SQL_C_SLONG:
        ret = data.getIntValue();
      break;
      case SQL_C_DOUBLE:
        ret = data.getDoubleValue();
      break;
      case SQL_C_TYPE_TIMESTAMP:
        const tm& tmp = data.getDateValue();          
        ret = mktime(&(tm&)tmp);
      break;
    };
  } catch (const dba::ConversionException& pEx) {
    handleError(DBA_DB_ERROR,pEx.what());
  };
  return ret;
}

struct tm 
OdbcResult::doGetDate(int pField) const {
  const ColumnData& data = findColumn(pField);
  struct tm ret;
  try {
    switch(data.getCType()) {
      case SQL_C_CHAR:
        ret = parseISODateTime(data.getStringBuf());
      break;
      case SQL_C_WCHAR:
        wideCharToCP(mStrBuf,data.getWStringBuf());    
        ret = parseISODateTime(mStrBuf.c_str());
      break;
      case SQL_C_SLONG: {
        time_t val = data.getIntValue();
        if (dba_localtime(&val,&ret) != &ret) {
          std::stringstream err;
          err << "Failed to convert int value'" << val << "' to date";
          handleError(DBA_DB_ERROR,err.str().c_str());
        };
      } break;
      case SQL_C_DOUBLE: {
        time_t val = (time_t)data.getDoubleValue();
        if (dba_localtime(&val,&ret) != &ret) {
          std::stringstream err;
          err << "Failed to convert double value'" << val << "' to date";
          handleError(DBA_DB_ERROR,err.str().c_str());
        };
      } break;
      case SQL_C_TYPE_TIMESTAMP:
        ret = data.getDateValue();          
      break;
    };
  } catch (const dba::ConversionException& pEx) {
    handleError(DBA_DB_ERROR,pEx.what());
  };
  return ret;
}

bool 
OdbcResult::doCheckNull(int pField) const {
  const ColumnData& data = findColumn(pField);
  return data.isNull();
}

bool 
OdbcResult::fetchRow() {
  RETCODE ret = SQLFetch(mHstmt);
  if (ret == SQL_NO_DATA)
    return false;
  if (!SQL_SUCCEEDED(ret)) {
    handleStatementError(mHstmt, "fetchRow",this);
    return false;
  }
  //use SQLGetData to retrieve all data for all columns is result 
  for (int i = 0; i != mNumCols; ++i) {
    ColumnData& col = mColumns[i];
    //bind field and null indicator
    SQLLEN len;
    while (ret = SQLGetData(mHstmt,col.mColDesc.getOdbcIndex(),col.getCType(),col.getDataEndPtr(),col.getRemainingSize(),&len) != SQL_NO_DATA) {
    //std::cerr << "readed " << col.getRemainingSize() << " bytes of data at " << ((char*)col.getDataEndPtr() - col.getStringBuf()) << std::endl;
      SQLLEN max_to_read = col.getRemainingSize();
      if (ret == SQL_SUCCESS) {
        //all data was readed or we got NULL value
        if (len != SQL_NULL_DATA)
          col.updateFilledSize(len);
        break;
      } else if (ret == SQL_SUCCESS_WITH_INFO) {
        dumpDiagnostics(SQL_HANDLE_STMT,mHstmt,"SQLGetData");
        if (col.isUsingCharBuf()) {
          //we did not fit into preallocated buffer?
          if (len == SQL_NO_TOTAL) {
            //and we don't know how many data needs to be retrieved        
            col.updateFilledSize(col.getRemainingSize());
            col.growData(col.getCapacity());
          } else {
            //resize internal buffer to match rest of data
            if (len > max_to_read) {
              col.updateFilledSize(col.getRemainingSize());
              //grow buffer to fit remaining data and terminating character
              col.growData(len - max_to_read + sizeof(wchar_t));
              std::cerr << "increased buffer size by " << len - max_to_read << ", capacity: " << col.getCapacity() << std::endl;
            } else {
              break;
            };
          };
          //remove current terminating character from buffer
          col.decrementEndDataPtr();
        } else {
          break;
        };
      } else {
        //error
        handleStatementError(mHstmt, "SQLGetData",this);
        return false;
      };
    };  
    if (len == SQL_NULL_DATA)
      col.mNull = true;
    else
      col.mNull = false;
  };
  return true;
}


OdbcResult::~OdbcResult() {
  delete[] mColumns;
}

void 
OdbcResult::initColumnsData() {
  int odbcIndex = 0;
  for (int i = 0; i != mNumCols; ++i) {
    ColumnData& col = mColumns[i];
    odbcIndex++; //ODBC first column is 1, not 0
    col.mColDesc.setOdbcIndex(odbcIndex);
    col.mColDesc.setDbCharset(mConvSpecs.mDbCharset);
    //try to read column name.
    col.mColDesc.mName.resize(128);
    SQLSMALLINT len;
    SQLRETURN ret = SQLColAttribute(mHstmt, odbcIndex, SQL_DESC_LABEL, (SQLPOINTER)col.mColDesc.mName.data(), col.mColDesc.mName.capacity(), &len, NULL);
    if (!SQL_SUCCEEDED(ret)) {
      handleStatementError(mHstmt, "SQLColAttributes (SQL_DESC_LABEL)",this);
      return;
    };
    //if size of column name was greater that 128, then resize 
    //buffer and try again
    if (len > col.mColDesc.mName.capacity()) {
      //make place for NUL terminating character
      col.mColDesc.mName.resize(len+1);
      ret = SQLColAttribute(mHstmt, odbcIndex, SQL_DESC_LABEL, (SQLPOINTER)col.mColDesc.mName.data(), col.mColDesc.mName.capacity(), &len, NULL);
      if (!SQL_SUCCEEDED(ret)) {
        handleStatementError(mHstmt, "SQLColAttributes (SQL_DESC_LABEL)",this);
        return;
      };
    };

    //remove reserved but unneeded memory
    col.mColDesc.mName.resize(len);

    //try to discover column type
    if (SQLColAttribute(mHstmt, odbcIndex, SQL_DESC_CONCISE_TYPE, NULL, 0, NULL, &col.mColDesc.mType) != SQL_SUCCESS) {
      handleStatementError(mHstmt, "SQLColAttributes (SQL_DESC_CONCISE_TYPE)",this);
      return;
    }
 
    //get size of data in described column
    //if column contains data with fixed length
    if (SQLColAttribute(mHstmt, odbcIndex, SQL_DESC_LENGTH, NULL, 0, NULL, &col.mColDesc.mDataSize) != SQL_SUCCESS) {
      col.mColDesc.setDataSize(0);
    };

    //initialize internal buffers and data pointer;
    col.init();
  };
};

OdbcResult::ColumnData::ColumnData()
: mNull(true),
  mCharBuf(0),
  mCharPtr(0),
  mDataSize(0),
  mBufCapacity(0),
  mCType(SQL_UNKNOWN_TYPE)
{
}

void 
OdbcResult::ColumnData::init() {
  //free old data
  cleanup();
  //prepare variant for data retrieval
  //assuming that for variable sized columns
  //we preallocate place for 256 bytes/chars/widechars
  //if we have data size from column description then
  //set buffer size to it plus place for terminating character
  int bufsize = mColDesc.mDataSize == 0 ? 256 : (mColDesc.mDataSize + 1);
  //for testing multiple calls to SQLGetData
  //int bufsize = 100; 
  switch (mColDesc.mType) {
    //if you add or remove types from here 
    //remember to do it in isUsingWCharBuf too
    case SQL_WCHAR:
    case SQL_WVARCHAR:
    case SQL_WLONGVARCHAR:
      mCType = SQL_C_WCHAR;
      mBufCapacity = bufsize*sizeof(wchar_t);
      mCharBuf = new SQLCHAR[mBufCapacity];
      mCharPtr = mCharBuf;
      mDataSize = 0;
    break;
    //we will return binary data as const char*
    //if you add or remove types from here 
    //remember to do it in isUsingCharBuf too
    case SQL_CHAR:
    case SQL_VARCHAR:
    case SQL_LONGVARCHAR:
    case SQL_BINARY:
    case SQL_VARBINARY:
    case SQL_LONGVARBINARY:
    case SQL_GUID:
      mCType = SQL_C_CHAR;
      mBufCapacity = bufsize;
      mCharBuf = new SQLCHAR[mBufCapacity];
      mCharPtr = mCharBuf;
      mDataSize = 0;
    break;
    //other data types fits in variant type
    case SQL_DECIMAL:
    case SQL_NUMERIC:
    case SQL_REAL:
    case SQL_FLOAT:
    case SQL_DOUBLE:
      mCType = SQL_C_DOUBLE;
      mDataSize = sizeof(mDoubleValue);
      mBufCapacity = mDataSize;
    break;
    case SQL_SMALLINT:
    case SQL_INTEGER:
    case SQL_BIT:
    case SQL_TINYINT:
    case SQL_BIGINT:
    //we conver time intervals to long because dba does not have better
    //data type for it
    case SQL_INTERVAL_MONTH:
    case SQL_INTERVAL_YEAR:
    case SQL_INTERVAL_YEAR_TO_MONTH:
    case SQL_INTERVAL_DAY:
    case SQL_INTERVAL_HOUR:
    case SQL_INTERVAL_MINUTE:
    case SQL_INTERVAL_SECOND:
    case SQL_INTERVAL_DAY_TO_HOUR:
    case SQL_INTERVAL_DAY_TO_MINUTE:
    case SQL_INTERVAL_DAY_TO_SECOND:
    case SQL_INTERVAL_HOUR_TO_MINUTE:
    case SQL_INTERVAL_HOUR_TO_SECOND:
    case SQL_INTERVAL_MINUTE_TO_SECOND:
      mCType = SQL_C_SLONG;
      mDataSize = sizeof(mIntValue);
      mBufCapacity = mDataSize;
    break;
    case SQL_TYPE_DATE:
    case SQL_TYPE_TIME:
    case SQL_TYPE_TIMESTAMP:
      mCType = SQL_C_TYPE_TIMESTAMP;
      mDataSize = sizeof(mTimestampValue);
      mBufCapacity = mDataSize;
    break;
  };
};

void* 
OdbcResult::ColumnData::getDataEndPtr() const {
  if (isUsingCharBuf()) {
    return mCharPtr;
  } else {
    return (void*)&mIntValue;
  };
};

bool
OdbcResult::ColumnData::isUsingCharBuf() const {
  return (getCType() == SQL_C_CHAR) || (getCType() == SQL_C_WCHAR);
};

void
OdbcResult::ColumnData::updateFilledSize(int pValue) {
  mDataSize += pValue;
  mCharPtr += pValue;
};

void
OdbcResult::ColumnData::growData(int pBytes) {
  SQLCHAR* newbuf = new SQLCHAR[mBufCapacity + pBytes];
  memcpy(newbuf,mCharBuf,sizeof(SQLCHAR)*mBufCapacity);
  delete [] mCharBuf;        
  mCharBuf = newbuf;
  mCharPtr = mCharBuf + mDataSize;
  mBufCapacity += pBytes;
};

void 
OdbcResult::ColumnData::cleanup() {
  if (SQL_C_WCHAR == mCType || SQL_C_CHAR == mCType) {
    delete [] mCharBuf;
    mCharBuf = 0;
    mCType = SQL_UNKNOWN_TYPE;
    mCharPtr = 0;
  }
}

void
OdbcResult::ColumnData::decrementEndDataPtr() {
  if (mCType == SQL_C_CHAR) {
    mCharPtr -= sizeof(SQLCHAR);
    mBufCapacity += sizeof(SQLCHAR);
  } else {
    mCharPtr -= sizeof(wchar_t);
    mBufCapacity += sizeof(wchar_t);
  };
};

const char* 
OdbcResult::ColumnData::getStringBuf() const {
  return reinterpret_cast<const char*>(mCharBuf);
};

const wchar_t* 
OdbcResult::ColumnData::getWStringBuf() const {
  return reinterpret_cast<wchar_t*>(mCharBuf);
};

long 
OdbcResult::ColumnData::getIntValue() const {
  return mIntValue;
};

struct tm 
OdbcResult::ColumnData::getDateValue() const {
  //we use system call to prepare returned struct
  //becase POSIX will fill time zone info here
  time_t now = time(NULL);
  struct tm val = *localtime(&now);
  //retrieved time can have different dst setting
  //that returned by mktime.
  val.tm_isdst = -1;
  val.tm_year = mTimestampValue.year - 1900;
  val.tm_mon = mTimestampValue.month - 1;
  val.tm_mday = mTimestampValue.day;
  val.tm_hour = mTimestampValue.hour;
  val.tm_min = mTimestampValue.minute;
  val.tm_sec = mTimestampValue.second;
  //normalize rest of fields
  mktime(&val);
  return val;
};

double 
OdbcResult::ColumnData::getDoubleValue() const {
  return mDoubleValue;
};

OdbcResult::ColumnData::~ColumnData() {
  cleanup();
};

OdbcResult::OdbcResult(OdbcConnection* pConn, HSTMT pHstmt) 
  : mHstmt(pHstmt),
    mColumns(0)
{
  setParentErrorHandler(pConn);
  setConversionSpecs(pConn->getConversionSpecs());
  SQLSMALLINT num_cols;
  if (SQLNumResultCols(mHstmt, &num_cols) != SQL_SUCCESS) {
    handleStatementError(mHstmt, "SQLNumResultCols",this);
    return;
  }
  mNumCols = num_cols;
  mColumns = new ColumnData[mNumCols];
  for (UWORD i = 0; i < num_cols; i++) {
    initColumnsData();
  }
}


int 
OdbcResult::getColumnIndex(const char* pName) const {
  for (int i = 0; i != mNumCols; ++i) {
    if (mColumns[i].mColDesc.getName() == std::string(pName))
      return i;
  };
  std::stringstream err;
  err << "Column " << pName << " not found in result";
  handleError(DBA_DB_ERROR,err.str().c_str());
  return -1;
}


Database* 
dbaCreateDb() {
  return new OdbcDb();
};

dbaPluginInfo 
dbaGetPluginInfo() {
  dbaPluginInfo info;
  strcpy(info.mName,"ODBC");
  strcpy(info.mLicense,DBA_LICENSE);
  strcpy(info.mDescription,"ODBC driver for DBA library");
  strcpy(info.mVendor,DBA_VENDOR);
  strcpy(info.mUrl,DBA_URL);
  info.mVersionMajor = DBA_VERSION_MAJOR;
  info.mVersionMinor = DBA_VERSION_MINOR;
  info.mVersionRelease = DBA_VERSION_RELEASE;
  info.mPluginType = DBA_DATABASE_DRIVER;
  return info;
};


void 
dbaDeleteDb(dba::Database* pDb) {
  delete pDb;
}

} //namespace
