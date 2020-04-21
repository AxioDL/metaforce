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
    Lowered = 2,
    Lowering = 3,
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
    float x10_loweredTimer = 0.f;
    float x14_zOffset;
    float x18_speed;
    // x1c unused
    float x20_scale;

  public:
    constexpr CBoid(const zeus::CVector3f& pos, float zOffset, float speed, float scale)
    : x0_pos(pos), xc_state(EBoidState::Raising), x14_zOffset(zOffset), x18_speed(speed), x20_scale(scale) {}

    constexpr const zeus::CVector3f& GetPosition() const { return x0_pos; }
    constexpr EBoidState GetState() const { return xc_state; }
    constexpr float GetLoweredTimer() const { return x10_loweredTimer; }
    constexpr float GetZOffset() const { return x14_zOffset; }
    constexpr float GetSpeed() const { return x18_speed; }
    constexpr float GetScale() const { return x20_scale; }
    constexpr void SetState(EBoidState v) { xc_state = v; }
    constexpr void SetLoweredTimer(float v) { x10_loweredTimer = v; }
    constexpr void SetZOffset(float v) { x14_zOffset = v; }
    constexpr void SetSpeed(float v) { x18_speed = v; }
  };

private:
  zeus::CVector3f xe8_scale;
  float xf4_boidSpacing;
  float xf8_height;
  float xfc_;
  float x100_weaponDamageRadius;
  float x104_maxPlayerDistance;
  float x108_loweredTime;
  float x10c_loweredTimeVariation;
  float x110_maxZOffset;
  float x114_speed;
  float x118_speedVariation;
  float x11c_;
  float x120_scaleMin;
  float x124_scaleMax;
  float x128_distanceBelowGround;
  // u32 x12c_ = 0;
  std::vector<CBoid> x134_boids;
  bool x140_24_hasGround : 1 = false;
  bool x140_25_modelAssetDirty : 1 = false;
  bool x140_26_playerTouching : 1 = false;
  zeus::CAABox x144_touchBounds = zeus::skInvertedBox;
  CDamageInfo x15c_damageInfo;
  // x178_ / x19c_: vectors of CSkinnedModel*, not needed
  rstl::reserved_vector<std::unique_ptr<CModelData>, 4> x1b0_modelData;
  CModelData::EWhichModel x1c4_which;
  std::unique_ptr<std::vector<zeus::CVector3f>> x1c8_boidPositions;
  std::unique_ptr<std::vector<EBoidPlacement>> x1cc_boidPlacement;
  u16 x1d0_sfx1;
  u16 x1d2_sfx2;
  u16 x1d4_sfx3;
  CSfxHandle x1d8_sfxHandle;
  TLockedToken<CGenDescription> x1dc_particleGenDesc;
  // TLockedToken<CGenDescription> x1e4_; both assign to x1dc_
  std::unique_ptr<CElementGen> x1ec_particleGen1;
  std::unique_ptr<CElementGen> x1f4_particleGen2;
  u32 x1fc_;
  float x200_; // unused?
  float x204_particleTimer = 0.f;

public:
  CSnakeWeedSwarm(TUniqueId uid, bool active, std::string_view name, const CEntityInfo& info,
                  const zeus::CVector3f& pos, const zeus::CVector3f& scale, const CAnimRes& animRes,
                  const CActorParameters& actParms, float spacing, float height, float f3, float weaponDamageRadius,
                  float maxPlayerDistance, float loweredTime, float loweredTimeVariation, float maxZOffset, float speed,
                  float speedVariation, float f11, float scaleMin, float scaleMax, float distanceBelowGround,
                  const CDamageInfo& dInfo, float /*f15*/, u32 sfxId1, u32 sfxId2, u32 sfxId3, CAssetId particleGenDesc1, u32 w5,
                  CAssetId particleGenDesc2, float f16);

  void Accept(IVisitor&) override;
  void ApplyRadiusDamage(const zeus::CVector3f& pos, const CDamageInfo& info, CStateManager& stateMgr);
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override;
  void Touch(CActor&, CStateManager&) override;
  void Think(float, CStateManager&) override;

private:
  void AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which);
  void HandleRadiusDamage(float radius, CStateManager& mgr, const zeus::CVector3f& pos);
  void FindGround(const CStateManager& mgr);
  zeus::CAABox GetBoidBox() const;
  int GetNumBoidsY() const;
  int GetNumBoidsX() const;
  void CreateBoids(CStateManager& mgr, int num);
  zeus::CVector2i GetBoidIndex(const zeus::CVector3f& pos) const;
  bool CreateBoid(const zeus::CVector3f& vec, CStateManager& mgr);
  float GetBoidOffsetY(const zeus::CVector3f& pos) const;
  float GetBoidOffsetX(const zeus::CVector3f& pos) const;
  void AddBoidPosition(const zeus::CVector3f& pos);
  void CalculateTouchBounds();
  void EmitParticles1(const zeus::CVector3f& pos);
  void EmitParticles2(const zeus::CVector3f& pos);
  void RenderBoid(u32 idx, const CBoid& boid, u32& posesToBuild) const;
};
} // namespace urde
