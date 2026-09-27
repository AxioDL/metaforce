#ifndef _CVALIDENTITYPREDICATE
#define _CVALIDENTITYPREDICATE

#include "MetroidPrime/TGameTypes.hpp"

class CStateManager;

class CValidEntityPredicate {
public:
  virtual ~CValidEntityPredicate() {}
  virtual bool operator()(const CStateManager& mgr, TUniqueId uid) const = 0;
};

#endif // _CVALIDENTITYPREDICATE
