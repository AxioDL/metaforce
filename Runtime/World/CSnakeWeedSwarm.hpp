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
    x0 = 0,
    x1 = 1,
    x2 = 2,
    x3 = 3,
  };

  class CBoid {
    zeus::CVector3f x0_pos;
    EBoidState xc_state;
    float x10_ = 0.f; // some timer
    float x14_; // height?
    float x18_;
    float x1c_ = 0.f;
    float x20_;

  public:
    constexpr CBoid(const zeus::CVector3f& pos, float f1, float f2, float f3)
    : x0_pos(pos), xc_state(EBoidState::x1), x14_(f1), x18_(f2), x20_(f3) {}

    constexpr const zeus::CVector3f& GetPosition() const { return x0_pos; }
    constexpr EBoidState GetState() const { return xc_state; }
    constexpr float Get_x10() const { return x10_; }
    constexpr float Get_x14() const { return x14_; }
    constexpr float Get_x18() const { return x18_; }
    constexpr float Get_x20() const { return x20_; }
    constexpr void SetState(EBoidState v) { xc_state = v; }
    constexpr void Set_x10(float v) { x10_ = v; }
    constexpr void Set_x14(float v) { x14_ = v; }
    constexpr void Set_x18(float v) { x18_ = v; }
  };

private:
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
  std::vector<CBoid> x134_boids;
  bool x140_24_ : 1;
  bool x140_25_modelAssetDirty : 1;
  bool x140_26_ : 1;
  zeus::CAABox x144_ = zeus::skInvertedBox;
  CDamageInfo x15c_dInfo;
//  std::vector<std::shared_ptr<CSkinnedModel>> x178_;
//  std::vector<std::unique_ptr<CSkinnedModel>> x19c_;
  rstl::reserved_vector<std::shared_ptr<CModelData>, 4> x1b0_modelData;
  CModelData::EWhichModel x1c4_;
  std::unique_ptr<std::vector<zeus::CVector3f>> x1c8_;
  std::unique_ptr<std::vector<int>> x1cc_;
  u16 x1d0_sfx1;
  u16 x1d2_sfx2;
  u16 x1d4_sfx3;
  CSfxHandle x1d8_ = 0;
  TLockedToken<CGenDescription> x1dc_;
  // TLockedToken<CGenDescription> x1e4_; both assign to x1dc_?
  std::unique_ptr<CElementGen> x1ec_particleGen1;
  std::unique_ptr<CElementGen> x1f4_particleGen2;
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
  std::optional<zeus::CAABox> GetTouchBounds() const override {
    if (x140_24_) {
      return {};
    }
    return x144_;
  }
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const override;
  void Touch(CActor&, CStateManager&) override;
  void Think(float, CStateManager&) override;

private:
  void AllocateSkinnedModels(CStateManager& mgr, CModelData::EWhichModel which);
  void sub_8023ca48(float param_1, CStateManager& mgr, const zeus::CVector3f& pos);
  void sub_8023c238(const CStateManager& mgr);
  zeus::CAABox sub_8023d3f4();
  int sub_8023c0fc();
  int sub_8023c154();
  void sub_8023bca8(CStateManager& mgr, int v);
  zeus::CVector2i sub_8023c1ac(const zeus::CVector3f& vec);
  bool CreateBoid(const zeus::CVector3f& vec, CStateManager& mgr);
  float sub_8023bbc8(const zeus::CVector3f& vec);
  float sub_8023bc38(const zeus::CVector3f& vec);
  void sub_8023bfb8(const zeus::CVector3f& vec);
  void sub_8023d204();
  void EmitParticles1(const zeus::CVector3f& pos);
  void EmitParticles2(const zeus::CVector3f& pos);
  void RenderBoid(u32 p1, const CBoid* boid, u32 *p3) const;
};
} // namespace urde
