// File: dbplugin.cpp
// Purpose: Support for loading Database drivers
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2005
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)


#include "dba/conversion.h"
#include "dba/dbplugin.h"
#include <stdlib.h>
#include <string.h>

#if !defined(_MSC_VER)
  #include "config.h"

  #if defined(HAVE_DLOPEN)
    #include <dlfcn.h>
  #endif

#endif //_MSC_VER

namespace dba {

#ifdef _MSC_VER
std::string
getLastError(const char* pBegin) {
  LPVOID buf = NULL;
  DWORD dw = GetLastError();
  FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER
                |FORMAT_MESSAGE_FROM_SYSTEM,NULL,
                dw,
                MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                (LPTSTR) &buf,0,NULL
  );

  std::string error(pBegin);
  error += " (code ";
  error += toStr((int)dw);
  error += ")";
  if (NULL != buf) {
    error += ": ";
    error += (const char*)buf;
  };
  LocalFree(buf);
  return error;
};
#endif

DbPlugin::DbPlugin() 
  : mLibHandle(NULL)
{
};

DbPlugin::DbPlugin(const char* pFilename) throw (Exception)
  : mLibHandle(NULL)
{
  load(pFilename);
};

dbaPluginInfo 
DbPlugin::getInfo() const throw (Exception) {

  if (mLibHandle == NULL)
    throw DbPluginException("No dba library loaded");

  dbaPluginInfoFunction plf;
#ifdef _MSC_VER
  plf = (dbaPluginInfoFunction)::GetProcAddress(mLibHandle,TEXT("dbaGetPluginInfo"));
  if (NULL == plf) {
    throw DbPluginException(getLastError("DbaPlugin::getInfo failed"));
  };
#endif
#ifdef HAVE_DLOPEN
  char* error;
  plf = (dbaPluginInfoFunction)dlsym(mLibHandle,"dbaGetPluginInfo");
  if ((error = dlerror()) != NULL)  {
    throw DbPluginException(std::string("DbaPlugin::getInfo failed: ") + error);
  }
#endif
  dbaPluginInfo info = (*plf)();
  return info;  
};

void
DbPlugin::close() {


  /*
    Do not throw on close - if new library will open
    successfully then we end with memory leak only
  */
  if (mLibHandle != NULL) {
#if _POSIX_SOURCE
    dlerror();    /* Clear any existing error */
    if (dlclose(mLibHandle) != 0) {
      fprintf (stderr, "Error closing library: %s\n", dlerror());
    };
#endif
#ifdef _MSC_VER
    if (!::FreeLibrary(mLibHandle)) {
      fprintf (stderr, getLastError("DbPlugin::close failed:").c_str());
    };
#endif
//    std::cerr << "closed lib handle "<< mLibHandle << std::endl;
  };
};

void
DbPlugin::load(const char* pFilename) throw (Exception) {
  close();
  if (pFilename == NULL)
    throw DbPluginException("DbaPlugin::load failed: plugin name is NULL");
  std::string name;
  name.append(pFilename); 
#ifdef _MSC_VER
  name.append(".dll");
  mLibHandle = ::LoadLibrary(TEXT(name.c_str()));
  if (mLibHandle == NULL) {
    throw DbPluginException(getLastError("DbaPlugin::load failed"));
  }
#elif defined(HAVE_DLOPEN)
  #if defined(__APPLE_)
    name.append(".bundle");
  #else
    name.append(".so");
  #endif
  dlerror();    /* Clear any existing error */
  mLibHandle = dlopen(name.c_str(), RTLD_LAZY);
  if (mLibHandle == NULL) {
    throw DbPluginException(std::string("DbaPlugin::load failed: ") + dlerror());
  }
#else
  throw DbPluginException("Runtime library loader not found");
#endif
};

Database*
DbPlugin::createHandle() const throw (Exception) {
  dbaDbCreationFunction cf;
#if defined(_MSC_VER)
  cf = (dbaDbCreationFunction)::GetProcAddress(mLibHandle,TEXT("dbaCreateDb"));
  if (NULL == cf) {
    throw DbPluginException(getLastError("DbaPlugin::createHandle failed"));
  };
#elif defined(HAVE_DLOPEN)
  char* error;
  cf = (dbaDbCreationFunction)dlsym(mLibHandle,"dbaCreateDb");
  if ((error = dlerror()) != NULL)  {
    throw DbPluginException(std::string("DbaPlugin::createHandle failed: ") + error);
  }
#else
  throw DbPluginException("Runtime library loader not found");
#endif
  return (*cf)();
};

void 
DbPlugin::destroyHandle(Database* pDb) {
  dbaDbDestructionFunction df;
#ifdef _MSC_VER
  df = (dbaDbDestructionFunction)::GetProcAddress(mLibHandle,TEXT("dbaDeleteDb"));
  if (NULL == df) {
    throw DbPluginException(getLastError("DbaPlugin::createHandle failed"));
  };
#elif defined(HAVE_DLOPEN)
  char* error;
  df = (dbaDbDestructionFunction)dlsym(mLibHandle,"dbaDeleteDb");
  if ((error = dlerror()) != NULL)  {
    throw DbPluginException(std::string("DbaPlugin::destroyHandle failed: ") + error);
  }
#else
  throw DbPluginException("Runtime library loader not found");
#endif
  return (*df)(pDb);
};


DbPlugin::~DbPlugin() {
};

};
