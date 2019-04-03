#pragma once

#include "CActor.hpp"

namespace urde {
class CFishCloudModifier : public CActor {
  float xe8_radius;
  float xec_priority;
  bool xf0_isRepulsor;
  bool xf1_swirl;

public:
  CFishCloudModifier(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& eInfo,
                     const zeus::CVector3f& pos, bool isRepulsor, bool swirl, float radius,
                     float priority);
  void Accept(IVisitor& visitor);
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);

  void AddSelf(CStateManager&);
  void RemoveSelf(CStateManager&);
};
} // namespace urde
