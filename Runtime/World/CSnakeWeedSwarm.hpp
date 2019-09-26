#pragma once

#include "Runtime/Collision/CCollisionSurface.hpp"
#include "Runtime/World/CActor.hpp"

namespace urde {
class CAnimationParameters;

class CSnakeWeedSwarm : public CActor {
public:
  CSnakeWeedSwarm(TUniqueId, bool, std::string_view, const CEntityInfo&, const zeus::CVector3f&, const zeus::CVector3f&,
                  const CAnimationParameters&, const CActorParameters&, float, float, float, float, float, float, float,
                  float, float, float, float, float, float, float, const CDamageInfo&, float, u32, u32, u32, u32, u32,
                  u32, float);

  void Accept(IVisitor&) override;
  void ApplyRadiusDamage(const zeus::CVector3f& pos, const CDamageInfo& info, CStateManager& stateMgr) {}
};
} // namespace urde
