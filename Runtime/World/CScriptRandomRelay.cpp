#include "CScriptRandomRelay.hpp"

namespace urde
{
CScriptRandomRelay::CScriptRandomRelay(TUniqueId uid, const std::string& name, const CEntityInfo& info, s32 connCount, s32 variance,
                                       bool clamp, bool active)
    : CEntity(uid, info, active, name),
      x34_connectionCount((clamp && connCount > 100) ? 100 : connCount),
      x38_variance(variance),
      x3c_clamp(clamp)
{
}
}
