//
// C++ Interface: chandle
//
// Description: 
//
//
// Author: Lukasz Michalski <lm@zork.pl>, (C) 2007
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef DBACHANDLE_H
#define DBACHANDLE_H

namespace dba {

/**
class for managing c resources in exception safe way
*/
template <typename P, typename D>
class CHandle {
  public:
    CHandle(P pPtr = NULL) : mPtr(pPtr) {};
    //conversion to C pointer
    operator P () {
      return mPtr;
    };
    operator P () const {
      return mPtr;
    };
    P ptr() {
      return mPtr;
    };
    void reset(P pPtr = NULL) {
      if (mPtr != NULL) {
        D deallocator;
        deallocator.dealloc(mPtr);
      };
      mPtr = pPtr;
    };
    P release() {
      P ret = mPtr;
      mPtr = NULL;
      return ret;
    };
    ~CHandle() {
      reset();
    };
  private:
    P mPtr;

    //not implemented - there is no reference counting
    CHandle& operator=(const CHandle& pCol);
    bool operator==(const CHandle& pCol);
};

} //namesapce

#endif
