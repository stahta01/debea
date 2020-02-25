// File: defs.cpp
// Purpose: Missing core functions and utilites
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/defs.h"

namespace dba {

#if !defined(HAVE_LOCALTIME_R)

struct tm* 
dba_localtime(const time_t* pFrom, struct tm* pTo) {
  *pTo = *(localtime(pFrom));
  return pTo;
};

#else

struct tm* 
dba_localtime(const time_t* pFrom, struct tm* pTo) {
  return localtime_r(pFrom,pTo);
};

#endif //HAVE_LOCALTIME_R

} //namespace

