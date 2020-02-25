// File: localechanger.cpp
// Purpose: Various converison utilities
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2001
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

extern "C" {
  #include <locale.h>
  #include <string.h>
  #include <stdlib.h>
}

#include "dba/localechanger.h"

namespace dba {

LocaleChanger::LocaleChanger(int pCategory, const char* pNewLocale)
  : mSavedLocale(NULL),
    mCategory(pCategory)
{
  if (pNewLocale != NULL) {
    char* old_locale;
    old_locale = setlocale(mCategory,NULL);
    mSavedLocale = strdup(old_locale);
    if (mSavedLocale == NULL) {
      return;    
    };
    if (setlocale(pCategory,pNewLocale) == NULL) {
      //locale was not changed
      free(mSavedLocale);
      mSavedLocale = NULL;
    };
  };  
}

bool 
LocaleChanger::isOk() const {
  return mSavedLocale != NULL;
};

LocaleChanger::~LocaleChanger()
{
  setlocale(mCategory,mSavedLocale);
  free(mSavedLocale);
}


}
