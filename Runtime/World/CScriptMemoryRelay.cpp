#include "CScriptMemoryRelay.hpp"

namespace urde
{

CScriptMemoryRelay::CScriptMemoryRelay(TUniqueId uid, const std::string& name, const CEntityInfo& info, bool b1, bool b2, bool b3)
    : CEntity(uid, info, true, name),
      x34_24_(b1),
      x34_25_(b2),
      x34_26_(b3)
{
}

}
