// File: csv_manual.cpp
// Purpose: Example of handling csv files with manual
//          column mapping
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2007
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)
 
/**
@file csv_manual.cpp
*/

#include <sstream> //for ostringstream
#include <iostream>
#include "dba/dba.h"
#include "dba/csv.h"

/*
  Instances of this class can be stored in 
  Archive, because its STORE TABLE has relation name
  different from NULL.
*/
class Person : public dba::Storeable {
    DECLARE_STORE_TABLE();
  public:
    std::string mName;
    std::string mSurname;
    int mAge;
};

//Store table for Person class
BEGIN_STORE_TABLE(Person, dba::Storeable, "person_table")
  BIND_STR(Person::mName,dba::String,"name")
  BIND_STR(Person::mSurname,dba::String,"surname")
  BIND_INT(Person::mAge,dba::Int,"age")
END_STORE_TABLE()

int 
main (int argc, char** argv) {
  try {
    //create CSVArchive
    dba::CSVArchive ar;
    
    //create new file - try to unlink old one just in case
    unlink("testfile.csv");        
    
    //we not store column names in first row
    ar.hasColumnNames(false);
    ar.open("testfile.csv");
    
    //store members in order: name, surname, country
    //not that relation name *must* match one in Person store table
    ar.addMapping(0,"person_table","name");
    ar.addMapping(1,"person_table","surname");
    ar.addMapping(2,"person_table","age");
    
    dba::CSVOStream ostream = ar.getOStream();
    
    ostream.open();    
    for(int i = 0; i < 10; i++) {
      //you cannot update record in csv file (yet)
      //so every dba::Storeable derived instance 
      //passed to CSVOStream::put() have to be in NEW state
      Person p;
      std::ostringstream name;
      name << "Name_" << i;
      p.mName = name.str();
      
      std::ostringstream surname;
      surname << "Surname_" << i;
      p.mSurname = surname.str();
    
      p.mAge = i + 15;
    
      ostream.put(&p);
    };
    
    //in case of CSVArchive you have to use only one stream in one time
    //destroy output stream before creating input stream for reading
    ostream.destroy();
    
    //CSVIStream does not support searching or retrieving 
    //data from middle of file. You have to read record by
    //record from beginning to end.
    dba::CSVIStream istream = ar.getIStream();
    Person p;    
    istream.open(p);
    while(istream.getNext(&p)) {
      std::cerr <<
      "Loaded Person, name: " << p.mName <<
      ", surname: " << p.mSurname <<
      ", age: " << p.mAge 
      << std::endl;
    };
    
    return 0;
  } catch (const dba::CSVParseException& pEx) {
    std::cout << "Parse error: " << pEx.what() << std::endl;
  } catch (const dba::Exception& pEx) {
    std::cout << "Error: " << pEx.what() << std::endl;
    return -1;
  };
};
