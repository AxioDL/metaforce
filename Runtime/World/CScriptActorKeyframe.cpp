#include "CScriptActorKeyframe.hpp"

namespace urde
{
CScriptActorKeyframe::CScriptActorKeyframe(TUniqueId uid, const std::string& name, const CEntityInfo& info, s32 animId,
                                           bool b1, float f1, bool b2, u32 w2, bool active, float totalPlayback)
: CEntity(uid, info, active, name),
  x34_animationId(animId),
  x38_(f1),
  x3c_(totalPlayback),
  x40_(f1),
  x44_24_(b1),
  x44_25_(b2),
  x44_26_(w2)
{
}

void CScriptActorKeyframe::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr)
{

}

void CScriptActorKeyframe::Think(float dt, CStateManager& mgr)
{
    if (x44_25_)
        ;
}

void CScriptActorKeyframe::UpdateEntity(TUniqueId, CStateManager&)
{

}
}
