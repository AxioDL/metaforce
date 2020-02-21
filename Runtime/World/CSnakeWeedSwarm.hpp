#pragma once

#include "Runtime/Collision/CCollisionSurface.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"

namespace urde {
class CAnimationParameters;

class CSnakeWeedSwarm : public CActor {
  zeus::CVector3f xe8_scale;
  float xf4_;
  float xf8_;
  float xfc_;
  float x100_;
  float x104_;
  float x108_;
  float x10c_;
  float x110_;
  float x114_;
  float x118_;
  float x11c_;
  float x120_;
  float x124_;
  float x128_;
  u32 x12c_ = 0;
  // x130_
  u32 x134_ = 0;
  u32 x138_ = 0;
  u32 x13c_ = 0;
  bool x13c_xbf_modelAssetDirty : 1;
  zeus::CAABox x144_ = zeus::skInvertedBox;
  CDamageInfo x15c_dInfo;
  // x178_ = 0
  // x19c_ = 0
  rstl::reserved_vector<std::shared_ptr<CModelData>, 4> x1b4_modelData;
  // x1c8_ = ptr to 0x10 sz?
  // x1cc_ = ptr to 0x10 sz?
  u16 x1d0_sfx1;
  u16 x1d2_sfx2;
  u16 x1d4_sfx3;
  u32 x1d8_ = 0;
  TLockedToken<CGenDescription> x1dc_;
  // TLockedToken<CGenDescription> x1e4_; both assign to x1dc_?
  std::unique_ptr<CElementGen> x1ec_;
  std::unique_ptr<CElementGen> x1f4_;
  u32 x1fc_;
  float x200_;
  float x204_ = 0.f;

public:
  CSnakeWeedSwarm(TUniqueId, bool, std::string_view, const CEntityInfo&, const zeus::CVector3f&, const zeus::CVector3f&,
                  const CAnimRes&, const CActorParameters&, float, float, float, float, float, float, float, float,
                  float, float, float, float, float, float, const CDamageInfo&, float, u32, u32, u32, CAssetId, u32,
                  CAssetId, float);

  void Accept(IVisitor&) override;
  void ApplyRadiusDamage(const zeus::CVector3f& pos, const CDamageInfo& info, CStateManager& stateMgr) {}
  std::optional<zeus::CAABox> GetTouchBounds() const override { /* FIXME check flags */ return x144_; }
};
} // namespace urde
