// File: string_filter.cpp
// Purpose: Storeable filter for std::string
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/conversion.h"
#include "dba/string_filter.h"
#include "dba/archive.h"

namespace dba {

String::String(std::string& pData)
  : StoreableFilter<std::string>(pData)
{
}

std::string 
String::toString(const ConvSpec& pSpec) const throw (StoreableFilterException) {
  return *mMember;
};

bool 
String::isNull() const {
  return mMember->empty();
};

void 
String::fromInt(const ConvSpec& pSpec, int pData) throw (StoreableFilterException) {
  try {
    convert(pData,*mMember);  
  } catch (const ConversionException& pEx) {
    throw StoreableFilterException(pEx.what());
  };
};

void 
String::fromDouble(const ConvSpec& pSpec, double pData) throw (StoreableFilterException) {
  try {
    convert(pData,*mMember);  
  } catch (const ConversionException& pEx) {
    throw StoreableFilterException(pEx.what());
  };
};

void 
String::fromString(const ConvSpec& pSpec, const std::string& pData) throw (StoreableFilterException) {
  *mMember = pData;
};

void 
String::fromDate(const ConvSpec& pSpec, const tm& pDate) throw (StoreableFilterException) {
  char buffer[64];
  int size;
  #ifdef _WIN32
  if (pDate.tm_year < 0)
    throw StoreableFilterException("Cannot convert struct tm with year < 1970");
  #endif
  size = strftime(buffer,64,pSpec.mTimestampFormat,&(tm&)pDate);
  if (size == 0)
    throw StoreableFilterException("Conversion from struct tm to std::string failed");
  *mMember = std::string(buffer,size);
};

void
String::fromNull() throw (StoreableFilterException) {
  mMember->erase();
};

String::~String()
{
}


}
