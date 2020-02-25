// File: storeablefilter.cpp
// Purpose: Base class for storeable filters
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/storeablefilter.h"

namespace dba {

void 
StoreableFilterBase::fromInt(const ConvSpec& pSpec, int pData) throw (StoreableFilterException) {
  throw StoreableFilterException("Conversion from integer is not supported by this filter");
};

void 
StoreableFilterBase::fromDouble(const ConvSpec& pSpec, double pData) throw (StoreableFilterException) {
  throw StoreableFilterException("Conversion from double is not supported by this filter");
};

void 
StoreableFilterBase::fromString(const ConvSpec& pSpec, const std::string& pData) throw (StoreableFilterException) {
  throw StoreableFilterException("Conversion from string is not supported by this filter");
};

void 
StoreableFilterBase::fromDate(const ConvSpec& pSpec, const tm& pDate) throw (StoreableFilterException) {
  throw StoreableFilterException("Conversion from date is not supported by this filter");
};


}
