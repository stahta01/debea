//
// C++ Implementation: xmlistream
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
#include <sstream>

#include "dba/xmlistream.h"
#include "dba/xmlutils.h"
#include "dba/collectionfilter.h"

namespace dba {

XMLIStream::XMLIStream(xmlDocPtr pDocument, xmlNodePtr pNode, const ConvSpec& pSpecs, bool pUseElements)
  : IStream(),
    ConvSpecContainer(pSpecs),
    mDocument(pDocument),
    mParentNode(pNode),
    mIgnoreNonMappedNodes(false),
    mIgnoreOrder(false),
    mUseElements(pUseElements)
{
  mCurrentNode = findNonTextNode(mParentNode);
}

xmlNodePtr
XMLIStream::setNextNode(xmlNodePtr pNode) {
  if (pNode) 
    pNode = pNode->next;
  return findNonTextNode(pNode);
};

xmlNodePtr
XMLIStream::findNonTextNode(xmlNodePtr pNode) {
  while(pNode != NULL && (xmlIsBlankNode(pNode) == 1))
    pNode = pNode->next;
  return pNode;
};

void 
XMLIStream::ignoreNonMappedNodes(bool pFlag) {
  throw APIException("ignoreNonMappedNodes is not implemented yet");
};

void 
XMLIStream::ignoreNodeOrder(bool pFlag) {
  throw APIException("ignoreNodeOrder is not implemented yet");
};

void 
XMLIStream::close() {
  //nothing to do here
};

void 
XMLIStream::destroy() {
  mParentNode = mCurrentNode = NULL;
};

void
XMLIStream::updateCollection(Storeable* pParent, ColMemberEntry* pEntry, xmlNodePtr pNode) {
  xmlNodePtr node = findNonTextNode(pNode);
  if (node == NULL)
    return;

  CollectionFilterBase* childFilter = pEntry->getFilter();
  Storeable& child(childFilter->create());
  open(child,pEntry->getTableName());

  while(node != NULL) {
    //debug("checking node %s", (const char*)node->name);
    if (!strcmp(getRootTableName(child),(const char*)node->name)) {
      debug("updating data for member %s, starting at node %s", getRootTableName(child),(const char*)node->name);
      childFilter->updateRef((char*)pParent + pEntry->getMemberOffset());
      Storeable* ref = childFilter->put();
      applyFilters(ref,node);
      getChildren(ref,node->children);
      if (node->children) debug("up at node %s", (const char*)node->name);
    };
    node = setNextNode(node);
  };
};

void
XMLIStream::getChildren(Storeable* pObject, xmlNodePtr pNode) {
  pNode = findNonTextNode(pNode);
  const ColTable* table = getColTable(*pObject);
  if (pNode != NULL) debug("going down, node %s, updating object '%s' collections", pNode->name, getRootTableName(*pObject));
  while(pNode != NULL) {
    //traverse through call collection members that
    //have fkey name. 
    while(table != NULL) {
      ColMemberEntry* entry = table->getMembers();
      while(entry != NULL) {
        if (entry->getFKeyName() != NULL) {
          //This means that that member entry
          //describe list of elements that is a child of
          //our node
          xmlNodePtr node = findNode(pNode,entry->getFKeyName());
          if (node == NULL) {
            std::ostringstream err;
            err << "child node of '" << pNode->name << "' for member named '" << entry->getFKeyName() << "' not found";
            throw DataException(err.str());
          } else {
            debug("got fkeyname, going down to node %s children", node->name);
            updateCollection(pObject,entry,node->children);
            debug("up (from fknode) at node %s", node->name);
          };
        } else {
          updateCollection(pObject,entry,pNode);
        };
        entry = entry->getNextMember();
      };
      table = table->getNextTable();
    };
    pNode = setNextNode(pNode);
  };
};

bool
XMLIStream::get(CollectionFilterBase* pFilter, const char* pRootTable) {
  bool ret = false;

  //create our root object using filter.
  Storeable& rootobj(pFilter->create());

  open(rootobj,pRootTable);

  //get name of root store table
  const char* rootTableName = getRootTableName(rootobj);

  debug("store table set to %s", rootTableName);
  mCurrentNode = findNode(mCurrentNode, rootTableName);
  while(mCurrentNode != NULL) {
  
    //apply non-collection filters to fetch data from
    //attributes and subelements of node
    applyFilters(&rootobj,mCurrentNode);
  
    //traverse on xml child nodes and try to find members from 
    //collection tables
    Storeable* ref = pFilter->put();
    if (mCurrentNode->children != NULL) {
      getChildren(ref,mCurrentNode->children);
      if (mCurrentNode->children != NULL) debug("up at node %s", (const char*)mCurrentNode->name);
    };
    ret = true;

    //find node that corrsponds to root object store table
    moveCurrentNode(rootTableName);
  };
  return ret;
};

xmlNsPtr
XMLIStream::getXMLNamespace(const char* pName, xmlNodePtr pNode) {
  char* ns = XMLUtils::getNsFromName(pName);
  xmlNsPtr nsptr = NULL;
  if (ns != NULL) {
    nsptr = xmlSearchNs(mDocument,pNode,(xmlChar*)ns);
    if (nsptr == NULL) {
      std::string err("Error loading data from node '");
      err += pName;
      err += "'. Namespace not added, call addNamespace on XMLArchive first";
      delete [] ns;
      throw APIException(err.c_str());
    };
    delete [] ns;
  };
  return nsptr;
};

xmlAttrPtr
XMLIStream::findAttribute(xmlNodePtr pNode, const char* pMemberName) {
  xmlAttrPtr ret = NULL;
  xmlNsPtr nsptr = getXMLNamespace(pMemberName, pNode);
  if (nsptr == NULL) {
    ret = xmlHasProp(pNode,(xmlChar*)pMemberName);
  } else {
    char* name = XMLUtils::getNameWithoutNs(pMemberName);
    ret = xmlHasNsProp(pNode,(xmlChar*)name,nsptr->href);
    delete [] name;
  };
  return ret;
};

xmlNodePtr 
XMLIStream::findNode(xmlNodePtr pNode, const char* pName) {
  xmlNsPtr nsptr = getXMLNamespace(pName, pNode);
  xmlChar* name = NULL;
  if (nsptr == NULL) {
    name = (xmlChar*)pName;
  } else {
    name = (xmlChar*)XMLUtils::getNameWithoutNs(pName);
  };

  xmlNodePtr node = NULL;
  for(node = pNode; node != NULL; node = node->next) {
    if (!xmlNodeIsText(node) && !xmlStrcmp(name,node->name))
      break;
    //if (!xmlNodeIsText(node)) debug("findNode: ignoring node %s", node->name != NULL ? (const char*)node->name : "(null)");
  };

  if (nsptr != NULL)
    delete [] name;

  return node;
};

void 
XMLIStream::moveCurrentNode(const char* pName) {
  mCurrentNode = mCurrentNode->next;
  mCurrentNode = findNode(mCurrentNode, pName);
  if (mCurrentNode == NULL)
    return;
  debug("mCurrentNode moved to %s", mCurrentNode->name != NULL ? (const char*)mCurrentNode->name : "(null)");
};

void
XMLIStream::applyFilters(Storeable* pObject, xmlNodePtr pNode) {
  const StoreTable* tbl = Stream::getTable(*pObject);
  const char* current_table_name = tbl->getTableName();
  if (current_table_name == NULL)
    throw APIException("Root table has no name");
  debug("applying filters to tables with root name = '%s' from node '%s'",current_table_name,(const char*)pNode->name);
  while(tbl != NULL) {
    if (tbl->getTableName() != NULL)
      current_table_name = tbl->getTableName();
    StoreTableMember* member = tbl->getMembers();
    while (member != NULL) {
      dba::StoreableFilterBase* filter(member->getFilter());
      filter->updateRef((char*)pObject + (int)(member->getMemberOffset() + tbl->getClassOffset()));
      xmlChar* xmldata = NULL;
      if (mUseElements) {
        xmlNodePtr node = findNode(pNode->children, member->getMemberName());
        if (node != NULL) {
          debug("member %s: using child node",(const char*)node->name);
          xmldata = xmlNodeGetContent(node);
        };
      };
      if (xmldata == NULL) {
        xmlAttrPtr attr = findAttribute(pNode,member->getMemberName());
        if (attr != NULL) {
          debug("member %s: using attribute",(const char*)attr->name);
          xmldata = xmlNodeGetContent(attr->children);
        };
      };
      if (xmldata != NULL) {
        std::string data((const char*)xmldata);
        filter->fromString(mConvSpecs, data);
      } else {
        filter->fromNull();
      };
      member = member->getNextMember();
    };
    tbl = tbl->getNextTable();
  };
  updateVars();
};

bool 
XMLIStream::getNext(Storeable* pObject) {
  if (mCurrentNode == NULL)
    return false;
  applyFilters(pObject,mCurrentNode);
  mCurrentNode = setNextNode(mCurrentNode);
  return true;
};

bool 
XMLIStream::updateVars(xmlNodePtr pNode) {
  if (mCurrentNode == NULL)
    return false;
  for(VarMap::iterator it = mBindings.begin(); it != mBindings.end(); it++) {
    if (!xmlStrcmp((xmlChar*)(it->mTable), pNode->name)) {
      xmlAttrPtr attr = findAttribute(mCurrentNode, it->mField);
      dba::StoreableFilterBase& filter(*(it->mFilter));
      if (attr != NULL) {
        xmlChar* xmldata = xmlNodeGetContent(attr->children);
        if (xmldata == NULL) {
          filter.fromNull();
        } else {
          std::string data((const char*)xmldata);
          filter.fromString(mConvSpecs, data);
        };
      } else {
        filter.fromNull();
      };
    };
  };
  return true;
};

void 
XMLIStream::setIdsCondition(const char* pFKeyName, id pRelationId, const std::vector<id>& pIds) {
  //no implementation needed
};

XMLIStream::~XMLIStream()
{
}


} //namespace
