#pragma once

#include "CGameProjectile.hpp"
#include "Camera/CCameraShakeData.hpp"

namespace urde {

class CEnergyProjectile : public CGameProjectile {
  CSfxHandle x2e8_sfx;
  zeus::CVector3f x2ec_dir;
  float x2f8_mag;
  CCameraShakeData x2fc_camShake;
  bool x3d0_24_dead : 1 = false;
  bool x3d0_25_ : 1 = false;
  bool x3d0_26_ : 1 = false;
  bool x3d0_27_camShakeDirty : 1 = false;
  float x3d4_curTime = 0.f;
  void StopProjectile(CStateManager& mgr);

public:
  CEnergyProjectile(bool active, const TToken<CWeaponDescription>& desc, EWeaponType type, const zeus::CTransform& xf,
                    EMaterialTypes excludeMat, const CDamageInfo& damage, TUniqueId uid, TAreaId aid, TUniqueId owner,
                    TUniqueId homingTarget, EProjectileAttrib attribs, bool underwater, const zeus::CVector3f& scale,
                    const std::optional<TLockedToken<CGenDescription>>& visorParticle, u16 visorSfx,
                    bool sendCollideMsg);
  void SetCameraShake(const CCameraShakeData& data) {
    x2fc_camShake = data;
    x3d0_27_camShakeDirty = true;
  }
  void PlayImpactSound(const zeus::CVector3f& pos, EWeaponCollisionResponseTypes type);
  void ChangeProjectileOwner(TUniqueId owner, CStateManager& mgr);
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) override;
  void Accept(IVisitor& visitor) override;
  void ResolveCollisionWithWorld(const CRayCastResult& res, CStateManager& mgr);
  void ResolveCollisionWithActor(const CRayCastResult& res, CActor& act, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void Render(CStateManager& mgr) override;
  void AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) override;
  void Touch(CActor& act, CStateManager& mgr) override;
  virtual bool Explode(const zeus::CVector3f& pos, const zeus::CVector3f& normal, EWeaponCollisionResponseTypes type,
                       CStateManager& mgr, const CDamageVulnerability& dVuln, TUniqueId hitActor);
  void Set3d0_26(bool v) { x3d0_26_ = v; }
};

} // namespace urde
