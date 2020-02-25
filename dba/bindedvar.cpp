// File: bindedvar.cpp
// Purpose: Base class for binded variables
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/bindedvar.h"

namespace dba {

/**
  Suport for binding primitives
  */
BindedBase::BindedBase(Stream* pOwner, void* pPtr)
  : mOwner(pOwner),
    mVarPtr(pPtr)
{
};

  //FIXME implement auto unbinding. Now when stream and var is created on stack
  //crash might happen because order of destruction is unknown
BindedBase::~BindedBase() {
//  mOwner->unbindVar(mVarPtr);
};

BindedStid::BindedStid(Stream* pOwner, id* pVar) 
  : BindedBase(pOwner,pVar)
{
  mValue = pVar;
};

id
BindedStid::operator=(id i) {
  *mValue = i;
  return *mValue;
};

BindedStid::operator id() const {
  return *mValue;
};


id
BindedStid::getVal() const {
  return *mValue;
};

BindedStid::~BindedStid() {
};


BindedInt::BindedInt(Stream* pOwner, int* pVar) 
  : BindedBase(pOwner,pVar)
{
  mValue = pVar;
};

int
BindedInt::operator=(int i) {
  *mValue = i;
  return *mValue;
};

BindedInt::operator int() const {
  return *mValue;
};


int
BindedInt::getVal() const {
  return *mValue;
};

BindedInt::~BindedInt() {
};

BindedString::BindedString(std::string* pVar) {
  mValue = pVar;
};

const std::string&
BindedString::operator=(const std::string& pStr) {
  *mValue = pStr;
  return *mValue;
};

BindedString::operator std::string() const {
  return *mValue;
};


const std::string&
BindedString::getVal() const {
  return *mValue;
};

BindedString::~BindedString() {
};


}
