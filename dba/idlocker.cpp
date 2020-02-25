// File: idlocker.cpp
// Purpose: Support for cloning objects
// Author: Lukasz Michalski <lm at zork.pl>, Copyright 2005
// License: See COPYING file that comes with this distribution
//
// This file is a part of debea library (http://debea.net)

#include "dba/idlocker.h"
#include <iostream>

namespace dba {

IdLocker::mode dba_idlocker_lock_var = IdLocker::UNLOCKED;

bool 
IdLocker::isLocked() {
  return dba_idlocker_lock_var != IdLocker::UNLOCKED;
};

bool 
IdLocker::isUnlocked() {
  return !isLocked();
};

IdLocker::IdLocker(mode pMode)
{
  dba_idlocker_lock_var = pMode;
}


IdLocker::~IdLocker()
{
  dba_idlocker_lock_var = UNLOCKED;
}


}
