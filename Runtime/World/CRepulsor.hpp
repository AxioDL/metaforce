#pragma once

#include "CActor.hpp"
namespace urde {
class CRepulsor : public CActor {
  float xe8_affectRadius;

public:
  CRepulsor(TUniqueId, bool, std::string_view, const CEntityInfo&, const zeus::CVector3f&, float);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;

  float GetAffectRadius() const { return xe8_affectRadius; }
};
} // namespace urde
