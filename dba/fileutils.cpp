//
// C++ Implementation: fileutils
//
// Description: 
//
//
// Author: Lukasz Michalski <lm@zork.pl>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
extern "C" {
  #include <errno.h>
  #include <sys/stat.h>
}

#include <string>

#include "dba/fileutils.h"

namespace dba {

bool 
FileUtils::exists(const char* pFilename) {
  errno = 0;
  struct stat buf;
  int ret = stat(pFilename,&buf);
  if (ret != 0) {
    if (errno == ENOENT) {
      return false;
    } else {
      std::string err("Unknown error accessing file ");
      err += pFilename;
      throw FileException(err.c_str());
    };
  } else {
    return true;
  };
}

} //namespace
