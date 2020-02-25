// File: inheritance.cpp
// Purpose: Example of storing members from different
//          classes in inheritance tree
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)
 
/**
@file inheritance.cpp
This is an example showing how inheritance works.
*/

#include "dba/dba.h"
#include <iostream>

/*
  Instances of this class can be stored in 
  Archive, because its STORE TABLE has relation name
  different from NULL.
*/
class FooBase : public dba::Storeable {
    DECLARE_STORE_TABLE();
  public:
    int mIntVal;
};

//Store table for FooBase class
BEGIN_STORE_TABLE(FooBase, dba::Storeable, "foo_table")
  BIND_INT(FooBase::mIntVal,dba::Int,"intval")
END_STORE_TABLE()

/*
  This class derives from FooBase. When you serialize instance of this
  class two records will be created in database: one in foo_table 
  and one in foo_derived relation. Both records will
  have the same id.
*/
class FooDerived : public FooBase {
    DECLARE_STORE_TABLE()
  public:
    int mDerivedVal;
};

/*
  You have to set parent table to FooBase to serialize
  members from that class.
*/
BEGIN_STORE_TABLE(FooDerived, FooBase, "foo_derived")
  BIND_INT(FooDerived::mDerivedVal,dba::Int,"derivedval")
END_STORE_TABLE()

/*
  Bar classes shows more compilcated scenario. BarBase
  does not have relation name, so in order to serialize
  instances of this class you have to pass
  relation name to stream open(). This way relation name for 
  final instaces of this class must be choosen
  at runtime.
*/
class BarBase : public dba::Storeable {
    DECLARE_STORE_TABLE()
  public:
    int mBase;    
};

BEGIN_STORE_TABLE(BarBase, dba::Storeable, NULL)
  BIND_INT(BarBase::mBase,dba::Int,"base")
END_STORE_TABLE()

/*
  BarDerived class inherits from BarBase, which does not 
  have relation name in its store table. This will cause 
  all of store table members from BarBase to be stored in 
  BarDerived relation.
*/
class BarDerived : public BarBase {
    DECLARE_STORE_TABLE()
  public:
    int mDerived;    
};

BEGIN_STORE_TABLE(BarDerived, BarBase, "bar_table")
  BIND_INT(BarDerived::mDerived,dba::Int,"derived")
END_STORE_TABLE()


//begin of SQL schema 
dba::SQL counter_create(
"CREATE TABLE debea_object_count ("
"  id INT"
")");

dba::SQL foobase_create(
"CREATE TABLE foo_table ("
"  id INT PRIMARY KEY,"
"  intval INT"
")");

dba::SQL fooderived_create(
"CREATE TABLE foo_derived ("
"  id INT PRIMARY KEY,"
"  derivedval INT"
")");

dba::SQL custom_create(
"CREATE TABLE custom_table ("
"  id INT PRIMARY KEY,"
"  base INT"
")");

dba::SQL bar_create(
"CREATE TABLE bar_table ("
"  id INT PRIMARY KEY,"
"  base INT,"
"  derived INT"
")");
//end of SQL schema 

/*
  Example of FooBase and FooDerived serialization
*/
void 
storeFoo(dba::SQLArchive& pAr) {
  dba::SQLOStream ostream = pAr.getOStream();
  ostream.open();
  
  //write base object in database.
  //this will create one record in foo_table
  FooBase b;
  b.mIntVal = 7;
  
  ostream.put(&b); //id of object is assigned here
  std::cout << "FooBase was stored with id = " 
    << b.getId() << std::endl;
  
  //write base object in database.
  //this will create two records: one in foo_table and 
  //one in foo_derived
  FooDerived d;
  d.mIntVal = 8;
  d.mDerivedVal = 9;
    
  ostream.put(&d); //id of object is assigned here
  std::cout << "FooDerived was stored with id = " 
    << d.getId() << std::endl;
    
  //destroy stream and release connection
  ostream.destroy();
  
  //print contents of foo tables on screen
  int id, intval;
  std::cout << "FooBase data:" << std::endl;
  std::auto_ptr<dba::DbResult> res(pAr.getIStream().sendQuery(
    dba::SQL("SELECT id, intval FROM foo_table").into(id).into(intval)
  ));
  while(res->fetchRow()) {
    std::cout << "id: "<< id << " intval: " 
      << intval << std::endl;
  };
  std::cout << "=======================" << std::endl;
  
  std::cout << "FooDerived data:" << std::endl;
  res.reset(pAr.getIStream().sendQuery(
    dba::SQL("SELECT id, derivedval FROM foo_derived").into(id).into(intval)
  ));
  while(res->fetchRow()) {
    std::cout << "id: "<< id << " derivedval: " 
      << intval << std::endl;
  };
  std::cout << "=======================" << std::endl;
};

/*
  Example of BarBase and BarDerived serialization
*/
void
storeBar(dba::SQLArchive& pAr) {
  dba::SQLOStream ostream = pAr.getOStream();
  
  //write base object in database.
  //this will create one record in custom_table
  ostream.open("custom_table");
  BarBase b;
  b.mBase = 7;
  
  ostream.put(&b); //id of object is assigned here
  std::cout << "BarBase was stored with id = " 
    << b.getId() << std::endl;
  
  //write BarDerived object in database.
  //this will create two records: one in foo_table 
  //and one in foo_derived
  BarDerived d;
  d.mBase = 8;
  d.mDerived = 9;
    
  //we have to reopen stream withh NULL
  //root table because stream still remembers
  //that it should store most derived class
  //members in "custom_table"
  ostream.open();
  ostream.put(&d); //id of object is assigned here
  std::cout << "BarDerived was stored with id = " 
    << d.getId() << std::endl;
    
  //destroy stream and release connection
  ostream.destroy();
  
  //print contents of foo tables on screen
  std::cout << "Custom table data:" << std::endl;
  std::auto_ptr<dba::DbResult> res(pAr.getIStream().sendQuery(
    "SELECT id, base FROM custom_table"
  ));
  while(res->fetchRow()) {
    std::cout << "id: "<< res->getInt(0) << " base: " 
      << res->getInt(1) << std::endl;
  };
  std::cout << "=======================" << std::endl;
  
  std::cout << "Bar table data:" << std::endl;
  res.reset(pAr.getIStream().sendQuery(
    "SELECT id, base, derived FROM bar_table"
  ));
  while(res->fetchRow()) {
    std::cout << 
    "id: "<< res->getInt(0) << 
    " base: " << res->getInt(1) << 
    " derived: " << res->getInt(2) 
    << std::endl;
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
    //We use generic one that needs special SQL table 
    //debea_object_count. 
    ar.setIdFetcher(new dba::GenericFetcher());
    
    //sqlite3 backend creates new empty database
    //if there is no file
    unlink("foobasefile.sqt3");        
    
    //this will load database plugin and create new empty database
    ar.open("dbasqlite3-static", "dbname=foobasefile.sqt3");
    
    //create needed tables
    ar.getOStream().sendUpdate(counter_create);
    ar.getOStream().sendUpdate(foobase_create);
    ar.getOStream().sendUpdate(fooderived_create);
    ar.getOStream().sendUpdate(custom_create);
    ar.getOStream().sendUpdate(bar_create);

    //first id
    ar.getOStream().sendUpdate(dba::SQL("INSERT INTO debea_object_count VALUES (:d)") << 1);

    storeFoo(ar);
    storeBar(ar);
    
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
