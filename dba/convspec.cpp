// File: convspec.cpp
// Purpose: Archive conversion specification
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/convspec.h"
#include <stdlib.h>

#ifdef _WIN32
  #include <windows.h> //for code page constants
#endif

namespace dba {

ConvSpec::ConvSpec()
{
  //default values
  mTimestampFormat = "%Y-%m-%d %H:%M:%S";
  mDateFormat = "%Y-%m-%d";
  mGMTOffset = 0;
  mDecimalPoint = '.';
  mDbCharset = UTF8;
}

void 
ConvSpecContainer::setConversionSpecs(const ConvSpec& pSpecs) {
  mConvSpecs = pSpecs;
};

const ConvSpec& 
ConvSpecContainer::getConversionSpecs() const {
  return mConvSpecs;
};

ConvSpec& 
ConvSpecContainer::conversionSpecs() {
  return mConvSpecs;
};

#ifdef _WIN32
/*static*/
int 
ConvSpec::getMSWCodePage(dba::ConvSpec::charset pCharset) {
  //reference: http://msdn2.microsoft.com/en-us/library/ms776446(VS.85).aspx
  switch (pCharset) {
    case dba::ConvSpec::ISO8859_1: return 28591;
    case dba::ConvSpec::ISO8859_2: return 28592;
    case dba::ConvSpec::ISO8859_3: return 28593;
    case dba::ConvSpec::ISO8859_4: return 28594;
    case dba::ConvSpec::ISO8859_5: return 28595;
    case dba::ConvSpec::ISO8859_6: return 28596;
    case dba::ConvSpec::ISO8859_7: return 28597;
    case dba::ConvSpec::ISO8859_8: return 28598;
    case dba::ConvSpec::ISO8859_9: return 28599;
    case dba::ConvSpec::ISO8859_10: return 28600;
    case dba::ConvSpec::ISO8859_11: return 874;
    case dba::ConvSpec::ISO8859_13: return 28603;
    //case dba::ConvSpec::ISO8859_14: return ; ??
    case dba::ConvSpec::ISO8859_15: return 28605;
  
    case dba::ConvSpec::KOI8: return 20866;
    case dba::ConvSpec::KOI8_U: return 21866;
    //case dba::ConvSpec::BULGARIAN: return wxFONTENCODING_BULGARIAN; ??
  
    case dba::ConvSpec::CP437: return 437;
    case dba::ConvSpec::CP850: return 850;
    case dba::ConvSpec::CP852: return 852;
    case dba::ConvSpec::CP855: return 855;
    case dba::ConvSpec::CP866: return 866;
    case dba::ConvSpec::CP874: return 874;
    case dba::ConvSpec::CP932: return 932;
    case dba::ConvSpec::CP936: return 936;
    case dba::ConvSpec::CP949: return 949;
    case dba::ConvSpec::CP950: return 950;
    case dba::ConvSpec::CP1250: return 1250;
    case dba::ConvSpec::CP1251: return 1251;
    case dba::ConvSpec::CP1252: return 1252;
    case dba::ConvSpec::CP1254: return 1254;
    case dba::ConvSpec::CP1255: return 1255;
    case dba::ConvSpec::CP1256: return 1256;
  
    case dba::ConvSpec::UTF7: return CP_UTF7;
    case dba::ConvSpec::UTF8: return CP_UTF8;
    case dba::ConvSpec::EUC_JP: return 20932;
    default: return -1;
  };
};
#endif

const char* 
ConvSpec::getUnixLCType(dba::ConvSpec::charset pCharset) {
  switch (pCharset) {
    case dba::ConvSpec::ISO8859_1: return "en_US.iso88591";
    case dba::ConvSpec::ISO8859_2: return "pl_PL.iso88592";
    case dba::ConvSpec::ISO8859_3: return "eo.iso88593";
    case dba::ConvSpec::ISO8859_4: return "C"; //?
    case dba::ConvSpec::ISO8859_5: return "ru_RU.iso88595";
    case dba::ConvSpec::ISO8859_6: return "ar_AE.iso88596";
    case dba::ConvSpec::ISO8859_7: return "el_GR.iso88597";
    case dba::ConvSpec::ISO8859_8: return "he_IL.iso88598";
    case dba::ConvSpec::ISO8859_9: return "tr_TR.iso88599";
    case dba::ConvSpec::ISO8859_10: return "tn_ZA.iso885910";
    case dba::ConvSpec::ISO8859_11: return "th_TH.iso885911";
    case dba::ConvSpec::ISO8859_13: return "lt_LT.iso885913";
    case dba::ConvSpec::ISO8859_14: return "cy_GB.iso885914";
    case dba::ConvSpec::ISO8859_15: return "gd_GB.iso885915";
  
    case dba::ConvSpec::KOI8: return "ky_KG.koi8k"; 
    case dba::ConvSpec::KOI8_U: return "uk_UA.koi8u";
    //case dba::ConvSpec::BULGARIAN: return wxFONTENCODING_BULGARIAN; ??
  
    //case dba::ConvSpec::CP437: return 437;
    //case dba::ConvSpec::CP850: return 850;
    //case dba::ConvSpec::CP852: return 852;
    //case dba::ConvSpec::CP855: return 855;
    //case dba::ConvSpec::CP866: return 866;
    //case dba::ConvSpec::CP874: return 874;
    //case dba::ConvSpec::CP932: return 932;
    //case dba::ConvSpec::CP936: return 936;
    //case dba::ConvSpec::CP949: return 949;
    //case dba::ConvSpec::CP950: return 950;
    //case dba::ConvSpec::CP1250: return 1250;
    //case dba::ConvSpec::CP1251: return 1251;
    //case dba::ConvSpec::CP1252: return 1252;
    //case dba::ConvSpec::CP1254: return 1254;
    //case dba::ConvSpec::CP1255: return 1255;
    //case dba::ConvSpec::CP1256: return 1256;
  
    //case dba::ConvSpec::UTF7: return CP_UTF7;
    case dba::ConvSpec::UTF8: return "en_US.utf8";
    case dba::ConvSpec::EUC_JP: return "ja_JP.eucjp";
    default: return NULL;
  };
};

const char* 
ConvSpec::getXmlEncodingName(dba::ConvSpec::charset pCharset) {
  switch (pCharset) {
    case dba::ConvSpec::ISO8859_1: return "iso-8859-1";
    case dba::ConvSpec::ISO8859_2: return "iso-8859-2";
    case dba::ConvSpec::ISO8859_3: return "iso-8859-3";
    case dba::ConvSpec::ISO8859_4: return "iso-8859-4"; 
    case dba::ConvSpec::ISO8859_5: return "iso-8859-5";
    case dba::ConvSpec::ISO8859_6: return "iso-8859-6";
    case dba::ConvSpec::ISO8859_7: return "iso-8859-7";
    case dba::ConvSpec::ISO8859_8: return "iso-8859-8";
    case dba::ConvSpec::ISO8859_9: return "iso-8859-9";
    case dba::ConvSpec::ISO8859_10: return "iso-8859-10";
    case dba::ConvSpec::ISO8859_11: return "iso-8859-11";
    case dba::ConvSpec::ISO8859_13: return "iso-8859-13";
    case dba::ConvSpec::ISO8859_14: return "iso-8859-14";
    case dba::ConvSpec::ISO8859_15: return "iso-8859-15";
  
    //case dba::ConvSpec::KOI8: return ?; 
    //case dba::ConvSpec::KOI8_U: return ?;
    //case dba::ConvSpec::BULGARIAN: return ?
  
    //case dba::ConvSpec::CP437: return ?;
    //case dba::ConvSpec::CP850: return ?;
    //case dba::ConvSpec::CP852: return ?;
    //case dba::ConvSpec::CP855: return ?;
    //case dba::ConvSpec::CP866: return ?;
    //case dba::ConvSpec::CP874: return ?;
    //case dba::ConvSpec::CP932: return ?;
    //case dba::ConvSpec::CP936: return ?;
    //case dba::ConvSpec::CP949: return ?;
    //case dba::ConvSpec::CP950: return ?;
    case dba::ConvSpec::CP1250: return "win-1250";
    case dba::ConvSpec::CP1251: return "win-1251";
    case dba::ConvSpec::CP1252: return "win-1252";
    case dba::ConvSpec::CP1254: return "win-1254";
    case dba::ConvSpec::CP1255: return "win-1255";
    case dba::ConvSpec::CP1256: return "win-1256";
  
    case dba::ConvSpec::UTF7: return "utf-7";
    case dba::ConvSpec::UTF8: return "utf-8";
    //case dba::ConvSpec::EUC_JP: return "?";
    default: return NULL;
  };
};


} //namespace

