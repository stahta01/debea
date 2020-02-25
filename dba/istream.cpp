// File: istream.cpp
// Purpose: Base class for archive istreams
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include <string.h>
#include "dba/istream.h"
#include "dba/collectionfilter.h"
#include "dba/int_filter.h"
#include "dba/single.h"

namespace dba {

IStream::IStream()  
  : Stream()
{
};

IStream::bindMatch::bindMatch(void* pPtr) 
  : mPtr(pPtr),
    mTable(NULL),
    mField(NULL)
{}

IStream::bindMatch::bindMatch(const char* pTable, const char* pField) 
  : mPtr(NULL),
    mTable(pTable),
    mField(pField)
{}

bool 
IStream::bindMatch::operator() (const _bind_data& pData) {
  if (mPtr != NULL)
    if (pData.mFilter->getRef() == mPtr)
      return true;
  if (mTable != NULL)
    if (!strcmp(pData.mTable,mTable) && (!strcmp(pData.mField,mField)))
      return true;
  return false;
};

bool 
IStream::get(Storeable* pObject, const char* pRootTable) {
  if (isOpen()) {
    throw APIException("IStream must be closed when get is called");
  }
  std::auto_ptr<CollectionFilterBase> filter(new Single(*pObject));
  return get(filter.get(),pRootTable);
};

bool
IStream::get(CollectionFilterBase* pFilter, const char* pRootTable) {
  bool ret = false;
  //remove all data from root list
  pFilter->clear();
  //Load objects on root list
  Storeable& parent = pFilter->create();
  open(parent,pRootTable);
  std::vector<id> ids;
  while(getNext(&parent)) {
    ret = true;
    pFilter->put();
    ids.push_back(parent.getId());
  };
  //for every BIND_COL entry load sublists recursively
  const ColTable* table = getColTable(parent);
  getChildren(pFilter, table, ids);
  setRootTable(NULL);
  return ret;
};

bool
IStream::getChildren(CollectionFilterBase* pParentFilter, const ColTable* pTable, const std::vector<id>& pParentIds) {
  if (pParentIds.empty())
    return false;
  while(pTable != NULL) {
    ColMemberEntry* member = pTable->getMembers();
    while(member != NULL) {
      //we have to remember original reference to object pointed by pFilter
      //because parent and child can use the same filter instance
      //to traverse through collection items
      //That's why reference to collection must be updated
      //before every iteration creation 
      //(note that CollectionFilter::find creates iterator internally)
      void* parentRef = pParentFilter->getRef();
      CollectionFilterBase* childFilter = member->getFilter();
      //remove all existing objects on all collections owned by objects
      //from pFilter list for this BIND_COL entry before load.
      { 
        std::auto_ptr<CollectionFilterIterator> iterator(pParentFilter->createIterator());
        while(iterator->hasNext()) {
          const Storeable& parent = iterator->get();
          childFilter->updateRef((char*)&parent + member->getMemberOffset() + pTable->getClassOffset());
          childFilter->clear();
          iterator->moveForward();
        };
      };
      //std::auto_ptr<CollectionFilterIterator> iterator(pFilter->createIterator());
      
      //set condition to load all child objects for this filter for all 
      //collections for parent objects 
      Storeable& child = childFilter->create();
      const char* childTable = member->getTableName();
      std::string fkey_fullname(childTable);
      fkey_fullname += ".";
      fkey_fullname += member->getFKeyName();
      setIdsCondition(fkey_fullname.c_str(),member->getRelationId(),pParentIds);
      id fkey = 0;
      bind(childTable, member->getFKeyName(), new Int(fkey));
      open(child,childTable);
      while(getNext(&child)) {
        //find owner of this child and add child to owner member pointing
        //childFilter on it.
        pParentFilter->updateRef(parentRef);
        const Storeable& parent = pParentFilter->find(fkey);
        childFilter->updateRef((char*)&parent + member->getMemberOffset() + pTable->getClassOffset());
        childFilter->put();
      };
      unbind(childTable, member->getFKeyName());
    
      //call getChildren for all loaded child lists
      std::auto_ptr<CollectionFilterIterator> iterator(pParentFilter->createIterator());
      while(iterator->hasNext()) {
        const Storeable& parent = iterator->get();
        childFilter->updateRef((char*)&parent + member->getMemberOffset() + pTable->getClassOffset());
        getChildren(childFilter,getColTable(child),childFilter->getIds());
        iterator->moveForward();
      };
      member = member->getNextMember();
    };
    pTable = pTable->getNextTable();
  };
  return true;
};

void 
IStream::setIdsCondition(const char* pFKeyName, id pRelationId, const std::vector<id>& pIds) {
  throw APIException("This stream cannot set contition for loading objects with ids from custom set");
};


}
