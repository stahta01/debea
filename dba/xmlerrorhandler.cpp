//
// C++ Implementation: xmlerrorhandler
//
// Description: 
//
//
// Author: Lukasz Michalski <lm@zork.pl>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "dba/xmlerrorhandler.h"
#include <libxml/xmlerror.h>

extern "C" {
  #include <stdio.h>
  #include <stdlib.h>
  #include <stdarg.h>
  #include <string.h>
};

namespace dba {

void 
errorHandler_c(void* ctx, const char* msg, ...) 
{
  if (ctx == NULL) {
    throw APIException("Fatal: No xml context for error message");
  };
  XMLErrorHandler* handler = (XMLErrorHandler*)ctx;
  va_list ap;
  va_start(ap, msg);
  handler->onError(msg,ap);
  va_end(ap);
};

XMLErrorHandler::XMLErrorHandler() 
{
}

void
XMLErrorHandler::onError(const char* pMsg, va_list pArgs) {
  char* msg = make_message(pMsg,pArgs);
  mLastError += msg;
  free(msg);
};

char*
XMLErrorHandler::make_message(const char *fmt, va_list pArgs) {
  static const int size = 8000;
  char buffer[size];  
  memset(buffer,'\0',size-1);
#ifdef WIN32   
    _vsnprintf (buffer, size, fmt, pArgs);
#else
    vsnprintf (buffer, size, fmt, pArgs);
#endif    
  buffer[size-1] = '\0';
  return strdup(buffer);
};

XMLErrorHandler::~XMLErrorHandler()
{
}


XMLErrorHandler::ErrorContext::ErrorContext(XMLErrorHandler* pHandler) 
  : mHandler(pHandler)
{
  xmlSetGenericErrorFunc(pHandler,errorHandler_c);
};

XMLException
XMLErrorHandler::ErrorContext::createException() const {
  return XMLException(mHandler->mLastError.c_str());
};

bool
XMLErrorHandler::ErrorContext::wasError() const {
  return mHandler->mLastError.size() != 0;
};

XMLErrorHandler::ErrorContext::~ErrorContext() {
  xmlSetGenericErrorFunc(NULL,errorHandler_c);
};

} //namespace
