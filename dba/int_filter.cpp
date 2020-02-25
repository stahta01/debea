// File: int_filter.cpp
// Purpose: StoreableFilter for int
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/conversion.h"
#include "dba/int_filter.h"

namespace dba {

Int::Int(int& pData)
  : StoreableFilter<int>(pData)
{
}

Int::Int(unsigned int& pData)
  : StoreableFilter<int>((int&)pData)
{
};


std::string 
Int::toString(const ConvSpec& pSpec) const throw (StoreableFilterException) {
  std::string dest;
  try {
    convert(*mMember,dest);
  } catch (ConversionException& pEx) {
    throw StoreableFilterException(pEx.what());
  };
  return dest;
};

bool 
Int::isNull() const {
  return false;
};

void 
Int::fromInt(const ConvSpec& pSpec, int pData) throw (StoreableFilterException) {
  *mMember = pData;
};

void
Int::fromDouble(const ConvSpec& pSpec, double pData) throw (StoreableFilterException) {
  *mMember = (int)pData;
};

void 
Int::fromString(const ConvSpec& pSpec, const std::string& pData) throw (StoreableFilterException) {
  try {
    convert(pData,*mMember);
  } catch (ConversionException& pEx) {
    throw StoreableFilterException(pEx.what());
  };
};

void 
Int::fromDate(const ConvSpec& pSpec, const tm& pDate) throw (StoreableFilterException) {
  try {
    time_t val = mktime(&(tm&)pDate);
    *mMember = static_cast<int>(val);
  } catch (ConversionException& pEx) {
    throw StoreableFilterException(pEx.what());
  };
};

void 
Int::fromNull() throw (StoreableFilterException) {
  *mMember = 0;
};

Int::~Int()
{
}


}
