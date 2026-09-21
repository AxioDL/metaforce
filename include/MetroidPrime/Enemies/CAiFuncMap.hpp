#ifndef _CAIFUNCMAP
#define _CAIFUNCMAP

#include "types.h"
#include <rstl/pair.hpp>
#include <rstl/vector.hpp>

enum EStateMsg {
  kStateMsg_Activate = 0,
  kStateMsg_Update = 1,
  kStateMsg_Deactivate = 2,
};

class CAi;
class CStateManager;
typedef void (CAi::*CAiStateFunc)(CStateManager& mgr, EStateMsg msg, float arg);
typedef bool (CAi::*CAiTriggerFunc)(CStateManager& mgr, float arg);

class CAiFuncMap {
public:
  CAiFuncMap();

  const CAiStateFunc GetStateFunc(const char* state) const;
  const CAiTriggerFunc GetTriggerFunc(const char* state) const;

private:
  rstl::vector< rstl::pair< const char*, CAiStateFunc > > x0_states;
  rstl::vector< rstl::pair< const char*, CAiTriggerFunc > > x10_triggers;
};

#endif // _CAIFUNCMAP
