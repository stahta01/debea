//
// C++ Implementation: globalfiltermapper
//
// Description: 
//
//
// Author: Lukasz Michalski <lm@zork.pl>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "dba/filtermapper.h"

#include <iostream>
#include <sstream>

//all filters here
#include "dba/int_filter.h"
#include "dba/string_filter.h"
#include "dba/double_filter.h"
#include "dba/bool_filter.h"
#include "dba/datetime_filter.h"

namespace dba {

FilterMapper::FilterMapper()
{
  initDefaultMappings();
}

FilterMapper::~FilterMapper()
{
  for(FilterMap::iterator it = mMap.begin(); it != mMap.end(); it++) {
    delete it->second;
  }
}

void 
FilterMapper::initDefaultMappings() {
  mapType(typeid(id), new dba::Int());
  mapType(typeid(int), new dba::Int());
  mapType(typeid(std::string), new dba::String());
  mapType(typeid(float), new dba::Float());
  mapType(typeid(double), new dba::Double());
  mapType(typeid(bool), new dba::Bool());
  mapType(typeid(tm), new dba::DateTime());
}

void 
FilterMapper::mapType(const std::type_info& pType, StoreableFilterBase* pFilter) {
  std::pair<FilterMap::iterator,bool> ret(mMap.insert(std::make_pair(pType.name(),pFilter)));
  if(!ret.second) {
    delete ret.first->second;
    ret.first->second = pFilter;
  }
  //std::cerr << "Added filter for [" << pType.name() << "] to list of default filters map" << std::endl;
}

dba::StoreableFilterBase& 
FilterMapper::findFilter(const char* pTypeName) const throw (FilterNotFoundException) {
  FilterMap::const_iterator it = mMap.find(pTypeName);
  if (it == mMap.end()) {
    std::stringstream error;
    error << "Default filter for type " << pTypeName << " not found ";
    throw FilterNotFoundException(error.str());
  }
  return *(it->second);
}

} //namepspace
