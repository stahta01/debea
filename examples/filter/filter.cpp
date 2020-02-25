// File: filter.cpp
// Purpose: Example of creation custom conversion filter
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)
/**
@file filter.cpp
*/

#include "dba/dba.h"
#include <iostream>
#include <algorithm>

/*
  This class will be serialized as member of Foo
  in one relation field
*/
class Address {
  public:
    std::string mCity;
    std::string mStreet;
    std::string mPostcode;
    std::string mCountry;
    std::string mRegion;
};

/*
  Minimal filter must know how to convert C++ type
  to string and from one of database type.
  It also and had to know when C++ should be 
  converted to null.
  
  We will use BIND_STR macro for serialization 
  so we have to implement fromString
*/
class AddressFilter : public dba::StoreableFilter<Address> {
  public:
    AddressFilter(Address& pAddress) 
      : dba::StoreableFilter<Address>(pAddress) 
    {}
    
    //create string from address
    virtual std::string toString(const dba::ConvSpec& pSpec) const 
      throw (dba::StoreableFilterException) 
    {
      std::string ret;
      ret += mMember->mStreet + ",";
      ret += mMember->mPostcode + " ";
      ret += mMember->mCity;
      ret += " (" + mMember->mRegion + "),";
      ret += mMember->mCountry;
      return ret;
    };
    
    //parse string readed from database    
    virtual void fromString(const dba::ConvSpec& pSpec, 
      const std::string& pString) throw (dba::StoreableFilterException)
    {
      std::string::const_iterator e = 
        std::find(pString.begin(), pString.end(), (','));
      if (e == pString.end())
        throw dba::StoreableFilterException("Street part not found");
      mMember->mStreet = std::string(pString.begin(),e);
      std::string::const_iterator b = ++e;
      e = std::find(b, pString.end(), (' '));
      if (e == pString.end())
        throw dba::StoreableFilterException("Postcode part not found");
      mMember->mPostcode = std::string(b,e);
    
      b = ++e;
      e = std::find(b, pString.end(), ('('));
      if (e == pString.end())
        throw dba::StoreableFilterException("City part not found");
      mMember->mCity = std::string(b,--e);
      
      b = std::find(b, pString.end(), ('('));
      e = std::find(b, pString.end(), (')'));
      if (b == pString.end() || e == pString.end())
        throw dba::StoreableFilterException("Region part not found");
      mMember->mRegion = std::string(++b,e);
    
      b = std::find(b, pString.end(), (','));
      b++;
      e = pString.end();
      if (b == e)
        throw dba::StoreableFilterException("Country not found");
      mMember->mCountry = std::string(b,e);  
    
    };
    
    virtual bool isNull() const {
      //lets assume that we should store NULL in relation 
      //when there is no street.
      return mMember->mStreet.empty();    
    };
    
    virtual void fromNull() throw (dba::StoreableFilterException) {
      //just clear all members
      *mMember = Address();
    };
};

/*
  Class with Adress to serialize
*/
class Foo : public dba::Storeable {
    DECLARE_STORE_TABLE()
  public:
    Address mAddress;
};

/*
  We use AddressFilter to serialize and deserialize Address intstance
*/
BEGIN_STORE_TABLE(Foo, dba::Storeable, "foo_table")
  BIND_STR(Foo::mAddress,AddressFilter,"address")
END_STORE_TABLE()

//begin of SQL schema 
dba::SQL counter_create(
"CREATE TABLE debea_object_count ("
"  id INT"
")");

dba::SQL foo_create(
"CREATE TABLE foo_table ("
"  id INT PRIMARY KEY,"
"  address VARCHAR"
")");
//end of SQL schema 

/*
  Example of custom filter usage
*/
void 
storeFoo(dba::SQLArchive& pAr) {
  dba::SQLOStream ostream = pAr.getOStream();
  ostream.open();
  
  //write base object in database.
  //this will create one record in foo_table
  Foo b1;
  b1.mAddress.mStreet = "Boulder 18 St.";
  b1.mAddress.mPostcode = "456-34";
  b1.mAddress.mCity = "New York City";
  b1.mAddress.mRegion = "New York";
  b1.mAddress.mCountry = "United States";
  
  
  ostream.put(&b1); //id of object is assigned here
  std::cout << "FooBase was stored with id = " 
    << b1.getId() << std::endl;
  
  //destroy stream and release connection
  ostream.destroy();
  
  //load just stored object and print address on console
  dba::SQLIStream istream = pAr.getIStream();
  Foo b2;
  istream.open(b2);
  istream.getNext(&b2);
  
  std::cout << "Loaded object id:" << b2.getId() 
     << ", with Address: "<< std::endl << std::endl;
  std::cout << b2.mAddress.mStreet << std::endl;
  std::cout << b2.mAddress.mPostcode << " " << b2.mAddress.mCity 
     << " (" << b2.mAddress.mRegion << ")" << std::endl;
  std::cout << b2.mAddress.mCountry << std::endl;

  //load data using SQL query and convert it using our filter.
  //Note that you can also call SQLArchive::mapType() to map Address class with AddressFilter pernamently
  Address address;
  std::auto_ptr<dba::DbResult> res(istream.sendQuery(dba::SQL("SELECT address FROM foo_table WHERE id=:d").into(new AddressFilter(address)) << b2.getId()));
  res->fetchRow();

  std::cout << "=================================" << std::endl;
  std::cout << "Address loaded by direct SQL query:" << std::endl;
  std::cout << address.mStreet << std::endl;
  std::cout << address.mPostcode << " " << address.mCity 
     << " (" << address.mRegion << ")" << std::endl;
  std::cout << address.mCountry << std::endl;

};

/*
  Standard main function for all examples
*/
int 
main (int argc, char** argv) {
  try {
    dba::SQLArchive ar;
    
    //set object responsible for assigning id numbers 
    //for serialized objects.
    //We use generic one that needs special SQL table 
    //debea_object_count. 
    ar.setIdFetcher(new dba::GenericFetcher());
    
    //sqlite3 backend creates new empty database
    //if there is no file
    unlink("foobasefile.sqt3");        
    
    //this will load database plugin and create new empty database
    ar.open("dbasqlite3-static", "dbname=foobasefile.sqt3");
    
    //create needed tables sending simple SQL
    ar.getOStream().sendUpdate(counter_create);
    ar.getOStream().sendUpdate(foo_create);

    //first id
    ar.getOStream().sendUpdate(dba::SQL("INSERT INTO debea_object_count VALUES (:d)") << 1);
    
    storeFoo(ar);
    
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
