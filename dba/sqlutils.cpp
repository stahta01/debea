// File: sqlutils.cpp
// Purpose: Various utilites for creation SQL queries
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2002
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#include "dba/sqlutils.h"
#include "dba/stdfilters.h"
#include "dba/archive.h"

namespace dba {

std::string
SQLUtils::escapeSQLData(const std::string& pData) {
  std::string ret(pData);
  replaceAll(ret,"\\","\\\\");
  replaceAll(ret,"'","''");
  return ret;
};

std::string
SQLUtils::escapeSQLLike(const std::string& pData) {
  std::string ret(escapeSQLData(pData));
  replaceAll(ret,"!","!!");
  replaceAll(ret,"_","!_");
  replaceAll(ret,"%","!%");
  return ret;
};


};//namespace

