// File: bindstb.cpp
// Purpose: Example of storing two store tables in one relation
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)
 
/**
@file bindstb.cpp
This is an example showing how you can store
members of two store tables in one relation
using BIND_STB macro.
*/

#include "dba/dba.h"
#include <iostream>

/*
  This class will not be really stored
  All members of store table will be
  transfered to Bar store table
*/
class Foo : public dba::Storeable {
    DECLARE_STORE_TABLE();
  public:
    int mIntVal;
};

//Relation name is ignored in BIND_STB
BEGIN_STORE_TABLE(Foo, dba::Storeable, "fake_relation_name")
  BIND_INT(Foo::mIntVal,dba::Int,"intval")
END_STORE_TABLE()

/*
  Bar agregates Foo  
*/
class Bar : public dba::Storeable {
    DECLARE_STORE_TABLE()
  public:
    int mBase;    
    Foo mFoo;
};

/*
  BIND_STB transfers all members from Foo to Bar
*/
BEGIN_STORE_TABLE(Bar, dba::Storeable, "bar_table")
  BIND_INT(Bar::mBase,dba::Int,"base")
  BIND_STB(Bar::mFoo)
END_STORE_TABLE()

//begin of SQL schema 
dba::SQL counter_create(
"CREATE TABLE debea_object_count ("
"  id INT"
")");


dba::SQL bar_create(
"CREATE TABLE bar_table ("
"  id INT PRIMARY KEY,"
"  base INT,"
"  intval INT"
")");
//end of SQL schema 

/*
  Example of FooBase and FooDerived serialization
*/
void 
storeStb(dba::SQLArchive& pAr) {
  dba::SQLOStream ostream = pAr.getOStream();
  ostream.open();
  
  //write base object in database.
  //this will create one record in bar_table
  Bar b;
  b.mBase = 5;
  b.mFoo.mIntVal = 6;
  
  //Store Bar class. This will create one record in bar_table
  //Note that b.mFoo will not change state from NEW to OK
  ostream.put(&b); //id of object is assigned here
  std::cout << "Bar was stored with id = " << b.getId() << std::endl;
  
  //destroy stream and release connection
  ostream.destroy();
  
  //print contents of bar table on screen
  int id, base, intval;
  std::cout << "Bar data:" << std::endl;
  std::auto_ptr<dba::DbResult> res(pAr.getIStream().sendQuery(
    dba::SQL("SELECT id, base, intval  FROM bar_table").into(id).into(base).into(intval)
  ));
  while(res->fetchRow()) {
    std::cout << 
    "id: "<< id << 
    " base: " << base << 
    " intval: " << intval << 
    std::endl;
  };
  std::cout << "=======================" << std::endl;
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
    //We use generic one that needs special 
    //SQL table debea_object_count. 
    ar.setIdFetcher(new dba::GenericFetcher());
    
    //sqlite3 backend creates new empty database
    //if there is no file
    unlink("foobasefile.sqt3");        
    
    //this will load database plugin and create new empty database
    ar.open("dbasqlite3-static", "dbname=foobasefile.sqt3");
    
    //create needed tables
    ar.getOStream().sendUpdate(counter_create);
    ar.getOStream().sendUpdate(bar_create);

    //first id
    ar.getOStream().sendUpdate(dba::SQL("INSERT INTO debea_object_count VALUES (:d)") << 1);

    storeStb(ar);
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
