// File: transaction.cpp
// Purpose: Example of transation handling
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)
 
/**
@file transaction.cpp
This is an example showing how transactions works.
*/

#include "dba/dba.h"
#include <iostream>

/*
  Instances of this class can be stored in 
  Archive, because its STORE TABLE has relation name
  different from NULL.
*/
class Foo : public dba::Storeable {
    DECLARE_STORE_TABLE();
  public:
    int mIntVal;
};

//Store table for FooBase class
BEGIN_STORE_TABLE(Foo, dba::Storeable, "foo_table")
  BIND_INT(Foo::mIntVal,dba::Int,"intval")
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


//end of SQL schema 

/*
  Example of dba::Transaction usage
*/
void 
storeFoo(dba::SharedSQLArchive& pAr) {
  //transaction scope
  {
    //as long as transaction object exists
    //transaction to database is opened.
    dba::Transaction t(pAr.createTransaction());
    
    //streams should be obtained from transaction, not from archive
    //obtaining stream from archive when transaction exists 
    //will create new connection to database
    dba::SQLOStream ostream = t.getOStream();
    
    ostream.open();
    
    //write base object in database.
    //this will create one record in foo_table
    Foo b;
    b.mIntVal = 7;
    
    ostream.put(&b); //id of object is assigned here
    std::cout << "Foo was stored with id = " 
      << b.getId() << std::endl;
    
    //Streams obtained from transaction do not own connection
    //You don't must not call destroy them because every new created
    //stream will reuse connection that transaction owns.
    Foo loaded;  
    dba::SQLIStream istream = t.getIStream();
    if (!istream.get(&loaded)) {
      std::cerr << "Fatal error loading just stored object" 
        << std::endl;
      return;
    };
       
    //print loaded object 
    std::cout << "loaded Foo id=" << loaded.getId() << 
      ",intval: " << loaded.mIntVal << std::endl;
   
    //Mark transaction to be rolled back. If you don't do it then 
    //it will be commmited when t goes out of scope.
    t.rollback();
    
    //transaction is rolled back in transaction destructor
  };  
  
  //print contents of foo tables on screen
  std::auto_ptr<dba::DbResult> res(pAr.getIStream().sendQuery(
    "SELECT id, intval FROM foo_table"
  ));
  if (res->fetchRow()) {
    std::cerr << "Fatal error: there should be no objects "
      "after transaction is rolled back" << std::endl;
  } else {
    std::cout << "No objects in database, "
      "transaction was rolled back" << std::endl;
  };
};

int 
main (int argc, char** argv) {
  try {
    dba::SharedSQLArchive ar;
    
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
