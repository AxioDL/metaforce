#include "Runtime/World/CScriptRandomRelay.hpp"

#include "Runtime/CStateManager.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CScriptRandomRelay::CScriptRandomRelay(TUniqueId uid, std::string_view name, const CEntityInfo& info, s32 sendSetSize,
                                       s32 sendSetVariance, bool percentSize, bool active)
: CEntity(uid, info, active, name)
, x34_sendSetSize((percentSize && sendSetSize > 100) ? 100 : sendSetSize)
, x38_sendSetVariance(sendSetVariance)
, x3c_percentSize(percentSize) {}

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

  std::vector<std::pair<CEntity*, EScriptObjectMessage>> objs;
  objs.reserve(10);
  for (const SConnection& conn : x20_conns) {
    auto list = stateMgr.GetIdListForScript(conn.x8_objId);
    for (auto it = list.first; it != list.second; ++it) {
      CEntity* ent = stateMgr.ObjectById(it->second);
      if (ent && ent->GetActive())
        objs.emplace_back(ent, conn.x4_msg);
    }
  }

  s32 targetSetSize = x34_sendSetSize;
  if (x3c_percentSize)
    targetSetSize = s32(0.5f + (float(x34_sendSetSize * objs.size()) / 100.f));
  targetSetSize += s32(float(x38_sendSetVariance) * 2.f * stateMgr.GetActiveRandom()->Float()) - x38_sendSetVariance;
  targetSetSize = zeus::clamp(0, targetSetSize, 64);

  while (objs.size() > targetSetSize) {
    s32 randomRemoveIdx = s32(stateMgr.GetActiveRandom()->Float() * float(objs.size()) * 0.99f);
    auto it = objs.begin();
    for (s32 i = 0; i < randomRemoveIdx; ++i) {
      ++it;
      if (it == objs.end())
        break;
    }
    if (it != objs.end())
      objs.erase(it);
  }

  for (const auto& o : objs)
    stateMgr.SendScriptMsg(o.first, GetUniqueId(), o.second);
}
} // namespace urde
