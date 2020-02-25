// File: datetime_filter.cpp
// Purpose: StoreableFilter for struct tm
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "conversion.h"
#include "datetime_filter.h"
#include "archive.h"

namespace dba {

DateTime::DateTime(::tm& pDate)
  : StoreableFilter<tm>(pDate)
{
}

std::string 
DateTime::toString(const ConvSpec& pSpec) const throw (StoreableFilterException) {
  char buffer[64];
  int size;
  tm moved_tm(*mMember);
  //we use mktime to add or substract gmt offset to broken down
  //structure.
  if (pSpec.mGMTOffset != 0) {
    //on unix machines mktime sets time zone and 
    //treats argument as UTC date if time zone was not set
    //this causes date change, so we reassign original date values
    //after that change and add GMT offset to that values
    if (mktime(&moved_tm) == -1) {
      throw StoreableFilterException("Cannot normalize struct tm for printing");
    };
    moved_tm.tm_sec = mMember->tm_sec + pSpec.mGMTOffset;
    moved_tm.tm_min = mMember->tm_min;
    moved_tm.tm_hour = mMember->tm_hour;
    moved_tm.tm_mday = mMember->tm_mday;
    moved_tm.tm_mon = mMember->tm_mon;
    moved_tm.tm_year = mMember->tm_year;
    if (mktime(&moved_tm) == -1) {
      throw StoreableFilterException("Cannot normalize struct tm for printing");
    };
  };
  #ifdef _WIN32
  if (moved_tm.tm_year < 0)
    throw StoreableFilterException("Cannot convert struct tm with year < 1970 to string");
  #endif
  size = strftime(buffer,64,pSpec.mTimestampFormat,&moved_tm);
  if (size == 0)
    throw StoreableFilterException("DateTime::toString: strftime failed");
  return std::string(buffer,size);
};

bool 
DateTime::isNull() const {
  if (mMember->tm_year != -1) return false;
  if (mMember->tm_mon != -1) return false;
  if (mMember->tm_mday != -1) return false;
  if (mMember->tm_hour != -1) return false;
  if (mMember->tm_min != -1) return false;
  if (mMember->tm_sec != -1) return false;
  return true;
};

void 
DateTime::fromInt(const ConvSpec& pSpec, int pData) throw (StoreableFilterException) {
  time_t data(pData);
  data -= pSpec.mGMTOffset;
  dba_localtime(&data,mMember);
};

void
DateTime::fromDouble(const ConvSpec& pSpec, double pData) throw (StoreableFilterException) {
  time_t data((time_t)pData);
  data -= pSpec.mGMTOffset;
  dba_localtime(&data,mMember);
};

void 
DateTime::fromString(const ConvSpec& pSpec, const std::string& pData) throw (StoreableFilterException) {
  *mMember = parseISODateTime(pData.c_str());
  if (pSpec.mGMTOffset == 0)
    return;
  time_t data = mktime(mMember);
  if (data == -1) {
    throw StoreableFilterException("Date is out of mktime() range");
  };
  data -= pSpec.mGMTOffset;
  dba_localtime(&data,mMember);
};

void 
DateTime::fromDate(const ConvSpec& pSpec, const tm& pDate) throw (StoreableFilterException) {
  if (pSpec.mGMTOffset != 0) {
    time_t data = mktime((tm*)&pDate);
    data -= pSpec.mGMTOffset;
    dba_localtime(&data,mMember);
  } else {
    *mMember = pDate;
  };
};

void 
DateTime::fromNull() throw (StoreableFilterException) {
  mMember->tm_year = -1;
  mMember->tm_mon = -1;
  mMember->tm_mday = -1;
  mMember->tm_hour = -1;
  mMember->tm_min = -1;
  mMember->tm_sec = -1;
};

DateTime::~DateTime()
{
}


}
