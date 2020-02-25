// File: membertree.cpp
// Purpose: Utilities for class member tables 
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2002
// License: See COPYING file that comes with this distribution

#include "dba/membertree.h"

#include <iostream>
#include <string.h>

namespace dba {

mt_mlist::mt_mlist() {
  mFirst = new mt_class;
  mFirst -> firstField = new mt_member;
  mFirst -> firstField -> name = NULL;
  mFirst -> firstField -> next = NULL;
  mFirst -> name = NULL;
  mFirst -> next = NULL;
};

mt_mlist::~mt_mlist() {
  mt_class* next;
  mt_member* nf;
  while(mFirst != NULL) {
    next = mFirst -> next;
    while(mFirst -> firstField != NULL) {
      nf = mFirst -> firstField -> next;
      delete mFirst -> firstField;
      mFirst -> firstField = nf;
    };
    delete mFirst;
    mFirst = next;
  };
};

void
mt_mlist::add(const char* table,const char* field,int offset,void* f, int type) {
  mt_class* first = mFirst;
  while(first != NULL) {
    if (first -> name == NULL) {
      first -> name = table;
      add(first,field,offset,f,type);
      return;
    };
    if (!strcmp(first -> name,table)) {
      add(first,field,offset,f,type);
      return;
    };
    first = first -> next;
  };
  mt_class* n = new mt_class;
  n -> name = table;
  n -> firstField = new mt_member;
  n -> firstField -> name = NULL;
  n -> firstField -> next = NULL;
  add(n,field,offset,f,type);
  n -> next = mFirst;
  mFirst = n;
};

void
mt_mlist::clear() {
  mt_class* first = mFirst;
  while(first -> name != NULL) {
    clear(first);
    first = first -> next;    
    if (first == NULL)
      break;
  };
};

void
mt_mlist::clear(mt_class* t) {
  t -> name = NULL;
  mt_member* f = t -> firstField;
  while(f != NULL) {
    f -> name = NULL;
    f = f -> next;
  };
};

void
mt_mlist::add(mt_class* t, const char* name, int offset,void* f,int type) {
  mt_member* n = t -> firstField;
  while(n != NULL) {
    if (n -> name == NULL) {
      n -> name = name;
      n -> offset = offset;
      n -> func = f;
      n -> type = type;
      return;
    };
    n = n -> next;
  };
  n = new mt_member;
  n -> name = name;
  n -> offset = offset;
  n -> type = type;
  n -> next = t -> firstField;
  n -> func = f;
  t -> firstField = n;    
};

mt_member*
mt_mlist::findMember(const char* pClassName, const char* pFieldName) const {
  mt_class* first = mFirst;
  while(first != NULL) {
    if (first -> name == NULL)
      return NULL;      
    if (!strcmp(first -> name,pClassName))
      return findMember(first,pFieldName);
    first = first -> next;
  };
  return NULL;
};

mt_member*
mt_mlist::findMember(mt_class* pTable, const char* pField) const {
  mt_member* n = pTable -> firstField;
  while(n != NULL) {
    if (n -> name == NULL)
      return NULL;
    if (!strcmp(n -> name,pField))
      return n;
    n = n -> next;
  };
  return NULL;
};



void
mt_mlist::addClass(const char* table) {
  mt_class* first = mFirst;
  while(first != NULL) {
    if (first -> name == NULL) {
      //FIXME fill data here instead replacing mFirst.
      break;
    };
    if (!strcmp(first -> name,table)) {
      return;
    };
    first = first -> next;
  };
  mt_class* n = new mt_class;
  n -> name = table;
  n -> firstField = new mt_member;
  n -> firstField -> name = NULL;
  n -> firstField -> next = NULL;
  n -> next = mFirst;
  mFirst = n;
};

void*
mt_mlist::findFuncByOffset(int pOffset) const {
  mt_class* first = mFirst;
  while(first != NULL) {
    if (first -> name == NULL) {
      break;      
    } else {
      void* func = findFuncByOffset(first,pOffset);
      if (func != NULL) {
        return func;
      };
    };
    first = first -> next;
  };
  return NULL;
};

void*
mt_mlist::findFuncByOffset(mt_class* pTable, int pOffset) const {
  mt_member* n = pTable -> firstField;
  while(n != NULL) {
    if (n -> name != NULL) {
      if (n -> offset == pOffset)
        return n -> func;
    };
    n = n -> next;
  };
  return NULL;
};

};//namespace
