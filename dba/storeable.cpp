// File: storeable.cpp
// Purpose: Object that can be serialized by archive
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2002
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#include "storeable.h"
#include <assert.h>
#include <locale.h>
#include "dba/idlocker.h"
#include "dba/collectionfilter.h"

#ifdef _MSC_VER
#pragma warning (disable:4100)
#endif

namespace dba {

using namespace std;

extern IdLocker::mode dba_idlocker_lock_var;
const Storeable* const Storeable::dba_pointer_place = (Storeable*)17;
StoreTableList Storeable::sStoreTableList;

StoreTableList::StoreTableList() 
{
};

StoreTableList::~StoreTableList() {
  for(std::set<StoreTable*>::iterator it = mStoreTables.begin(); it != mStoreTables.end(); it++) {
    delete (*it);
  };
  for(std::set<ColTable*>::iterator it = mColTables.begin(); it != mColTables.end(); it++) {
    delete (*it);
  };
};


//==================== MemberEntryBase ==================

MemberEntryBase::MemberEntryBase(const char* pMemberName, int pMemberOffset)
  : mMemberName(pMemberName),
    mMemberOffset(pMemberOffset)
{};

void 
MemberEntryBase::setMemberOffset(int pNewOffset) {
  mMemberOffset = pNewOffset;
};

int 
MemberEntryBase::getMemberOffset() const {
  return mMemberOffset;
};

const char* 
MemberEntryBase::getMemberName() const {
  return mMemberName;
};
//==================== StoreTableMember ==================
StoreTableMember::StoreTableMember(StoreTable* pOwner, const char* pMemberName, int pMemberOffset, StoreableFilterBase* pFilter, int pDatabaseType) 
  : MemberEntryBase(pMemberName,pMemberOffset),
    mFilter(pFilter),
    mFilterOwner(true),
    mDatabaseType(pDatabaseType),
    mNextMember(NULL)
{
  pOwner->addMember(this);
}

void 
StoreTableMember::setFilterOwner(bool pFlag) {
  mFilterOwner = pFlag;
};

StoreableFilterBase* 
StoreTableMember::getFilter() {
  return mFilter;
};

StoreTableMember* 
StoreTableMember::getNextMember() const {
  return mNextMember;
};
    
void 
StoreTableMember::setNextMember(StoreTableMember* pMember) {
  mNextMember = pMember;
};


int 
StoreTableMember::getDatabaseType() {
  return mDatabaseType;
}

StoreTableMember::~StoreTableMember() {
  if (mFilterOwner)
    delete mFilter;
};


//==================== ColMemberEntry ==================

ColMemberEntry::ColMemberEntry(ColTable* pOwner, const char* pMemberName, int pMemberOffset, CollectionFilterBase* pFilter, const char* pFKeyName)
  : MemberEntryBase(pMemberName, pMemberOffset),
    mTableName(NULL),
    mFKeyName(pFKeyName),
    mRelationId(Storeable::InvalidId),
    mFilter(pFilter),
    mNextMember(NULL)
{
  pOwner->addMember(this);
};

ColMemberEntry::ColMemberEntry(ColTable* pOwner, const char* pMemberName, int pMemberOffset, CollectionFilterBase* pFilter, const char* pFKeyName, const char* pTableName, id pRelationId)
  : MemberEntryBase(pMemberName, pMemberOffset),
    mTableName(pTableName),
    mFKeyName(pFKeyName),
    mRelationId(pRelationId),
    mFilter(pFilter),
    mNextMember(NULL)
{
  pOwner->addMember(this);
};

void 
ColMemberEntry::setNextMember(ColMemberEntry* pNextMember) {
  mNextMember = pNextMember;
};

const char* 
ColMemberEntry::getTableName() const {
  if (mTableName != NULL)
    return mTableName;
  return mFilter->create().getRootTable();
};

const char* 
ColMemberEntry::getFKeyName() const {
  return mFKeyName;
};

id 
ColMemberEntry::getRelationId() const {
  return mRelationId;
};

ColMemberEntry* 
ColMemberEntry::getNextMember() const {
  return mNextMember;
};

CollectionFilterBase* 
ColMemberEntry::getFilter() {
  return mFilter;
};

ColMemberEntry::~ColMemberEntry() {
  delete mFilter;
};
//==================== Storeable ==================
/**
  @bug why mState is set to OK when pId != Invalid ?
*/
Storeable::Storeable(id pId) : mId(pId) {
  if (mId == InvalidId)
    mStoreState = NEW;
  else
    mStoreState = OK;
  mStoredTables = 0;
  mIdLocked = UNLOCKED;
};

Storeable::Storeable(const Storeable& pObj) {
  switch (dba_idlocker_lock_var) {
    case UNLOCKED:
      mStoredTables = pObj.mStoredTables;
      mId = pObj.mId;
      mStoreState = pObj.mStoreState;
      mIdLocked = pObj.mIdLocked;
    break;
    case LOCKED:
    default:
      mId = InvalidId;
      mStoreState = NEW;
      mStoredTables = 0;
      mIdLocked = UNLOCKED;
    break;
  };
};

Storeable::stState
Storeable::getState() const {
  return mStoreState;
};

int
Storeable::countTables() {
  int ret = 0; //do not count Storeable dummy table
  const StoreTable* tbl = st_getTable();
  while(tbl != NULL) {
    ret++;
    tbl = tbl->getNextTable();
  };
  return ret;
};


void
Storeable::setState(stState pState) {
  switch(mStoreState) {
    case DELETED:
    case CHANGED:
    case OK:
      if (pState == NEW)
        mStoreState = CHANGED;
      else
        mStoreState = pState;
    break;
    case NEW:
      if (!isNew()) {
        mStoredTables = 0;
        mId = InvalidId;
        mStoreState = NEW;
      };
    break;  
  };  
};

Storeable&
Storeable::operator=(const Storeable& pObj) {
  if (this == &pObj)
    return *this;
  lock_state m;
  //FIXME remove when Storeable::IdLocker will be removed
  if (dba_idlocker_lock_var != IdLocker::UNLOCKED)
    m = (lock_state)dba_idlocker_lock_var;
  else
    m = mIdLocked;
  switch (m) {
    case UNLOCKED:
      normalAssigment(pObj);
    break;
    case LOCKED:
      preserveIdAssigment(pObj);
    break;
    default:
      //for LOCKED_FORCE_OK we don't change state
    break;
  };
  return *this;
};

void
Storeable::normalAssigment(const Storeable& pObj) {
  if (mId != pObj.mId) {
    mStoredTables = pObj.mStoredTables;
  } else {
    if (mId == InvalidId) {
      //mId and pObj.mId invalid
      //Object is still new
      mStoreState = NEW;
      mStoredTables = 0;        
    } else {
      //mId and pObj.mId valid
      //copy only if we are assgning from object that has more tables stored.
      //Without this condition number of stored tables will be wrong
      //and will cause INSERTS instead of updates.
      if (mStoredTables < pObj.mStoredTables)
        mStoredTables = pObj.mStoredTables;   
    };
  };
  mId = pObj.mId;
  mStoreState = pObj.mStoreState;
};

void
Storeable::preserveIdAssigment(const Storeable& pObj) {
  setChanged();
};

const StoreTable*
Storeable::st_getTable() {
  return NULL;
};

const ColTable*
Storeable::st_getColTable() {
  return NULL;
};


const char* 
Storeable::getRootTable() {
  const StoreTable* table = st_getTable();
  if (table == NULL)
    return NULL;
  return table->getTableName();
};

void
Storeable::setChanged() {
  setState(CHANGED);
};

void
Storeable::setDeleted() {
  setState(DELETED);
};

void
Storeable::setOk() {
  setState(OK);
};

void
Storeable::setNew() {
  mStoreState = NEW;
  mId = InvalidId;
  mStoredTables = 0;
}


void
Storeable::setId(id pId) {
  if (mId == pId)
    return;
  assert(mId == InvalidId);
  mId = pId;
  if (mId == InvalidId)
    mStoreState = NEW;
  else {
    mStoreState = CHANGED;
    //polymorphic call - it won't change StoreableState!
    setState(CHANGED);
    mStoredTables = countTables();
  };
};


id
Storeable::getId() const {
  return mId;
};

bool
Storeable::isDeleted() const {
  return(mStoreState == DELETED);
};

bool
Storeable::isChanged() const {
  return (mStoreState == CHANGED);
};

bool
Storeable::isNew() const {
  return (mStoreState == NEW);
};

bool
Storeable::isOk() const {
  return (mStoreState == OK);
};

void 
Storeable::lockStid(bool pPreserveState) {
  if (pPreserveState)
    mIdLocked = LOCKED_PRESERVE_STATE;
  else
    mIdLocked = LOCKED;
};

void 
Storeable::unlockStid() {
  mIdLocked = UNLOCKED;
};

Storeable::~Storeable() {
}

};//namespace

