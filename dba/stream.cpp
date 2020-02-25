// File: stream.cpp
// Purpose: Base class archive streams
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include <algorithm>
#include <string.h>

#include "dba/ostream.h"
#include "dba/int_filter.h"
#include "dba/string_filter.h"
#include <iostream>

extern "C" {
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
};

namespace dba {

void 
Stream::setFilterPtr(StoreableFilterBase& pFilter, void* pMember) {
  pFilter.updateRef(pMember);
};

const StoreTable*
Stream::getTable(Storeable& pObject) {
  return pObject.st_getTable();
}

const ColTable*
Stream::getColTable(Storeable& pObject) {
  return pObject.st_getColTable();
}

Stream::Stream() 
  : mRootTable(NULL),
    mMemberList(new mt_mlist())
#ifdef _DEBUG
    ,mDebugFlag(false)
#endif
{
  mIsOpen = false;
};

void
Stream::open(Storeable& pClass, const char* pRootTable) {
  setRootTable(pRootTable);     
  createTree(pClass.st_getTable());
  mIsOpen = true;
};

void
Stream::setRootTable(const char* pRootTable) {
  free((char*)mRootTable);
  mRootTable = NULL;
  if (pRootTable != NULL) {
    mRootTable = strdup(pRootTable);
  };
};

void
Stream::alterId(Storeable* pObject, int pNewId) {
  pObject -> mId = pNewId;
};

int
Stream::getStoredTables(Storeable* pObject) {
  return pObject -> mStoredTables;
};

int
Stream::countTables(Storeable* pObject) {
  return pObject -> countTables();
};


void
Stream::setStoredTables(Storeable* pObject,int pTables) {
  pObject -> mStoredTables = pTables;
};


void
Stream::unbindAll() {
  mBindings.clear();
};

void
Stream::unbindAll(const char* pTable) {
  VarMap::iterator it = mBindings.begin();
  while (it != mBindings.end()) {
    if (!(strcmp(it->mTable,pTable))) {
      it = mBindings.erase(it);
    } else {
      it++;
    };
  };
};

void
Stream::unbind(const char* pTable, const char* pField) {
  VarMap::iterator it = std::find_if(mBindings.begin(),mBindings.end(),bindMatch(pTable, pField));
  if (it == mBindings.end())
    return;
  mBindings.erase(it);
};

void
Stream::unbindVar(void* pPtr) {
  VarMap::iterator it = std::find_if(mBindings.begin(),mBindings.end(),bindMatch(pPtr));
  if (it == mBindings.end())
    return;
  mBindings.erase(it);
};


bool
Stream::isOpen() const {
  return mIsOpen;
};

Stream::_bind_data::_bind_data(const char* pTable, const char* pField, StoreableFilterBase* pFilter, Database::StoreType pType)
  : mTable(strdup(pTable)),
    mField(strdup(pField)),
    mType(pType),
    mFilter(pFilter)
{
};

Stream::_bind_data::_bind_data(const _bind_data& pObj) 
  : mTable(strdup(pObj.mTable)),
    mField(strdup(pObj.mField)),
    mType(pObj.mType),
    mFilter(pObj.mFilter)
{
}

Stream::_bind_data&
Stream::_bind_data::operator=(const Stream::_bind_data& pData) {
  free(mTable);
  free(mField);
  mTable = strdup(pData.mTable);
  mField = strdup(pData.mField);
  mFilter = pData.mFilter;
  return *this;
};

Stream::_bind_data::~_bind_data() {
  free((void*)mTable);
  free((void*)mField);
}

BindedStid
Stream::bindStid(const char* pTable, const char* pField, StoreableFilterBase* pFilter) {
  id* var = new id();
  BindedStid bint(this,var);
  _bind_data b(pTable,pField,new Int(*var),Database::INTEGER);
  mBindings.push_back(b);
  return bint;
};


BindedInt
Stream::bindInt(const char* pTable, const char* pField, StoreableFilterBase* pFilter) {
  int* var = new int();
  BindedInt bint(this,var);
  _bind_data b(pTable,pField,new Int(*var),Database::INTEGER);
  mBindings.push_back(b);
  return bint;
};

BindedString
Stream::bindString(const char* pTable, const char* pField, StoreableFilterBase* pFilter) {
  std::string* s = new std::string();
  BindedString bstr(s);
  _bind_data b(pTable,pField,new String(*s),Database::STRING);
  mBindings.push_back(b);
  return bstr;
};

void 
Stream::bind(const char* pTable, const char* pField, StoreableFilterBase* pFilter, Database::StoreType pType) {
  mBindings.push_back(_bind_data(pTable,pField,pFilter,pType));
};

const char* 
Stream::getRootTableName(Storeable& pObj) {
  //get name of root store table
  const char* rootTableName = mRootTable;
  if (rootTableName == NULL) {
    const StoreTable* rootTable = getTable(pObj);
    if (rootTable == NULL)
      throw APIException("store table not defined, call get with overridden root table name");
    rootTableName = rootTable->getTableName();
  };
  if (rootTableName == NULL)
    throw APIException("no store table for object");
  return rootTableName;
};

void
Stream::createTree(const StoreTable* pRootTable) {
  const StoreTable* tbl = pRootTable;
  mMemberList->clear();
  const char* tableName = NULL;
  while(tbl != NULL) {
    //replace root table name with user supilled (if any)
    if (tbl == pRootTable) {
      if (mRootTable != NULL) {
        tableName = mRootTable;     
      } else {
        tableName = tbl->getTableName();
      };
      //root table cannot have NULL name
      if ((tableName == NULL)&&(tbl->getTableName() == NULL)) {
        throw APIException("Root table cannot be NULL");
      };
    } else {
      //if store table defines table name then 
      //use it as name of table, if not then use parent name
      if (tbl->getTableName() != NULL) {
        tableName = tbl->getTableName();
      };
    };
    mMemberList->addClass(tableName);
//    cerr << "offset correction for class: " << tbl -> tableName << " " << tbl -> classOffset << endl;

    StoreTableMember* member = tbl->getMembers();
    while (member != NULL) {
//      cerr << "adding " << member->getMemberName() << " for " << tbl->getTableName() << ",mo:" << member->getMemberOffset() << ",to:" << member->getMemberOffset()  + tbl->classOffset << endl;
      mMemberList->add(tableName,member->getMemberName(),member->getMemberOffset() + tbl->getClassOffset(),member->getFilter(),member->getDatabaseType());
      member = member->getNextMember();
    };
    tbl = tbl->getNextTable();
  };
};

void
Stream::makeOk(Storeable* pObject) {
  pObject->mStoreState = Storeable::OK;
};

#ifdef _DEBUG
void 
Stream::debug(const char* pFormat, ...) {
  if (!mDebugFlag)
    return;
/* Guess we need no more than 100 bytes. */
  int n, size = 100;
  char *p, *np;
  va_list ap;

  if ((p = (char*)malloc (size)) == NULL)
    return;

  while (1) {
    /* Try to print in the allocated space. */
    va_start(ap, pFormat);
#if defined(_MSC_VER) && _MSC_VER <= 1310
	n = _vsnprintf (p, size, pFormat, ap);
#else
    n = vsnprintf (p, size, pFormat, ap);
#endif
    va_end(ap);
    /* If that worked, return the string. */
    if (n > -1 && n < size)
      break;
    /* Else try again with more space. */
    if (n > -1)    /* glibc 2.1 */
      size = n+1; /* precisely what is needed */
    else           /* glibc 2.0 */
      size *= 2;  /* twice the old size */
    if ((np = (char*)realloc (p, size)) == NULL) {
      free(p);
      return;
    } else {
      p = np;
    }
  }
  std::cerr << p << std::endl;
  free(p);
};
#endif

Stream::~Stream() 
{
  unbindAll();
  //allocated by C strdup 
  free((char*)(mRootTable));
};

}
