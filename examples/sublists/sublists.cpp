// File: quicstart.cpp
// Purpose: Example that shows how to store composited objects
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)
 
/**
@file inheritance.cpp
This is an example showing how serialization of sublists works.
*/

#include "dba/dba.h"
#include <iostream>

/*
  Part of Foo class
*/
class Bar : public dba::Storeable {
    DECLARE_STORE_TABLE()
  public:
    int mBarValue;    
};

BEGIN_STORE_TABLE(Bar, dba::Storeable, "bar_table")
  BIND_INT(Bar::mBarValue,dba::Int,"value")
END_STORE_TABLE()

/*
  Part of Foo class - without relation name
*/
class CuBar : public dba::Storeable {
    DECLARE_STORE_TABLE()
  public:
    int mBarValue;    
};

BEGIN_STORE_TABLE(CuBar, dba::Storeable, NULL)
  BIND_INT(CuBar::mBarValue,dba::Int,"value")
END_STORE_TABLE()

/*
  Serializable class with std::lists that holds serializable objects
*/
class Foo : public dba::Storeable {
    DECLARE_STORE_TABLE();
  public:
    int mIntVal;
    CuBar mSpecialCuBar;
    std::list<Bar> mBars;
    std::list<CuBar> mCuBars;    
};

/*
  Store table for Foo class contains entries for std::list
  
  BIND_COL name of field is a name of foreign key field in SQL table
  
  BIND_CLA needs two additional parameters: relation name 
  and collection id.
  We have to set relation name because CuBar does not 
  have one in its store table
  We have to set collection id because we load CuBar 
  instances to two 'containers': std::list (1-n relation) and 
  CuBar type itself (1-1 relation)
  
  See implementation of dba::stdList filter for hints how
  to implement your own filters for different container types
*/
BEGIN_STORE_TABLE(Foo, dba::Storeable, "foo_table")
  BIND_INT(Foo::mIntVal,dba::Int,"intval")
  BIND_COL(Foo::mBars, dba::stdList<Bar>, "fk_foo")
  BIND_CLA(Foo::mCuBars, dba::stdList<CuBar>, "fk_foo",1,"cubar_table")
  BIND_CLA(Foo::mSpecialCuBar, dba::Single, "fk_foo",2,"cubar_table")
END_STORE_TABLE()


//begin of SQL schema 
dba::SQL counter_create(
"CREATE TABLE debea_object_count ("
"  id INT"
")");

dba::SQL foo_create(
"CREATE TABLE foo_table ("
"  id INT PRIMARY KEY,"
"  intval INT"
")");

dba::SQL bar_create(
"CREATE TABLE bar_table ("
"  id INT PRIMARY KEY,"
"  fk_foo INT NOT NULL,"
"  value INT"
")");

//note that we need to add special 
//dba_coll_id column for BIND_CLA
dba::SQL cubar_create(
"CREATE TABLE cubar_table ("
"  id INT PRIMARY KEY,"
"  fk_foo INT NOT NULL,"
"  dba_coll_id INT NOT NULL,"
"  value INT"
")");
//end of SQL schema 

/*
  Example of FooBase and FooDerived serialization
*/
void 
storeFoo(dba::SQLArchive& pAr) {
  dba::SQLOStream ostream = pAr.getOStream();
  ostream.open();
  
  //construct one foo instance with some children
  CuBar special;
  special.mBarValue = 777;
  
  Foo f;
  f.mIntVal = 7;
  f.mSpecialCuBar = special;

  //add 10 Bars and 5 Cubars to f
  for (int i = 10; i < 20; i++) {
    Bar b;
    b.mBarValue = i;
    f.mBars.push_back(b);
    if (i < 15) {
      CuBar c;
      c.mBarValue = i+100;
      f.mCuBars.push_back(c);
    };
  };
  
  //this should generate 16 SQL inserts (1 Foo + 10 Bars + 5 CuBars)
  ostream.put(&f); //id of object is assigned here
  std::cout << "Foo was stored with id = " << f.getId() << std::endl;
  
  //destroy stream and release connection
  ostream.destroy();
  
  //Now load everything back
  dba::SQLIStream istream = pAr.getIStream();
  Foo loaded;
  //tell stream which Foo object to load. Bars and CuBars will 
  //be retrieved using foreign key fields automatically.
  istream.setWhereId(f.getId());
  //this should sent 3 SQL SELECT's to database 
  //(1 Foo + 1 for Bars + 1 for CuBars)
  istream.get(&loaded);
  
  
  //print contents of loaded object
  std::cout << "Foo 'loaded' object id " << loaded.getId() << 
    ",mIntVal=" << loaded.mIntVal << std::endl;
  std::cout 
  << "Foo.mSpecialCuBar id " << loaded.mSpecialCuBar.getId() 
  << ",mBarVal=" << loaded.mSpecialCuBar.mBarValue 
  << std::endl;
  for(std::list<Bar>::const_iterator it = loaded.mBars.begin(); 
    it != loaded.mBars.end(); it++) 
  {
    std::cout << "Bar id=" << it->getId() << 
      ",mBarVal=" << it->mBarValue << std::endl;
  };
  for(std::list<CuBar>::const_iterator it = loaded.mCuBars.begin(); 
      it != loaded.mCuBars.end(); it++) 
  {
    std::cout << "cuBar id=" << it->getId() << 
      ",mBarVal=" << it->mBarValue << std::endl;
  };
  std::cout << "=======================" << std::endl;
};

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
    ar.getOStream().sendUpdate(foo_create);
    ar.getOStream().sendUpdate(bar_create);
    ar.getOStream().sendUpdate(cubar_create);

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
