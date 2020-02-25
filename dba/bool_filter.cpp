// File: bool_filter.cpp
// Purpose: Implementation of StoreableFilter for bool type
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/bool_filter.h"

namespace dba {

Bool::Bool(bool& pValue)
 : StoreableFilter<bool>(pValue)
{
}

std::string 
Bool::toString(const ConvSpec& pSpec) const throw (StoreableFilterException) {
  return *mMember ? "1" : "0";
};

bool 
Bool::isNull() const {
  return false;
};

void 
Bool::fromInt(const ConvSpec& pSpec, int pData) throw (StoreableFilterException) {
  *mMember = pData == 1 ? true : false;
};

void 
Bool::fromDouble(const ConvSpec& pSpec, double pData) throw (StoreableFilterException) {
  *mMember = pData == 1 ? true : false;
};

void 
Bool::fromString(const ConvSpec& pSpec, const std::string& pData) throw (StoreableFilterException) {
  if (pData == "true")
    *mMember = true;
  else if (pData == "1")
    *mMember = true;
  else if (pData == "t")
    *mMember = true;
  else
    *mMember = false;
};

void 
Bool::fromNull() throw (StoreableFilterException) {
  *mMember = false;
};

Bool::~Bool()
{
}


}
