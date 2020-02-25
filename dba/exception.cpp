// File: exception.cpp
// Purpose: Exception classes
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/exception.h"

namespace dba {

Exception::Exception(const std::string& pText)
 : mCause(pText)
{}

Exception::Exception(const char* pText) 
  : mCause(pText)
{};

} //namespace

