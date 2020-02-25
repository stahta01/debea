// File: odbc.h
// Purpose: ODBC driver for DBA library
// Author: Aleksander Jaromin, Lukasz Michalski <lm at zork.pl>, Copyright 2006
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)
//
//
//http://msdn.microsoft.com/library/default.asp?url=/library/en-us/odbc/htm/odbcabout_this_manual.asp

#ifndef ODBCODBC_H
#define ODBCODBC_H

#ifdef _WIN32
  #include <Windows.h>
#endif
//ODBC includes
#include <sql.h>
#include <sqlext.h>

#include <map>
#include <vector>
#include "dba/plugininfo.h"
#include "dba/convspec.h"
#include "dba/database.h"
#include "dba/chandle.h"

namespace odbc {

class OdbcConnection;

/**
  CHandle deallocator for HSTMT
*/
class HSTMTDealloc {
  public:
    void dealloc(HSTMT pHandle) {
      SQLRETURN ret = SQLCloseCursor(pHandle);
      ret = SQLFreeHandle(SQL_HANDLE_STMT, pHandle);
      //std::cerr << "deleting HSTMT " << pHandle << ":" << ret << std::endl; 
    };
};

/**
  CHandle deallocator for HDBC
*/
class HDBCDealloc {
  public:
    void dealloc(HDBC pHandle);
};

/**
  CHandle deallocator for HENV
*/
class HENVDealloc {
  public:
    void dealloc(HENV pHandle) {
      SQLRETURN ret = SQLFreeHandle(SQL_HANDLE_ENV, pHandle);
    };
};


class OdbcDb : public dba::Database  {
  public:
    /**
      Constructor initializes HENV
    */
    OdbcDb();  
    virtual dba::DbConnection* getConnection(const char* pParams);
    virtual ~OdbcDb();
  private:
    void registerConn(OdbcConnection* pConnection);
    void unregisterConn(OdbcConnection* pConnection);
    std::set<OdbcConnection*> mConnections;
    dba::CHandle<HENV,HENVDealloc> mHenv;
};

class OdbcColumn : public dba::DbColumn {
  public:
    OdbcColumn(dba::ConvSpec::charset pDbCharset = dba::ConvSpec::UTF8);
    OdbcColumn(const char* pName, dba::ConvSpec::charset pDbCharset);
    virtual const char* getName() const;
    SQLLEN getOdbcType() const { return mType; }
    void setOdbcType(SQLLEN pType) { mType = pType; }
    void setOdbcIndex(int pIndex) { mOdbcIndex = pIndex; }
    int getOdbcIndex() const { return mOdbcIndex; }
    void setDataSize(int pSize) { mDataSize = pSize; }
    int getDataSize() const { return mDataSize; }
    void setName(const char* pName);
    virtual ~OdbcColumn();
    //!SQL type of data
    SQLLEN mType;
    //!Column name    
    std::string mName;
    //!Data size for fixed size types, 0 otherwise
    long mDataSize;
    //!index for odbc calls
    int mOdbcIndex;
};

class OdbcConnection : public dba::DbConnection {
  public:
    /**
      Constructor
      @param pHDBC already created and connected HDBC handle
    */
    OdbcConnection(OdbcDb* pDb, HDBC pHDBC, unicode_flag_t pUseUnicode);
    virtual void begin();
    virtual void commit();
    virtual void rollback();
    virtual void disconnect();
    virtual bool isValid() const;
    virtual std::list<std::string> getRelationNames();
    virtual ~OdbcConnection();
  private:
    HSTMT createHstmt();
    HSTMT doSQLExec(const char* pSql);
    void setAutoCommit(bool pFlag);
    //!ODBC handle
    dba::CHandle<HDBC,HDBCDealloc> mHdbc;
    //!Should driver use SQLExec or SQLExecW
    unicode_flag_t mUseUnicode;
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
    /**
      Perform conversion from UTF8 to wide char for odbc
    */
    wchar_t* CPToWideChar(const char* pUTF8Str);
};

class OdbcResult : public dba::DbResult {
    friend class OdbcConnection;
  public:
    virtual int columns() const;
    virtual const dba::DbColumn& getColumn(int i) const { return findColumn(i).mColDesc; }
    virtual void cancel();
    virtual bool fetchRow();
    virtual ~OdbcResult();
  private:
    class ColumnData {
      public:
        ColumnData();
        /**
          Get pointer to first byte of retrieved data
        */
        const char* getStringBuf() const;
        const wchar_t* getWStringBuf() const;
        /**
          Get long value of retrieved field
        */
        long getIntValue() const;
        /**
          Get date value from retrieved field
        */
        struct tm getDateValue() const;
        /**
          Get doouble value from retrieved field
        */
        double getDoubleValue() const;
        /**
          Check if retrieved field contains NULL value
        */
        bool isNull() const { return mNull; }
        /**
          Allocate initial buffer, data pointer and CType
        */
        void init();
        /**
          resize char or wchar_t buffer
        */
        void growData(int pChars);
        /**
          get number of chars that we can read into buffer
        */
        int getRemainingSize() const { return mBufCapacity - mDataSize; }
        /**
          get current pointer to first free char
        */
        void* getDataEndPtr() const;
        /**
          Move internal pointer by pValue chars
        */
        void updateFilledSize(int pValue);
        /**
          Get total size of buffer
        */
        int getCapacity() const { return mBufCapacity; }
        /**
          get %SQL C Type that should be passed to SQLGetData
        */
        SQLSMALLINT getCType() const { return mCType; }
        bool isUsingCharBuf() const;
        void decrementEndDataPtr();
        /**
          Description retrieved from statement handle
          valid for all rows
        */
        OdbcColumn mColDesc;
        /**NULL flag*/
        bool mNull;
        /**
          Destructor deletes allocated mCharPtr or mWCharPtr if column has text type
        */
        ~ColumnData();
      private:
        ColumnData(const ColumnData&);
        ColumnData& operator=(const ColumnData&);
        /**
        Remove all previously assigned data
        */
        void cleanup();
        /**
          Row data. Variant used depends on mColumn.getOdbcType
        */
        union {
          SQLINTEGER mIntValue;
          SQLDOUBLE mDoubleValue;
          SQLCHAR* mCharBuf;
          TIMESTAMP_STRUCT mTimestampValue;
        };
        /**
          Pointer to place where we will write next part of field data
        */
        SQLCHAR* mCharPtr;
        /**Retrieved data size for variable sized fields*/
        int mDataSize;
        /**Buffer capacity*/
        int mBufCapacity;
        /**type passed to SQLGetData*/
        SQLSMALLINT mCType;
    };

    class colNameMatcher {
      public:
        colNameMatcher(const std::string& pName) : mName(pName) {};
        bool operator() (const ColumnData& pCol) {
          return pCol.mColDesc.getName() == mName;
        };
      private:
        const std::string& mName;
    };

    //implementation
    virtual const char* doGetString(int pField) const;
    virtual long doGetInt(int pField) const;
    virtual double doGetDouble(int pField) const;
    virtual struct tm doGetDate(int pField) const;
    virtual int getColumnIndex(const char* pField) const;
    virtual bool doCheckNull(int pField) const;
    
    //!ODBC handle wrapped in exception-safe pointer
    dba::CHandle<HSTMT,HSTMTDealloc> mHstmt;
    
    //!description of columns and buffers for retrieved data
    ColumnData* mColumns;
    
    //!temporary buffer for getString from non-string data
    mutable std::string mStrBuf;
    
    //!number of elements in mColumns array
    int mNumCols;
    
    //!initialize mColumns from ODBC statement
    void initColumnsData();

    /**
      Constructor.    
    */
    OdbcResult(OdbcConnection* pConn, HSTMT pHstmt);
    /**
      Perform conversion from wide char to UTF8 for dba
    */
    void wideCharToCP(std::string& pDest, const wchar_t* pWideCharStr) const;
    const ColumnData& findColumn(int pField) const;
    void createColumn(int pStmtIndex);
};

extern "C" {
  dbaDLLEXPORT dba::Database* dbaCreateDb();
  dbaDLLEXPORT dba::dbaPluginInfo dbaGetPluginInfo();
  dbaDLLEXPORT void dbaDeleteDb(dba::Database* pDb);
};

};

#endif
