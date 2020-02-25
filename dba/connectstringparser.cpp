// File: connectstringparser.cpp
// Purpose: Parsing connect string parameters
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)
//
//
#include "dba/connectstringparser.h"
#include <sstream>

namespace dba {

void
ConnectStringParser::throwError(const char* pCause) {
  std::stringstream err;
  err << "ConnectStringParser: " << pCause << " at character " << mEye - mParams;
  throw ConnectStringParserException(err.str().c_str());
};

std::map<std::string,std::string>
ConnectStringParser::parse() {
  mStack.push_back(SCAN);
  mGotAssignMark = false;
  for(mEye = mParams; *mEye != '\0'; mEye++) {
    switch(*mEye) {
      case '\'':
        switch(mStack.back()) {
          case SCAN:
            mStack.push_back(QUOTE_TEST);
          break;
          case PARAM_NAME:
            mStack.push_back(QUOTE_TEST);
          break;
          case PARAM_VALUE:
            mStack.push_back(QUOTE_TEST);
          break;
          case QUOTED_NAME:
            mStack.push_back(QUOTE_TEST);
          break;
          case QUOTED_VALUE:
            mStack.push_back(QUOTE_TEST);
          break;
          case QUOTE_TEST:
            mStack.pop_back();
            switch(mStack.back()) {
              case SCAN:
                mToken += *mEye;
                mStack.push_back(PARAM_NAME); 
              break;
              case PARAM_NAME:
                mToken += *mEye;
              break;
              case PARAM_VALUE:
                mToken += *mEye;
              break;
              case QUOTED_NAME:
                mToken += *mEye;
              break;
              case QUOTED_VALUE:
                mToken += *mEye;
              break;
            };
          break;
        };
      break;
      case '=':
        switch(mStack.back()) {
          case SCAN:
            if (mName.empty())
              throwError("Missing parameter name");
            else
              mGotAssignMark = true;
          break;
          case PARAM_NAME:
            mStack.pop_back();
            mName = mToken;
            mGotAssignMark = true;
            mToken.erase();
          break;
          case PARAM_VALUE:
            throwError("value with assign must be quoted");
          break;
          case QUOTED_NAME:
            mToken += *mEye;
          break;
          case QUOTED_VALUE:
            mToken += *mEye;
          break;
          case QUOTE_TEST:
            mStack.pop_back();
            switch(mStack.back()) {
              case SCAN:
                if (mName.empty()) {
                  mStack.push_back(PARAM_NAME); 
                  mGotAssignMark = true;
                } else {
                  mStack.push_back(PARAM_VALUE);
                };
                mToken += *mEye;
              break;
              case PARAM_NAME:
                throwError("Unescaped quote in param name");
              break;
              case PARAM_VALUE:
                throwError("Unescaped quote in param value");
              break;
              case QUOTED_NAME:
                mStack.pop_back();
                mName = mToken;
                mGotAssignMark = true;
                mToken.erase();
              break;
              case QUOTED_VALUE:
                if (!mGotAssignMark) {
                  throwError("Syntax error - no assign after parameter name");
                };
                mResult[mName] = mToken;
                mGotAssignMark = false;
                mToken.erase();
                mName.erase();
                mStack.pop_back();
              break;
            };        
          break;
        };      
      break;
      case ' ':
      case '\n':
      case '\r':
        switch(mStack.back()) {
          case SCAN:
          break;
          case PARAM_NAME:
            mStack.pop_back();
            mName = mToken;
            mToken.erase();
          break;
          case PARAM_VALUE:
            mStack.pop_back();
            if (!mGotAssignMark) {
              throwError("Syntax error - no assign after parameter name");
            };
            mResult[mName] = mToken;
            mGotAssignMark = false;
            mToken.erase();
            mName.erase();
          break;
          case QUOTED_NAME:
            mToken += *mEye;
          break;
          case QUOTED_VALUE:
            mToken += *mEye;
          break;
          case QUOTE_TEST:
            mStack.pop_back();
            switch(mStack.back()) {
              case SCAN:
                if (mName.empty())
                  mStack.push_back(QUOTED_NAME); 
                else
                  mStack.push_back(QUOTED_VALUE);
                mToken += *mEye;
              break;
              case PARAM_NAME:
                throwError("Unescaped quote in param name");
              break;
              case PARAM_VALUE:
                throwError("Unescaped quote in param value");
              break;
              case QUOTED_NAME:
                mStack.pop_back();
                mName = mToken;
                mToken.erase();
              break;
              case QUOTED_VALUE:
                if (!mGotAssignMark) {
                  throwError("Syntax error - no assign after parameter name");
                };
                mResult[mName] = mToken;
                mGotAssignMark = false;
                mToken.erase();
                mName.erase();
                mStack.pop_back();
              break;
            };        
          break;
        };      
      break;
      default:
        switch(mStack.back()) {
          case SCAN:
            if (mName.empty())
              mStack.push_back(PARAM_NAME); 
            else
              mStack.push_back(PARAM_VALUE);
          break;
          case QUOTE_TEST:
            mStack.pop_back();
            switch(mStack.back()) {
              case SCAN:
                if (mName.empty())
                  mStack.push_back(QUOTED_NAME); 
                else
                  mStack.push_back(QUOTED_VALUE);
              break;
            };
          break;
        };
        mToken += *mEye;
      break;
    };
  };
  if (!mName.empty()) {
    if (!mGotAssignMark) {
      throwError("Syntax error - no assign after parameter name");
    };
    mResult[mName] = mToken;
  } else if (!mToken.empty()) {
    //we got orphan token
    throwError("Syntax error - no assign after parameter name");
  };
  return mResult;
};

} //namespace
