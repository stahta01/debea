// File: ostream.cpp
// Purpose: Base class for archive output streams
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/ostream.h"
#include "dba/collectionfilter.h"
#include "dba/int_filter.h"
#include <algorithm>

namespace dba {

OStream::OStream() 
  : Stream()
{    
};

bool
OStream::put(Storeable* pObject) {
  bool ret = false;
  switch(pObject->getState()) {
    case Storeable::DELETED:
      ret = erase(pObject);
    break;
    case Storeable::CHANGED:
      ret = update(pObject);
      if (ret)
        makeOk(pObject);
    break;
    case Storeable::NEW:
      ret = store(pObject);
      if (ret)
        makeOk(pObject);
    break;
    case Storeable::OK:
      //nothing for OK
    break;
  };
  if (isCollectionFilterSupported())
    ret = putChildren(pObject);
  return ret;
};

bool
OStream::putMemberChildren(Storeable* pObject, ColMemberEntry& pMember, CollectionFilterBase& pFilter, const char* pTableName) {
  bool ret = false;
  bool was_changed = false;
  //ref data contains id - old fk_value pairs
  id rel_id = pMember.getRelationId();
  std::vector<id> ids = loadRefData(pTableName, pMember.getFKeyName(), rel_id, pObject->getId());
  //if object is deleted forget about storing children and 
  //start deleting them
  if (!pObject->isDeleted()) {
    std::auto_ptr<CollectionFilterIterator> iterator(pFilter.createIterator());
    //bind foreign key to object table
    id fk_value = pObject->getId();
    //iterate through collection and store all changed objects
    //deleted objects are erased with their all children recursively
    while(iterator->hasNext()) {
      //this is current instance of storeable object from BIND_COL list
      //we cast away const for update and update will alter
      //Storeable member fields. Because of this collection order
      //should not be determined by any of Storeable field.
      Storeable& toStore = (Storeable&)(iterator->get());
      //if object is deleted we just ignore it
      //its id is on ids list and this object will be deleted
      //with rest of objects loaded from database and not on this list
      if (!toStore.isDeleted()) {
        //if object is not on list then it is new or
        //was moved from another parent
        std::vector<id>::iterator it = std::find(ids.begin(),ids.end(),toStore.getId());
        if (it == ids.end()) {
          toStore.setChanged();
        } else {
          //prevent erasing object and its children by eraseChildren(ids,...) below
          ids.erase(it);
        };
        bind(pTableName,pMember.getFKeyName(), new Int(fk_value), Database::INTEGER);
        if (rel_id != Storeable::InvalidId)
          bind(pTableName,"dba_coll_id", new Int(rel_id), Database::INTEGER);
        open(pTableName);
        if (toStore.isNew()) {
          was_changed = store(&toStore);
        } else if (toStore.isChanged()) {
          was_changed = update(&toStore);
        };
        if (was_changed) {
          makeOk(&toStore);
          ret = was_changed;
        }
        unbind(pTableName,pMember.getFKeyName());
        if (rel_id != Storeable::InvalidId)
          unbind(pTableName,"dba_coll_id");
        setRootTable(NULL);
        was_changed = putChildren(&toStore);
        if (!ret) ret = was_changed;
      };
      iterator->moveForward();
    };
  };
  //there are unknown child objects loaded from database.
  //synthesize instance for them and delete them with
  //their children recursively.
  if (!ids.empty()) {
    Storeable& for_table = pFilter.create();
    was_changed = eraseChildren(ids,for_table,pTableName);
    if (!ret) ret = was_changed;
  };
  return ret;
};

bool
OStream::putChildren(Storeable* pObject) {
  bool ret = false;
  const ColTable* table = getColTable(*pObject);
  while(table != NULL) {
    ColMemberEntry* member = table->getMembers();
    initMemberChildrenStore(*pObject,*member);
    while(member != NULL) {
      //getTableName uses create() to call Storeable::getRootTable()
      //to get name of table, but some filters (like Single)
      //can use CollectionFilter::mMember field for create()
      //so before every call to getTableName CollectionFilter::mMember
      //must be adjusted.
      CollectionFilterBase& filter = *member->getFilter();
      filter.updateRef((char*)pObject + member->getMemberOffset() + table->getClassOffset());
      const char* obj_table = member->getTableName();
      //call archive specific store alghoritm
      bool was_changed = putMemberChildren(pObject, *member, filter, obj_table);
      if (!ret) ret = was_changed;
      //move forward in loop
      member = member->getNextMember();
    };
    endMemberChildrenStore(*pObject,*member);
    //move forward in loop
    table = table->getNextTable();
  };
  return ret;
};

std::vector<id> 
OStream::loadRefData(const char* pTable, const char* pFkName, id pCollId, id pId) {
  throw APIException("This stream is not capable loading reference data for BIND_COL");
};

bool
OStream::eraseChildren(const std::vector<id>& pRefData, Storeable& pObj, const char* pRootTableName) {
  bool ret = false;
  if (!pRefData.empty()) {
    const ColTable* table = getColTable(pObj);
    while(table != NULL) {
      ColMemberEntry* member = table->getMembers();
      while(member != NULL) {
        const char* obj_table = member->getTableName();
        //delete child references of current object first
        for(std::vector<id>::const_iterator it = pRefData.begin(); it != pRefData.end(); it++) {
          std::vector<id> ids = loadRefData(obj_table, member->getFKeyName(), 0, *it);
          //adjust filter ptr before call to member->getTableName()
          CollectionFilterBase& filter = *member->getFilter();
          filter.updateRef((char*)&pObj + member->getMemberOffset() + table->getClassOffset());
          Storeable& for_table = member->getFilter()->create();
          eraseChildren(ids,for_table,member->getTableName());
        };
        //move forward in loop
        member = member->getNextMember();
      };
      //move forward in loop
      table = table->getNextTable();
    };
    if (pRootTableName == NULL)
      pRootTableName = pObj.getRootTable();
    //delete object itself from all store tables
    bool isRoot = true;
    const StoreTable* store_table = getTable(pObj);
    while(store_table != NULL) {
      const char* table_name = store_table->getTableName();
      if (table_name == NULL) {
        table_name = pRootTableName;
      } else {
        if (isRoot && pRootTableName != NULL) {
          table_name = pRootTableName;
        };
      };
      isRoot = false;
      bool was_del = deleteRefData(pRefData,table_name);
      if (!ret) ret = was_del;
      store_table = store_table->getNextTable();
    };
  }
  return ret;
};

void 
OStream::open(const char* pRootTable) {
  setRootTable(pRootTable);
  mIsOpen = true;
};

bool
OStream::deleteRefData(const std::vector<id>& pIds, const char* pTableName) {
  throw APIException("This stream is not capable deleting reference data for BIND_COL");
};

}
