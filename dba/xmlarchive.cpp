//
// C++ Implementation: xmlarchive
//
// Description: 
//
//
// Author: Lukasz Michalski <lm@zork.pl>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "dba/xmlarchive.h"
#include "dba/fileutils.h"
#include "dba/xmlistream.h"
#include "dba/xmlostream.h"

namespace dba {

XMLArchive::XMLArchive()
  : Archive(),
    mDocument(NULL),
    mRootNode(NULL),
    mUseElements(false)
{
  mConvSpecs.mTimestampFormat = "%Y-%m-%dT%H:%M:%S";
  mRootNodeName = xmlStrdup((xmlChar*)"dba");
}

void
XMLArchive::setRootNodeName(const char* pName) {
  xmlFree(mRootNodeName);
  mRootNodeName = xmlStrdup((xmlChar*)pName);
};

void
XMLArchive::open(xmlDocPtr pDocument) {
  mDocument = pDocument;
  xmlNodePtr root = xmlDocGetRootElement(mDocument);
  if (root == NULL)
    if (mRootNodeName != NULL) {
      root = xmlNewDocNode(mDocument,NULL,mRootNodeName,NULL);
    } else {
      root = xmlNewDocNode(mDocument,NULL,(xmlChar*)"__dba_for_replace",NULL);
    };
  xmlDocSetRootElement(mDocument,root);
  if ((mRootNodeName != NULL) && (xmlStrcmp(mRootNodeName,root->name)))
    throw DataException("Wrong root node name");
  updateEncoding();
  mRootNode = xmlDocGetRootElement(mDocument);
};

void 
XMLArchive::open(const char* pOpenStr) {
  mFilename = pOpenStr;
  close();
  //must be after close()
  ErrorContext c(this);
  if (FileUtils::exists(pOpenStr)) {
    mDocument = xmlParseFile(pOpenStr);
    if (mDocument == NULL) {
      if (c.wasError())
        throw c.createException();
      else
        throw XMLException("Error reading file");
    };
    xmlNodePtr root = xmlDocGetRootElement(mDocument);
    if (root == NULL)
      throw DataException("Root node not found");
    if (root->name == NULL)
      throw DataException("Root node has no name");
    if ((mRootNodeName != NULL) && (xmlStrcmp(mRootNodeName,root->name)))
      throw DataException("Wrong root node name");
  } else {
    mDocument = xmlNewDoc((const xmlChar*)"1.0");
    xmlNodePtr node;
    if (mRootNodeName != NULL) {
      node = xmlNewDocNode(mDocument,NULL,mRootNodeName,NULL);
    } else {
      node = xmlNewDocNode(mDocument,NULL,(xmlChar*)"__dba_for_replace",NULL);
    };
    xmlDocSetRootElement(mDocument,node);
  };
  updateEncoding();
  mRootNode = xmlDocGetRootElement(mDocument);
};

void 
XMLArchive::addNamespace(const char* pName, const char* pPrefix) {
  if (!isOpen())
    throw APIException("XMLArchive must be open to add namespace to it");
  xmlNsPtr ret = xmlNewNs(mRootNode, (xmlChar*)pName, (xmlChar*)pPrefix);
  if (ret == NULL)
    throw XMLException("Failed to add namespace to archive");
};

void
XMLArchive::close() {
  if (mDocument != NULL) {
    if (!mFilename.empty()) {
      write();
      xmlFreeDoc(mDocument);
    };
    mDocument = NULL;
  };
};

IStream* 
XMLArchive::getInputStream() {
  xmlNodePtr root = xmlDocGetRootElement(mDocument);
  if (mRootNodeName != NULL)
    root = root->children;
  return new XMLIStream(mDocument,root,mConvSpecs,mUseElements);
};

OStream* 
XMLArchive::getOutputStream() {
  return new XMLOStream(mDocument,mRootNode,mRootNodeName != NULL,mConvSpecs,mUseElements);
};

XMLIStream 
XMLArchive::getIStream() {
  xmlNodePtr root = xmlDocGetRootElement(mDocument);
  if (mRootNodeName != NULL)
    root = root->children;
  return XMLIStream(mDocument,root,mConvSpecs,mUseElements);
};

XMLOStream 
XMLArchive::getOStream() {
  XMLOStream ret(mDocument,mRootNode,mRootNodeName == NULL,mConvSpecs,mUseElements);
  return ret;
};

void
XMLArchive::updateEncoding() {
  if (mDocument->encoding == NULL)
    return;
  if (xmlStrcasecmp((xmlChar*)"utf-8",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::UTF8;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-1",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_1;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-2",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_2;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-3",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_3;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-4",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_4;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-5",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_5;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-6",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_6;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-7",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_7;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-8",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_8;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-9",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_9;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-10",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_10;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-11",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_11;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-13",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_13;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-14",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_14;
  else if (xmlStrcasecmp((xmlChar*)"iso-8859-15",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::ISO8859_15;
  else if (xmlStrcasecmp((xmlChar*)"win-1250",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::CP1250;
  else if (xmlStrcasecmp((xmlChar*)"win-1251",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::CP1251;
  else if (xmlStrcasecmp((xmlChar*)"win-1252",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::CP1252;
  else if (xmlStrcasecmp((xmlChar*)"win-1254",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::CP1254;
  else if (xmlStrcasecmp((xmlChar*)"win-1255",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::CP1255;
  else if (xmlStrcasecmp((xmlChar*)"win-1256",mDocument->encoding) == 0)
    mConvSpecs.mDbCharset = ConvSpec::CP1256;
};

void
XMLArchive::write() {
  ErrorContext c(this);
  int result = xmlSaveFileEnc(mFilename.c_str(), mDocument, ConvSpec::getXmlEncodingName(mConvSpecs.mDbCharset));
  if (result == -1) {
    if (c.wasError())
      throw c.createException();
    else
      throw XMLException("Unknown error when writing xml data to file");
  };
};

XMLArchive::~XMLArchive()
{
  close();
  xmlFree(mRootNodeName);
}


} //namespace
