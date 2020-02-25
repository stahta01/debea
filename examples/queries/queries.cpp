// File: queries.cpp
// Purpose: Example of sending parameterized SQL queries using SQLArchive
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2009
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)
 
/**
@file queries.cpp
This is an example showing how to send SQL statements using dba::SQLArchive.
*/

#include "dba/dba.h"
#include <iostream>
#include <sstream>

/*
  Custom class for conversion example
*/
class Keyboard {
  public:
    int mKeys;
};

/*
  Conversion filter for Keyboard class
*/
class KeyboardFilter : public dba::StoreableFilter<Keyboard> {
  public:
    KeyboardFilter() {};
    KeyboardFilter(Keyboard& pData) : dba::StoreableFilter<Keyboard>(pData) {};
    //used by SQLArchive when SQL SELECT is constructed
    virtual std::string toString(const dba::ConvSpec& pSpec) const throw (dba::StoreableFilterException) { 
      std::stringstream d;
      d << mMember->mKeys;
      return d.str(); 
    }
    virtual bool isNull() const { return false; }
    //used by SQLArchive when reading data from SQL integer fields
    virtual void fromInt(const dba::ConvSpec& pSpec, int pData) throw (dba::StoreableFilterException) {
      mMember->mKeys = pData;
    };
    //used by SQLArchive when reading data from SQL floating point number fields
    virtual void fromDouble(const dba::ConvSpec& pSpec, double pData) throw (dba::StoreableFilterException) {
      mMember->mKeys = static_cast<int>(pData);
    };
    //used by SQLArchive when reading data from SQL char,varchar fields
    virtual void fromString(const dba::ConvSpec& pSpec, const std::string& pData) throw (dba::StoreableFilterException) {
      std::istringstream ostr(pData);
      if (!ostr >> mMember->mKeys)
        throw dba::StoreableFilterException("Failed to convert string to int");
    };
    //we override default store type (which is Database::STRING) to tell debea driver to fetch
    //data as integers to aviod multiple coversions 
    virtual dba::Database::StoreType getPrefferedStoreType() const { return dba::Database::INTEGER; }
    //used always when reading null field of any type
    virtual void fromNull() throw (dba::StoreableFilterException) {
      mMember->mKeys = 0;
    };
    virtual ~KeyboardFilter() {};
};


dba::SQL table(
"CREATE TABLE data ("
"  intval INT,"
"  strval VARCHAR"
")");

/*
  Simple queries with in and out parameters
*/
void 
simple_queries(dba::SQLArchive& pAr) {

  //insert some data into table
  pAr.getOStream().sendUpdate(
    dba::SQL("INSERT INTO data(intval,strval) VALUES(:d,:s)") << 1 << "test"
  );

  //vars to put data in
  std::string strval;
  int intval;

  //and read inserted row
  std::auto_ptr<dba::DbResult> res(
    pAr.getIStream().sendQuery(dba::SQL("SELECT strval,intval FROM data").into(strval).into(intval))
  );

  while(res->fetchRow()) {
    std::cout << "intval: "<< intval << " strval: " 
      << strval << std::endl;
  };

  //queries can be constructed from parts too
  dba::SQL sel1("SELECT intval FROM data");
  sel1.into(intval);
  dba::SQL sel2(" WHERE strval LIKE '%:s%'");
  sel2 << "te";

  res.reset(pAr.getIStream().sendQuery(sel1 + sel2));
  while(res->fetchRow()) {
    std::cout << "intval: "<< intval << std::endl;
  };
};

/*
  How to pass custom class as query parameter
*/
void
custom_conversions(dba::SQLArchive& pAr) {
  Keyboard k;
  k.mKeys = 102;
  //we can use conversion filter to store custom type as tuple (field) in table row:
  pAr.getOStream().sendUpdate(dba::SQL("INSERT INTO data(intval) VALUES(:d)") << new KeyboardFilter(k));

  //we can set default filter for given C++ type this way:
  pAr.mapType(typeid(Keyboard), new KeyboardFilter());

  //and then when we pass custom class as query parameter set conversion
  //filter will be used.
  k.mKeys = 103;
  pAr.getOStream().sendUpdate(dba::SQL("INSERT INTO data(intval) VALUES(:d)") << k);

  //SQLUtils has some helpers to construct sets for IN keyword
  std::vector<int> intvals;
  intvals.push_back(102);
  intvals.push_back(103);
  dba::SQL sel1(dba::SQL("SELECT intval FROM data").into(k));
  dba::SQL sel2(dba::SQL(" WHERE intval IN ") + dba::SQLUtils::createIN(intvals));

  std::auto_ptr<dba::DbResult> res(pAr.getIStream().sendQuery(sel1 + sel2));
  while(res->fetchRow()) {
    std::cout << "got keyboard with  " << k.mKeys << " keys" << std::endl;
  };
};

/*
  Standard main function for all examples
*/
int 
main (int argc, char** argv) {
  try {
    dba::SQLArchive ar;

    //sqlite3 backend creates new empty database
    //if there is no file
    unlink("foobasefile.sqt3");

    //this will load database plugin and create new empty database
    ar.open("dbasqlite3-static", "dbname=foobasefile.sqt3");

    //create needed tables
    ar.getOStream().sendUpdate(table);

    simple_queries(ar);
    custom_conversions(ar);
    
    return 0;
  } catch (const dba::SQLException& pEx) {
    std::cout << "SQL Error: " << pEx.what() << std::endl;
    std::cout << "While executing: " << std::endl 
      << pEx.getQuery() << std::endl;
  } catch (const dba::Exception& pEx) {
    std::cout << "Error: " << pEx.what() << std::endl;
    return -1;
  };
};
