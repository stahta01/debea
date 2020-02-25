// File: quicstart.cpp
// Purpose: Example that shows basic debea features
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)
 
/**
@file quickstart.cpp
This is an example included in "quickstart" section of 
library documentation
*/

#include "dba/dba.h"
#include <iostream>

class Foo : public dba::Storeable {
    DECLARE_STORE_TABLE();
  public:
    int mIntVal;
    std::string mStrVal;  
};

//Class name, parent class name and relation name 
//(SQL table in case of SQL database)
BEGIN_STORE_TABLE(Foo, dba::Storeable, "foo_table")
  //we store mStrVal in varchar database field named 
  //"strval" using dba::String as conversion filter 
  BIND_STR(Foo::mStrVal,dba::String,"strval")
  //we store mIntVal in numeric database field named 
  //"intval" using dba::Int32 as conversion filter 
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
"  strval VARCHAR"
")");
//end of SQL schema 

int 
main (int argc, char** argv) {
  try {
    dba::SQLArchive ar;

    //set object responsible for assigning id numbers for serialized objects.
    //We use generic one that needs special SQL table debea_object_count. 
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

    Foo c1;
    c1.mIntVal = 12;
    c1.mStrVal = "test string";

    //write object in database
    dba::SQLOStream ostream = ar.getOStream();
    ostream.open();
    ostream.put(&c1); //id of object is assigned here
    std::cout << "Foo c1 was stored with id = " << c1.getId() << std::endl;

    //avoid creation new connection for istream
    //by releasing dba::DbConnection from ostream
    ostream.destroy();

    //read first object from database
    Foo c2;
    dba::SQLIStream istream = ar.getIStream();
    istream.open(c2);
    while (istream.getNext(&c2)) {
      std::cout << "readed Foo c2 with id: " << c2.getId() << std::endl;
    };

    //now c2 should be equal to c1
    if ((c1.mIntVal == c2.mIntVal)
      && (c1.mStrVal == c2.mStrVal)) 
      std::cerr << "Foo is Foo!" << std::endl;
    return 0;
  } catch (const dba::Exception& pEx) {
    std::cout << "Error: " << pEx.what() << std::endl;
    return -1;
  };
};
