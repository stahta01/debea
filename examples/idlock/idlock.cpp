// File: idlock.cpp
// Purpose: Example of cloning storeable object in database
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)
 
/**
@file idlock.cpp
*/

#include "dba/dba.h"
#include <iostream>

/*
  Instances of that class can be serialized
*/
class Foo : public dba::Storeable {
    DECLARE_STORE_TABLE();
  public:
    int mIntVal;
};

//Store table for Foo class
BEGIN_STORE_TABLE(Foo, dba::Storeable, "foo_table")
  BIND_INT(Foo::mIntVal,dba::Int,"intval")
END_STORE_TABLE()

//begin of SQL schema 
dba::SQL counter_create =
"CREATE TABLE debea_object_count ("
"  id INT"
")";

dba::SQL foo_create(
"CREATE TABLE foo_table ("
"  id INT PRIMARY KEY,"
"  intval INT"
")");
//end of SQL schema 

/*
  Example of object clonning
*/
void 
cloneFoo(dba::SQLArchive& pAr) {
  dba::SQLOStream ostream = pAr.getOStream();
  ostream.open();
  
  //write base object in database.
  //this will create one record in foo_table
  Foo one;
  one.mIntVal = 1;
  
  ostream.put(&one); //id of object is assigned here
  std::cout << "Foo was stored with id = " << one.getId() << std::endl;
  
  {
    //idlocker prevents from assiging dba::Storeable state
    //by copy constructor and assigment operator
    //As long as IdLocker instance is present all 
    //dba::Storeable derived objects will not change their 
    //state from dba::Storeable::NEW upon construction or assigment.
    dba::IdLocker l;
    Foo one_clone(one);
    ostream.put(&one_clone); //id of object is assigned here
    std::cout << "Foo was cloned with id = " 
      << one_clone.getId() << std::endl;
  };
  
  //destroy stream and release connection
  ostream.destroy();
  
  //print contents of foo tables on screen
  int id, intval;
  std::cout << "Foo data:" << std::endl;
  std::auto_ptr<dba::DbResult> res(pAr.getIStream().sendQuery(
    dba::SQL("SELECT id, intval FROM foo_table").into(id).into(intval)
  ));
  while(res->fetchRow()) {
    std::cout << "id: "<< id << " intval: " 
      << intval << std::endl;
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
    
    //create needed tables sending simple SQL
    ar.getOStream().sendUpdate(counter_create);
    ar.getOStream().sendUpdate(foo_create);

    //first id
    ar.getOStream().sendUpdate(dba::SQL("INSERT INTO debea_object_count VALUES (:d)") << 1);
    
    cloneFoo(ar);
    
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
