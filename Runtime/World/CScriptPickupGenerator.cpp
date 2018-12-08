#include "CScriptPickupGenerator.hpp"
#include "TCastTo.hpp"

namespace urde {
CScriptPickupGenerator::CScriptPickupGenerator(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                               const zeus::CVector3f& pos, float frequency, bool active)
: CEntity(uid, info, active, name), x34_position(pos), x40_frequency(frequency) {}

void CScriptPickupGenerator::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptPickupGenerator::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) {
  if (msg == EScriptObjectMessage::SetToZero && x30_24_active && x44_ != 100.f) {
    x44_ -= 1.f;
    if (x44_ < 0.000009f)
      sub8015E220();
    else {
    }
  }

  CEntity::AcceptScriptMsg(msg, objId, stateMgr);
}

void CScriptPickupGenerator::sub8015E220() {
  if (x40_frequency > 0.f)
    x44_ = (100.f / x40_frequency) + 100.f;
  else
    x44_ = std::numeric_limits<float>::max();
}
} // namespace urde
