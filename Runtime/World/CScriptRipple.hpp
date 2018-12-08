#pragma once

#include "CEntity.hpp"
#include "zeus/CVector3f.hpp"

namespace urde {
class CScriptRipple : public CEntity {
  float x34_magnitude;
  zeus::CVector3f x38_center;

public:
  CScriptRipple(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CVector3f&, bool, float);

  void Accept(IVisitor&);
  void Think(float, CStateManager&) {}
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
};
} // namespace urde
