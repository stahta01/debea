//
// C++ Implementation: xmlostream
//
// Description: 
//
//
// Author: Lukasz Michalski <lm@zork.pl>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include <string.h>

#include "dba/xmlostream.h"
#include "dba/conversion.h"
#include "dba/collectionfilter.h"
#include "dba/xmlutils.h"

namespace dba {

XMLOStream::XMLOStream(xmlDocPtr pDocument, xmlNodePtr pNode, bool pReplaceParentNode, const ConvSpec& pSpecs, bool pUseElements)
  : OStream(),
    ConvSpecContainer(pSpecs),
    mDocument(pDocument),
    mParentNode(pNode),
    mReplaceParentNode(pReplaceParentNode),
    mUseElements(pUseElements)
{
}

void 
XMLOStream::close() {
  //no implementation needed, document is managed by XMLArchive
};

void 
XMLOStream::destroy() {
  mDocument = NULL;
  mParentNode = mLastAddedNode = NULL;
};

void 
XMLOStream::assignId(Storeable* pObject) throw (Exception) {
  //no implementation needed
};

bool 
XMLOStream::erase(Storeable* pObject) {
  //not supported
  throw APIException("XML format does not support erase()");
  return true;
};

bool 
XMLOStream::update(Storeable* pObject) {
  //file is always created from scratch
  return store(pObject);
};

void
XMLOStream::setAttribute(xmlNodePtr pNode, const char* pName, const xmlChar* pData) {
  char* nsname(XMLUtils::getNsFromName(pName));
  if (nsname == NULL) {
    xmlNewProp(pNode,(xmlChar*)pName,pData);
  } else {
    xmlNsPtr ns = xmlSearchNs(mDocument,pNode,(xmlChar*)nsname);
    delete [] nsname;
    if (ns == NULL) {
      std::string err("Error setting attribute '");
      err += pName;
      err += "'. Namespace not added, call addNamespace on XMLArchive first";
      throw APIException(err.c_str());
    };
    char* propname(XMLUtils::getNameWithoutNs(pName));
    xmlNewNsProp(pNode,ns,(xmlChar*)propname,pData);
    delete [] propname;
  };
};

xmlNodePtr
XMLOStream::createNode(const char* pName) {
  char* nsname(XMLUtils::getNsFromName(pName));
  xmlNsPtr ns = NULL;
  xmlNodePtr node = NULL;
  if (nsname != NULL) {
    ns = xmlSearchNs(mDocument,mParentNode,(xmlChar*)nsname);
    delete [] nsname;
    if (ns == NULL) {
      std::string err("Error creating node '");
      err += pName;
      err += "'. Namespace not added, call addNamespace on XMLArchive first";
      throw APIException(err.c_str());
    };
    char* nodename(XMLUtils::getNameWithoutNs(pName));
    node = xmlNewNode(ns, (xmlChar*)nodename);
    delete [] nodename;
  } else {
    node = xmlNewNode(ns, (xmlChar*)pName);
  };
  return node;
};

void
XMLOStream::updateNodeFromObject(xmlNodePtr pNode, const Storeable& pObject, mt_class* pTable) {
  mt_member* member = pTable->firstField;
  while(member->name != NULL) {
    void* member_ptr = (char*)&pObject + (int)(member->offset);
    StoreableFilterBase& filter = *(StoreableFilterBase*)member->func;
    filter.updateRef(member_ptr);
    if (!filter.isNull()) {
      std::string strdata(filter.toString(getConversionSpecs()));
      updateNodeData(pNode,member->name,strdata.c_str());
    };
    member = member->next;
    if (member == NULL)
      break;
  };
};

void
XMLOStream::updateNodeFromVars(xmlNodePtr pNode, mt_class* pTable) {
  for (VarMap::const_iterator it = mBindings.begin(); it != mBindings.end(); it++) {
    if (!strcmp(it->mTable,pTable->name)) {
      dba::StoreableFilterBase& filter = *(it->mFilter);
      std::string data(filter.toString(mConvSpecs));
      if (!data.empty()) {
        updateNodeData(pNode,it->mField,data.c_str());
      };
    };
  };
};

void
XMLOStream::updateNodeData(xmlNodePtr pNode, const char* pName, const char* pContent) {
  if (mUseElements) {
    xmlNodePtr node(createNode(pName));
    xmlChar* encoded_data = xmlEncodeEntitiesReentrant(NULL,(xmlChar*)pContent);
    xmlNodeSetContent(node,encoded_data);
    xmlFree(encoded_data);
    debug("adding node '%s' to '%s'", pName, (const char*)pNode->name);
    xmlAddChild(pNode,node);
  } else {
    setAttribute(pNode,pName,(xmlChar*)pContent);
  };
};

bool 
XMLOStream::store(Storeable* pObject) {
  if (mDocument == NULL)
    throw APIException("XMLOStream destroyed, cannot store");

  createTree(Stream::getTable(*pObject));
  if (mMemberList->mFirst->name == NULL)
    return false;

  mt_class* current = mMemberList->mFirst;
  xmlNodePtr node = createNode(current->name);
  if (!mReplaceParentNode) {
    debug("Adding child node '%s' to '%s'", getRootTableName(*pObject), (const char*)mParentNode->name);
    //store table is readed from last element to first
    //and from child table to parent.
    //but we want to add elements in oposite order
    xmlNodePtr last = xmlGetLastChild(mParentNode);
    if (last == NULL)
      mLastAddedNode = xmlAddChild(mParentNode, node);
    else
      mLastAddedNode = xmlAddPrevSibling(last,node);
 } else {
    node->nsDef = xmlCopyNamespaceList(mParentNode->nsDef);
    xmlFree(xmlDocSetRootElement(mDocument,node));
    //no mParentNode means that we are the parent node
    mParentNode = NULL;
    mReplaceParentNode = false;
  };
  //FIXME add configuration for storing id?
  //xmlNewProp(node,(xmlChar*)"id",(xmlChar*)(toStr(id).c_str()));
  while(current->name != NULL) {
    updateNodeFromObject(node,*pObject,current);
    updateNodeFromVars(node,current);
    current = current->next;
    if (current == NULL)
      break;
  };
  return true;
};

void 
XMLOStream::initMemberChildrenStore(const Storeable& pObject, const ColMemberEntry& pMember) {
  //set new parent node for storing subobjects
  if (mParentNode != NULL) {
    debug("going down to last added child '%s' of mParent '%s'", (const char*)mLastAddedNode->name, (const char*)mParentNode->name);
    mParentNode = mLastAddedNode;
  } else {
    mParentNode = xmlDocGetRootElement(mDocument);
    debug("mParent set to '%s'", (const char*)mParentNode->name);
  };
};

void 
XMLOStream::endMemberChildrenStore(const Storeable& pObject, const ColMemberEntry& pMember) {
  //parent node was set to point to valid parent object
  //in hierarchy. After storing all children we have to reset it back to
  //parent of pObject node
  if (mParentNode != xmlDocGetRootElement(mDocument)) {
    debug("up from mParent '%s'", (const char*)mParentNode->name);
    mLastAddedNode = mParentNode;
    mParentNode = mParentNode->parent;
  };
  if (mParentNode == NULL)
    throw DatabaseException("Internal error: parent node is NULL");
};


std::vector<id> 
XMLOStream::loadRefData(const char* pTable, const char* pFkName, id pCollId, id pId) {
  //not used
  return std::vector<id>();
};

bool 
XMLOStream::deleteRefData(const std::vector<id>& pIds, const char* pTableName) {
  //not used
  return false;
};

bool 
XMLOStream::putMemberChildren(Storeable* pObject, ColMemberEntry& pMember, CollectionFilterBase& pFilter, const char* pTableName) {
  bool was_changed = false;
  bool ret = false;
  if (!pObject->isDeleted()) {
/*    if (mParentNode != NULL) {
      debug("going down to first child of mParent '%s'", (const char*)mParentNode->name);
      mParentNode = mParentNode->children;
    } else {
      mParentNode = xmlDocGetRootElement(mDocument);
      debug("mParent set to '%s'", (const char*)mParentNode->name);
    };*/
    std::auto_ptr<CollectionFilterIterator> iterator(pFilter.createIterator());
    //do not create fkey node if list is empty
    if (iterator->hasNext()) {
      if (pMember.getFKeyName() != NULL) {
        mParentNode = xmlAddChild(mParentNode, createNode(pMember.getFKeyName()));
      };
      while(iterator->hasNext()) {
        Storeable& toStore = (Storeable&)(iterator->get());
        if (!toStore.isDeleted()) {
          was_changed = put(&toStore);
          if (!ret) ret = was_changed;
        };
        iterator->moveForward();
      };
      if (pMember.getFKeyName() != NULL) {
        mParentNode = mParentNode->parent;
      };
    };
/*    if (mParentNode != xmlDocGetRootElement(mDocument)) {
      debug("up from mParent '%s'", (const char*)mParentNode->name);
      mParentNode = mParentNode->parent;
    };*/
  };
  return ret;
};

XMLOStream::~XMLOStream()
{
}


} //namespace
