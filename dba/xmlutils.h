//
// C++ Interface: xmlutils
//
// Description: 
//
//
// Author: Lukasz Michalski <lm@zork.pl>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DBAXMLUTILS_H
#define DBAXMLUTILS_H

#include "dba/defs.h"

namespace dba {

/**
varioud utils for xmlarchive and streams

	@author Lukasz Michalski <lm@zork.pl>
*/
class dbaDLLEXPORT XMLUtils {
  public:
    /**
      Get namespace prefix from name
      @return NULL if there is no prefix or newly allocated char* otherwise
    */
    static char* getNsFromName(const char* pName);
    static char* getNameWithoutNs(const char* pName);
    /**
      Split member name into xml namspace prefix part and name part
    */
    static void splitName(const char* pName, char* pRetData[1]);
};

} //namespace

#endif
