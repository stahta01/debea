// File: double_filter.cpp
// Purpose: StoreableFilter for double
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#include "dba/conversion.h"
#include "dba/double_filter.h"
#include <string.h>
#include <locale.h>
#include <limits.h>

namespace dba {

Double::Double(double& pDouble)
 : StoreableFilter<double>(pDouble)
{
}

std::string 
Double::toString(const ConvSpec& pSpec) const throw (StoreableFilterException) {
  try {
    std::string dest;
    convert(*mMember,dest);
    struct lconv* l = localeconv();
    //locale hack
    char* lptr = l->decimal_point;
    while((*lptr != '\0')&&(*lptr != CHAR_MAX)) {
      if (*lptr != pSpec.mDecimalPoint) {
        std::string::size_type pos = 0;
        while( (pos = dest.find(*lptr, pos)) != std::string::npos ) {
          dest[pos] = pSpec.mDecimalPoint;
        };
      };
      lptr++;
    };
    lptr = l->thousands_sep;
    while((*lptr != '\0')&&(*lptr != CHAR_MAX)) {
      std::string::size_type pos = 0;
      while( (pos = dest.find(*lptr, pos)) != std::string::npos ) {
        dest.erase(pos);
      };
      lptr++;
    };
    return dest;
  } catch (ConversionException& pEx) {
    throw StoreableFilterException(pEx.what());
  };
};

bool 
Double::isNull() const {
  return false;
};

void 
Double::fromInt(const ConvSpec& pSpec, int pData) throw (StoreableFilterException) {
  *mMember = (int)pData;
};

void
Double::fromDouble(const ConvSpec& pSpec, double pData) throw (StoreableFilterException) {
  *mMember = pData;
};

void 
Double::fromString(const ConvSpec& pSpec, const std::string& pData) throw (StoreableFilterException) {
  try {
    convert(pData,*mMember,&pSpec.mDecimalPoint);
  } catch (ConversionException& pEx) {
    throw StoreableFilterException(pEx.what());
  };
};

void 
Double::fromDate(const ConvSpec& pSpec, const tm& pDate) throw (StoreableFilterException) {
  time_t val = mktime((tm*)&pDate);
  *mMember = static_cast<double>(val);
};

void 
Double::fromNull() throw (StoreableFilterException) {
  *mMember = 0;
};


Double::~Double()
{
}

Float::Float(float& pFloat)
 : StoreableFilter<float>(pFloat)
{
}

std::string 
Float::toString(const ConvSpec& pSpec) const throw (StoreableFilterException) {
  double d = *mMember;
  Double f(d);
  return f.toString(pSpec);
};

bool 
Float::isNull() const {
  return false;
};

void 
Float::fromInt(const ConvSpec& pSpec, int pData) throw (StoreableFilterException) {
  *mMember = static_cast<float>(pData);
};

void
Float::fromDouble(const ConvSpec& pSpec, double pData) throw (StoreableFilterException) {
  *mMember = static_cast<float>(pData);
};

void 
Float::fromString(const ConvSpec& pSpec, const std::string& pData) throw (StoreableFilterException) {
  double d;
  Double f(d);
  f.fromString(pSpec, pData);
  *mMember = d;
};

void 
Float::fromDate(const ConvSpec& pSpec, const tm& pDate) throw (StoreableFilterException) {
  double d;
  Double f(d);
  f.fromDate(pSpec, pDate);
  *mMember = static_cast<float>(d);
};

void 
Float::fromNull() throw (StoreableFilterException) {
  *mMember = 0;
};

Float::~Float()
{
}


}
