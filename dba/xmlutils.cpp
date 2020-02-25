//
// C++ Implementation: xmlutils
//
// Description: 
//
//
// Author: Lukasz Michalski <lm@zork.pl>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "dba/xmlutils.h"
#include <string.h>
#include <malloc.h>

namespace dba {

char*
XMLUtils::getNsFromName(const char* pName) {
  const char* nspos = strchr(pName,':');
  if (nspos == NULL)
    return NULL;
  int len = nspos - pName;
  char* ns = new char[len+1];
  strncpy(ns,pName,len);
  ns[len] = '\0';
  return ns;
};

char*
XMLUtils::getNameWithoutNs(const char* pName) {
  const char* nspos = strchr(pName,':');
  if (nspos == NULL)
    return strdup(pName);
  int len = strlen(pName) - (nspos - pName);
  char* name = new char[len+1];
  strncpy(name,nspos+1,len);
  name[len] = '\0';
  return name;
};


//0 - ns
//1 - name
void 
XMLUtils::splitName(const char* pName, char* pRetData[1]) {
  const char* nspos = strchr(pName,':');
  if (nspos == NULL) {
    pRetData[1] = strdup(pName);
    pRetData[0] = NULL;
  };
  int nslen = nspos - pName;
  char* nsprefix = (char*)malloc(nslen+1);
  strncpy(nsprefix,pName,nslen+1);
  nsprefix[nslen] = '\0';
  int namelen = strlen(nspos+1);
  char* outname = (char*)malloc(namelen+1);
  strncpy(outname,nspos+1,namelen+1);
  pRetData[0] = nsprefix;
  pRetData[1] = outname;
};

} //namespace
