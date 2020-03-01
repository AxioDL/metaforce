#pragma once

#include "Runtime/Collision/CCollisionSurface.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CDamageInfo.hpp"

namespace urde {
class CAnimationParameters;

class CSnakeWeedSwarm : public CActor {
public:
  enum class EBoidState : u32 {
    Raised = 0,
    Raising = 1,
    x2 = 2,
    x3 = 3,
  };

  enum class EBoidPlacement : u32 {
    None = 0,
    Ready = 1,
    Invalid = 2,
    Placed = 3,
  };

  class CBoid {
    zeus::CVector3f x0_pos;
    EBoidState xc_state;
    float x10_ = 0.f; // some timer
    float x14_yOffset;
    float x18_;
    float x1c_ = 0.f;
    float x20_;

  public:
    constexpr CBoid(const zeus::CVector3f& pos, float f1, float f2, float f3)
    : x0_pos(pos), xc_state(EBoidState::Raising), x14_yOffset(f1), x18_(f2), x20_(f3) {}

    constexpr const zeus::CVector3f& GetPosition() const { return x0_pos; }
    constexpr EBoidState GetState() const { return xc_state; }
    constexpr float Get_x10() const { return x10_; }
    constexpr float GetYOffset() const { return x14_yOffset; }
    constexpr float Get_x18() const { return x18_; }
    constexpr float Get_x20() const { return x20_; }
    constexpr void SetState(EBoidState v) { xc_state = v; }
    constexpr void Set_x10(float v) { x10_ = v; }
    constexpr void SetYOffset(float v) { x14_yOffset = v; }
    constexpr void Set_x18(float v) { x18_ = v; }
  };

private:
  zeus::CVector3f xe8_scale;
  float xf4_boidSpacing;
  float xf8_height;
  float xfc_;
  float x100_weaponDamageRadius;
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
  // u32 x12c_ = 0;
  std::vector<CBoid> x134_boids;
  bool x140_24_hasGround : 1;
  bool x140_25_modelAssetDirty : 1;
  bool x140_26_playerTouching : 1;
  zeus::CAABox x144_touchBounds = zeus::skInvertedBox;
  CDamageInfo x15c_damageInfo;
  // x178_ / x19c_: vectors of CSkinnedModel*, not needed
  rstl::reserved_vector<std::shared_ptr<CModelData>, 4> x1b0_modelData;
  CModelData::EWhichModel x1c4_which;
  std::unique_ptr<std::vector<zeus::CVector3f>> x1c8_boidPositions;
  std::unique_ptr<std::vector<EBoidPlacement>> x1cc_boidPlacement;
  u16 x1d0_sfx1;
  u16 x1d2_sfx2;
  u16 x1d4_sfx3;
  CSfxHandle x1d8_sfxHandle = 0;
  TLockedToken<CGenDescription> x1dc_particleGenDesc;
  // TLockedToken<CGenDescription> x1e4_; both assign to x1dc_
  std::unique_ptr<CElementGen> x1ec_particleGen1;
  std::unique_ptr<CElementGen> x1f4_particleGen2;
  u32 x1fc_;
  float x200_;
  float x204_particleTimer = 0.f;

public:
  CSnakeWeedSwarm(TUniqueId, bool, std::string_view, const CEntityInfo&, const zeus::CVector3f&, const zeus::CVector3f&,
                  const CAnimRes&, const CActorParameters&, float, float, float, float, float, float, float, float,
                  float, float, float, float, float, float, const CDamageInfo&, float, u32, u32, u32, CAssetId, u32,
                  CAssetId, float);

  void Accept(IVisitor&) override;
  void ApplyRadiusDamage(const zeus::CVector3f& pos, const CDamageInfo& info, CStateManager& stateMgr);
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const override;
  void Touch(CActor&, CStateManager&) override;
  void Think(float, CStateManager&) override;

private:
  void AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which);
  void HandleRadiusDamage(float radius, CStateManager& mgr, const zeus::CVector3f& pos);
  void FindGround(const CStateManager& mgr);
  zeus::CAABox GetBoidBox();
  int GetNumBoidsY();
  int GetNumBoidsX();
  void CreateBoids(CStateManager& mgr, int num);
  zeus::CVector2i GetBoidIndex(const zeus::CVector3f& pos);
  bool CreateBoid(const zeus::CVector3f& vec, CStateManager& mgr);
  float GetBoidOffsetY(const zeus::CVector3f& pos);
  float GetBoidOffsetX(const zeus::CVector3f& pos);
  void AddBoidPosition(const zeus::CVector3f& pos);
  void CalculateTouchBounds();
  void EmitParticles1(const zeus::CVector3f& pos);
  void EmitParticles2(const zeus::CVector3f& pos);
  void RenderBoid(u32 idx, const CBoid& boid, u32& posesToBuild) const;
};
} // namespace urde
