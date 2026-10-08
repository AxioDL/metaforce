#ifndef _CSCRIPTDEBUGCAMERAWAYPOINT
#define _CSCRIPTDEBUGCAMERAWAYPOINT

#include "types.h"

#include "MetroidPrime/CActor.hpp"

class CScriptDebugCameraWaypoint : public CActor {
public:
  CScriptDebugCameraWaypoint(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                             const CTransform4f& xf, uint value);
#if VERSION < VERSION_GM8P_00
  ~CScriptDebugCameraWaypoint() override;
#endif

  DECLARE_TYPES_MATCH_OR_ACCEPT;

private:
  uint mValue;
};

#endif // _CSCRIPTDEBUGCAMERAWAYPOINT
