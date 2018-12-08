#include "CScriptRandomRelay.hpp"
#include "CStateManager.hpp"
#include "TCastTo.hpp"

namespace urde {
CScriptRandomRelay::CScriptRandomRelay(TUniqueId uid, std::string_view name, const CEntityInfo& info, s32 connCount,
                                       s32 variance, bool clamp, bool active)
: CEntity(uid, info, active, name)
, x34_connectionCount((clamp && connCount > 100) ? 100 : connCount)
, x38_variance(variance)
, x3c_clamp(clamp) {}

void CScriptRandomRelay::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptRandomRelay::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
  if (msg == EScriptObjectMessage::SetToZero) {
    if (!x30_24_active)
      return;
    SendLocalScriptMsgs(EScriptObjectState::Zero, stateMgr);
  }
}

void CScriptRandomRelay::SendLocalScriptMsgs(EScriptObjectState state, CStateManager& stateMgr) {
  if (state != EScriptObjectState::Zero) {
    SendScriptMsgs(state, stateMgr, EScriptObjectMessage::None);
    return;
  }

#if 0
    std::vector<std::pair<CEntity*, EScriptObjectMessage>> objs;
    objs.reserve(10);
    for (const SConnection& conn : x20_conns)
    {
        auto list = stateMgr.GetIdListForScript(conn.x8_objId);
        auto it = list.first;
        for (; it != list.second; ++it)
        {
            CEntity* ent = stateMgr.ObjectById(it->second);
            if (ent && ent->GetActive())
                objs.emplace_back(ent, conn.x4_msg);
        }
    }

    u32 r0 = x34_connectionCount;
    if (x3c_clamp)
        r0 = u32(0.5 + (float(x34_connectionCount) / 100.f));

    float fVariance = float(x38_variance) * (2.f * stateMgr.GetActiveRandom()->Float());

#endif
}
} // namespace urde
