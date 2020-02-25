// File: conversion.cpp
// Purpose: Various converison utilities
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2001
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/defs.h"
#include "dba/conversion.h"

#include <stdlib.h>
#include <string.h>
#include <cstdio>

using namespace std;

namespace dba {

int
replaceAll(string &s, const string &to_find, const string& repl_with) {
  string::size_type pos = 0;
  int i = 0;
  while( (pos = s.find(to_find, pos)) != string::npos ) {
    s.replace( pos, to_find.size(), repl_with );
    pos += repl_with.size();
    i++;
  };
  return i;
}

void 
trim(string& s) {
  string::size_type first = s.find_first_not_of(' ');
  string::size_type last = s.find_last_not_of(' ');
  if (first < last)
    if (last != s.size())
      s = s.substr(first,last+1);
    else
      s = s.substr(first);
};

static const std::string err_begin = "Conversion of '";
static const std::string err_end = "' to date failed: ";

char* 
parseISODatePart(const char* pStr, tm* pRet) {
  char* it = (char*)pStr;

  if (it == NULL)
    throw ConversionException(err_begin + pStr + err_end + "parse error (null string)");

  //omit all trailing whitespaces 
  if (isspace(*it) && (*it != '\0'))
    it++;
  
  errno = 0;
  pRet->tm_year = strtol(it,&it,10);
  if ((*it == '\0')||(errno)) {
    throw ConversionException(err_begin + pStr + err_end + "parse error (year) " + string(strerror(errno)));
  };
  pRet->tm_year -= 1900;
  it++;

  errno = 0;
  pRet->tm_mon = strtol(it,&it,10);
  if ((*it == '\0')||(errno)) {
    throw ConversionException(err_begin + pStr + err_end + "parse error (month) " + string(strerror(errno)));
  };
  pRet->tm_mon -= 1;
  it++;

  errno = 0;
  pRet->tm_mday = strtol(it,&it,10);
  if (errno) {
    throw ConversionException(err_begin + pStr + err_end + "parse error (day) " + string(strerror(errno)));
  };
  return it;
};

struct tm
parseISODateTime(const char* pDate) {
  struct tm ret;
#ifdef HAVE_GTMOFF_IN_TM
  ret.tm_gmtoff = 0;
#endif
  ret.tm_hour = 0;
  ret.tm_isdst = -1;
  ret.tm_mday = 0;
  ret.tm_min = 0;
  ret.tm_mon = 0;
  ret.tm_sec = 0;
  ret.tm_wday = 0;
  ret.tm_yday = 0;
  ret.tm_year = 0;
  
  char* it = parseISODatePart(pDate,&ret);
  it++;

  errno = 0;
  ret.tm_hour = strtol(it,&it,10);
  if ((*it == '\0')||(errno)) {
    throw ConversionException(err_begin + pDate + err_end + "parse error (day): " + string(strerror(errno)));
  };
  it++;
  
  errno = 0;
  ret.tm_min = strtol(it,&it,10);
  if ((*it == '\0')||(errno)) {
    throw ConversionException(err_begin + pDate + err_end + "parse error (hour): " + string(strerror(errno)));
  };
  it++;

  errno = 0;
  ret.tm_sec = strtol(it,&it,10);
  if (errno) {
    throw ConversionException(err_begin + pDate + err_end + "parse error (min): " + string(strerror(errno)));
  };
  if (*it == '\0') {
    return ret;
  };
  //ignore miliseconds
  if (*it == '.') {
    it++;
    errno = 0;
    strtol(it,&it,10);
    if (errno)
      throw ConversionException(err_begin + pDate + err_end + "parse error (gmt): " + string(strerror(errno)));
    if ((*it == '\0'))
      return ret;
  };
#if defined(HAVE_GTMOFF_IN_TM)
  //those are optional
  ret.tm_gmtoff = 0;
  errno = 0;
  ret.tm_gmtoff = strtol(it,&it,10);
  if (isspace(*it) && (*it != '\0'))
    it++;
  if ((*it != '\0')||(errno)) {
    string err(err_begin + pDate + err_end + "parse error: ");
    if (errno) {
      err += string(strerror(errno));
    } else {
      err += "unparsed chars at end of valid input";
    };
    throw ConversionException(err);
  };
  ret.tm_gmtoff /= 100; // format in pDate: +0100 
  ret.tm_gmtoff *= 60*60;
#endif
  return ret;
};

struct tm
parseISODate(const char* pDate) {
  struct tm ret;
#if defined(HAVE_GTMOFF_IN_TM)
  ret.tm_gmtoff = 0;
#endif
  ret.tm_hour = 0;
  ret.tm_isdst = -1;
  ret.tm_mday = 0;
  ret.tm_min = 0;
  ret.tm_mon = 0;
  ret.tm_sec = 0;
  ret.tm_wday = 0;
  ret.tm_yday = 0;
  ret.tm_year = 0;
  
  parseISODatePart(pDate,&ret);
  return ret;
};

string
strip(const string& s) {
  int start = s.find_first_not_of(" \t");
  int end = s.find_last_not_of(" \t");
  if (start != string::npos)
    return string(s,start,end+1);
  return(string(""));
}

string
getWord(const string& str,unsigned int num) {
  unsigned int start = 0;
  unsigned int end = 0;
  do {
    start = end;
    start = str.find_first_not_of(" \t",start);
    if (start != string::npos)
      end = str.find_first_of(" \t",start);
    else
      end = string::npos;
  } while(num--);
  if ((start != string::npos)&&(end == string::npos))
    return(string(str,start));
  if (start < end)
    return (string(str,start,end-start));
  return(string(""));
};

double
mystrtod(const char* s) {
  errno = 0;
  char* ptr = NULL;
  double d = strtod(s,&ptr);
  if ((*ptr != '\0')||(errno != 0))
    throw ConversionException("cannot convert " + (string)s);
  return d;
};

void
convert(const string& s,long& i, int base) {
  std::string toConvert(strip(s));
  char *ptr = NULL;
  if (*toConvert.c_str() == '\0')
    throw ConversionException("cannot convert '" + s + "' to number");
  errno = 0;
  i = strtol(toConvert.c_str(),&ptr, base);
  if ((*ptr != '\0')||(errno != 0))
    throw ConversionException("cannot convert '" + s + "' to number");
}

void
convert(const string& s,int& i, int base) {
  std::string toConvert(strip(s));
  char *ptr = NULL;
  if (*toConvert.c_str() == '\0')
    throw ConversionException("cannot convert '" + s + "' to number");
  errno = 0;
  i = strtol(toConvert.c_str(),&ptr, base);
  if ((*ptr != '\0')||(errno != 0))
    throw ConversionException("cannot convert '" + s + "' to number");
}

void
convert(const string& s, double& d, const char* pDecPointRepl) {
  std::string toConvert(strip(s));
  if (pDecPointRepl != NULL) {
    struct lconv* l = localeconv();
    char* lptr = l->decimal_point;
    if (*lptr != *pDecPointRepl) {
      size_t pos = toConvert.find(*pDecPointRepl);
      if (pos != std::string::npos) {
        toConvert.replace(pos,1,1,*lptr);
      };
    };
  };
  if (*toConvert.c_str() == '\0')
    throw ConversionException("cannot convert '" + s + "' to double");
  errno = 0;
  d = mystrtod(toConvert.c_str());
};


void
convert(const char* s, int& i, int base) {
  char *ptr = NULL;
  while (isspace(*s) && *s != '\0')
    s++;
  if (*s == '\0')
    throw ConversionException("cannot convert " + (string)s);
  errno = 0;
  i = strtol(s,&ptr, base);
  if ((*ptr != '\0')||(errno != 0))
    throw ConversionException("cannot convert " + (string)s);
};

void
convert(const char* s, long& i, int base) {
  char *ptr = NULL;
  while (isspace(*s) && *s != '\0')
    s++;
  if (*s == '\0')
    throw ConversionException("cannot convert " + (string)s);
  errno = 0;
  i = strtol(s,&ptr, base);
  if ((*ptr != '\0')||(errno != 0))
    throw ConversionException("cannot convert " + (string)s);
};


void
convert(const char* s, double& d, const char* pDecPointRepl) {
  while (isspace(*s) && *s != '\0')
    s++;
  if (*s == '\0')
    throw ConversionException("cannot convert " + (string)s);
  errno = 0;
  if (pDecPointRepl != NULL) {
    struct lconv* l = localeconv();
    char* lptr = l->decimal_point;
    if (*lptr != *pDecPointRepl) {
      char* s_dup = strdup(s);
      char* to_repl = strchr(s_dup,*pDecPointRepl);
      if (to_repl != NULL) {
        *to_repl = *lptr;
      };
      d = mystrtod(s_dup);
      free(s_dup);
    } else {
      d = mystrtod(s);
    };
  } else {
    d = mystrtod(s);
  };
};


void
convert(int i, char* s) {
  sprintf(s,"%d",i);
}

void
convert(long l, char* s) {
  sprintf(s,"%ld",l);
}

void
convert(double d, char* s) {
  sprintf(s,"%f",d);
};

void
convert(int i, string& s) {
  const int size = 24;
  char buf[size]; 
  sprintf(buf,"%d",i);
  s = string(buf);
}

void
convert(long l, string& s) {
  const int size = 24;
  char buf[size]; 
  sprintf(buf,"%ld",l);
  s = string(buf);
}

void
convert(double d, string& s) {
  const int size = 24;
  char buf[size]; 
  int written = sprintf(buf,"%f",d);
  if (written <= 0) {
    throw ConversionException("Error converting double value to string");
  };
  //remove trailing zeroes
  --written;
  while(buf[written] == '0' && written != 0) {
    buf[written] = 0x0;
    written--;
  };
  //remove dot if there is no fractional part
  const lconv* loc = localeconv();
  
  if (buf[written] == *(loc->decimal_point))
    buf[written] = 0x0;
   
  s = string(buf);
};


string
escape_sql(const string& s) {
  string ret = s;
  unsigned int pos = ret.find("'");
  while((pos < ret.length())&&(pos != string::npos)) {
    ret.insert(pos,"'");
    pos += 2;
    pos = ret.find("'",pos);
  };
  return ret;
};

string
toStr(double d,const char* format) {
  const int size = 24;
  char buf[size]; 
  sprintf(buf,format,d);
  return string(buf);
};

string
toStr(long i,const char* format) {
  const int size = 24;
  char buf[size]; 
  sprintf(buf,format,i);
  return string(buf);
};

string
toStr(int i,const char* format) {
  const int size = 24;
  char buf[size]; 
  sprintf(buf,format,i);
  return string(buf);
};

string
toStr(unsigned i,const char* format) {
  const int size = 24;
  char buf[size];
  sprintf(buf,format,i);
  return string(buf);
};

};//namespace

