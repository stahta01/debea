// File: xmlsimple.cpp
// Purpose: Basic example of XMLArchive usage
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2008
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)
 
/**
@file xmlsimple.cpp
*/

#include "dba/dba.h"
#include "dba/xmlarchive.h"
#include <iostream>

//Foo object will contain list of Bar objects, which will create
//simple xml file. Top "foo" element will have attributes mapped to
//class members and will have child "bar" elements, where
//attributes are mapped to Bar class members.

class Bar : public dba::Storeable {
    DECLARE_STORE_TABLE();
  public:
    Bar(int pIndex = 0) : mBarNumber(pIndex) {};
    int mBarNumber;

    bool operator==(const Bar& pObject) const {
      return mBarNumber == pObject.mBarNumber;
    };
};

class Foo : public dba::Storeable {
    DECLARE_STORE_TABLE();
  public:
    Foo(int pBarsCount = 0) {
      for(int i = 0; i < pBarsCount; i++) {
        mBars.push_back(Bar(i));
      };
    };
    int mIntVal;
    std::string mStrVal;
    std::list<Bar> mBars;

    bool operator==(const Foo& pObject) const {
      if (mIntVal != pObject.mIntVal) return false;
      if (mStrVal != pObject.mStrVal) return false;
      if (mBars != pObject.mBars) return false;
      return true;
    };
};

//Class name, parent class name and relation name 
//Foo is root class name, and XMLArchive will create
//'foo_element' element for it
BEGIN_STORE_TABLE(Foo, dba::Storeable, "foo")
  //we store mStrVal as strval attribute
  //using dba::String as conversion filter 
  BIND_STR(Foo::mStrVal,dba::String,"strval")
  //we store mIntVal as int_attr attribute
  //using dba::Int32 as conversion filter 
  BIND_INT(Foo::mIntVal,dba::Int,"int_attr")
  //we store list of "Bar" objects as subelements
  //Third macro argument is NULL, which means that 
  //this will not have parent element but all "Bar" elements
  //will be stored as direct children of Foo
  BIND_COL(Foo::mBars,dba::stdList<Bar>,NULL)
END_STORE_TABLE()

BEGIN_STORE_TABLE(Bar, dba::Storeable, "bar")
  BIND_INT(Bar::mBarNumber,dba::Int,"barnumber")
END_STORE_TABLE()

int 
main (int argc, char** argv) {
  try {
    dba::XMLArchive ar;

    //xmlarchive will create new empty file
    unlink("xmlsimple.xml");

    //this create new empty xml file
    ar.open("xmlsimple.xml");

    Foo c1(4);
    c1.mIntVal = 12;
    c1.mStrVal = "test string";

    //write object to xml file
    dba::XMLOStream ostream(ar.getOStream());
    ostream.open();
    ostream.put(&c1);
    std::cout << "Foo c1 was stored in xmlsimple.xml" << std::endl;

    //commit xml data change to file
    ostream.destroy();
    ar.close();

    //reopen archive - this time we will read from file.
    ar.open("xmlsimple.xml");
    //read first object from file
    Foo c2;
    dba::XMLIStream istream(ar.getIStream());
    istream.open(c2);
    while (istream.getNext(&c2)) {
      std::cout << "readed Foo c2 from xmlsimple.xml" << std::endl;
    };

    //now c2 should be equal to c1
    for(std::list<Bar>::const_iterator it = c2.mBars.begin(); it != c2.mBars.end(); it++)
      std::cerr << it->mBarNumber << std::endl;
    if (c1 == c2)
      std::cerr << "Foo is Foo!" << std::endl;
    return 0;
  } catch (const dba::Exception& pEx) {
    std::cout << "Error: " << pEx.what() << std::endl;
    return -1;
  };
};
