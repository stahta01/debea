// File: bind.cpp
// Purpose: Example of binding custom variables
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)
 
/**
@file bind.cpp
This is an example showing how to bind custom variable 
to object stream.
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
"  intval INT,"
"  binded_str VARCHAR"
")");
//end of SQL schema 

/*
  Example of writing and reading binded variable
*/
void 
bindedVars(dba::SQLArchive& pAr) {
  dba::SQLOStream ostream = pAr.getOStream();
  
  //write base object in database.
  //this will create one record in foo_table
  Foo b;
  b.mIntVal = 7;
  
  std::string var("Binded variable stored with Foo");
  //bind variable to relation field using String filter for 
  //data conversion. Last argument tells stream that data should 
  //be fetched from database text fields. 
  //For SQL database this is more important 
  //for reading than writing
  
  //table name must match one of relation names of written object. 
  //If is it not matched, then data will not be written.
  ostream.bind("foo_table","binded_str",
    new dba::String(var),dba::Database::STRING);
  
  ostream.open();
  ostream.put(&b); //id of object is assigned here
  std::cout << "FooBase was stored with id = " 
    << b.getId() << std::endl;
  
  ostream.destroy();  

 
  //print contents of foo tables on screen
  int id, intval;
  std::string binded_str;
  std::cout << "FooBase data:" << std::endl;
  std::auto_ptr<dba::DbResult> res(pAr.getIStream().sendQuery(
    dba::SQL("SELECT id, intval, binded_str FROM foo_table").into(id).into(intval).into(binded_str)
  ));

  while(res->fetchRow()) {
    std::cout << 
    "id: "<< id << 
    " intval: " << intval <<
    " binded_str: " << binded_str <<
    std::endl;
  };
  std::cout << "=======================" << std::endl;
  
  //now load stored data back
  dba::SQLIStream istream = pAr.getIStream();
  Foo readed;
  std::string str_readed;
  
  //bind must be always done before open
  istream.bind("foo_table","binded_str",
    new dba::String(str_readed),dba::Database::STRING);
  istream.open(readed);
  istream.getNext(&readed);
  
  std::cout << "Readed binded var: '" << str_readed 
    << "'" << std::endl;
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
    
    //create needed tables sending simple SQL
    ar.getOStream().sendUpdate(counter_create);
    ar.getOStream().sendUpdate(foo_create);

    //first id
    ar.getOStream().sendUpdate(dba::SQL("INSERT INTO debea_object_count VALUES (:d)") << 1);
    
    bindedVars(ar);
    
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
