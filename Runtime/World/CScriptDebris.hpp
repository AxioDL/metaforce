#pragma once

#include <array>
#include <memory>
#include <string_view>

#include "Runtime/GCNTypes.hpp"
#include "Runtime/World/CPhysicsActor.hpp"

#include <zeus/CColor.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {
class CElementGen;
class CScriptDebris : public CPhysicsActor {
public:
  enum class EScaleType { NoScale, EndsToZero };

  enum class EOrientationType { NotOriented, AlongVelocity, ToObject, AlongCollisionNormal };

private:
  zeus::CVector3f x258_velocity;
  zeus::CColor x264_color;
  zeus::CColor x268_endsColor;
  float x26c_zImpulse = 0.f;
  float x270_curTime = 0.f;
  float x274_duration = 0.f;
  float x278_ooDuration = 0.f;
  float x27c_restitution;
  CScriptDebris::EScaleType x280_scaleType = CScriptDebris::EScaleType::NoScale;
  bool x281_24_randomAngImpulse : 1;
  bool x281_25_particle1GlobalTranslation : 1 = false;
  bool x281_26_deferDeleteTillParticle1Done : 1 = false;
  bool x281_27_particle2GlobalTranslation : 1 = false;
  bool x281_28_deferDeleteTillParticle2Done : 1 = false;
  bool x281_29_particle3Active : 1 = false;
  bool x281_30_debrisExtended : 1 = false;
  bool x281_31_dieOnProjectile : 1 = false;
  bool x282_24_noBounce : 1 = false;
  EOrientationType x283_particleOrs[3] = {};
  float x288_linConeAngle = 0.f;
  float x28c_linMinMag = 0.f;
  float x290_linMaxMag = 0.f;
  float x294_angMinMag = 0.f;
  float x298_angMaxMag = 0.f;
  float x29c_minDuration = 0.f;
  float x2a0_maxDuration = 0.f;
  float x2a4_colorInT = 0.f;
  float x2a8_colorOutT = 0.f;
  float x2ac_scaleOutStartT = 0.f;
  zeus::CVector3f x2b0_scale;
  zeus::CVector3f x2bc_endScale;
  zeus::CVector3f x2c8_collisionNormal;
  std::array<std::unique_ptr<CElementGen>, 3> x2d4_particleGens; /* x2d4, x2d8, x2dc */
  TReservedAverage<float, 8> x2e0_speedAvg;

public:
  CScriptDebris(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                CModelData&& mData, const CActorParameters& aParams, CAssetId particleId,
                const zeus::CVector3f& particleScale, float zImpulse, const zeus::CVector3f& velocity,
                const zeus::CColor& endsColor, float mass, float restitution, float duration, EScaleType scaleType,
                bool b1, bool randomAngImpulse, bool active);

  CScriptDebris(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                CModelData&& mData, const CActorParameters& aParams, float linConeAngle, float linMinMag,
                float linMaxMag, float angMinMag, float angMaxMag, float minDuration, float maxDuration, float colorInT,
                float colorOutT, const zeus::CColor& color, const zeus::CColor& endsColor, float scaleOutStartT,
                const zeus::CVector3f& scale, const zeus::CVector3f& endScale, float restitution, float downwardSpeed,
                const zeus::CVector3f& localOffset, CAssetId particle1, const zeus::CVector3f& particle1Scale,
                bool particle1GlobalTranslation, bool deferDeleteTillParticle1Done, EOrientationType particle1Or,
                CAssetId particle2, const zeus::CVector3f& particle2Scale, bool particle2GlobalTranslation,
                bool deferDeleteTillParticle2Done, EOrientationType particle2Or, CAssetId particle3,
                const zeus::CVector3f& particle3Scale, EOrientationType particle3Or, bool solid, bool dieOnProjectile,
                bool noBounce, bool active);

  void Accept(IVisitor& visitor) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void Touch(CActor& other, CStateManager& mgr) override;
  std::optional<zeus::CAABox> GetTouchBounds() const override;
  void PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) override;
  void Render(CStateManager& mgr) override;

  void CollidedWith(TUniqueId uid, const CCollisionInfoList&, CStateManager&) override;
};
} // namespace urde
