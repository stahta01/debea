// File: connectstring.cpp
// Purpose: SQL database connect string
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2004
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/connectstring.h"
#include "dba/connectstringparser.h"

namespace dba {

ConnectString::ConnectString(const char* pStr)
{
  ConnectStringParser parser(pStr);
  mParams = parser.parse();
}

std::string
ConnectString::getParam(const char* pStr) const {
  std::map<std::string, std::string>::const_iterator it = mParams.find(pStr);
  if (it != mParams.end())
    return it->second;
  return std::string();
};

std::string
ConnectString::username() const {
  return getParam("user");
};

std::string
ConnectString::password() const {
  return getParam("password");
};

std::string
ConnectString::dbname() const {
  return getParam("dbname");
};

std::string
ConnectString::host() const {
  return getParam("host");
};

std::string
ConnectString::port() const {
  return getParam("port");
};

};
