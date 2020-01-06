#pragma once

#include "DataSpec/DNAMP1/SFX/Metaree.h"

#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CPatterned.hpp"

#include <zeus/CVector3f.hpp>

namespace urde::MP1 {
class CMetaree : public CPatterned {
  float x568_delay;
  float x56c_;
  float x570_dropHeight;
  zeus::CVector3f x574_offset;
  float x580_attackSpeed;
  zeus::CVector3f x584_lookPos;
  zeus::CVector3f x590_projectileDelta;
  zeus::CVector3f x59c_velocity;
  u32 x5a8_ = 0;
  CDamageInfo x5ac_damageInfo;
  u16 x5c8_attackSfx = SFXsfx0225;

  struct {
    struct {
      bool x5ca_24_ : 1;
      bool x5ca_25_started : 1;
      bool x5ca_26_deactivated : 1;
    };
    u16 _dummy;
  };

  u32 x5cc_;

public:
  DEFINE_PATTERNED(Metaree)
  CMetaree(TUniqueId, std::string_view, EFlavorType, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
           const CPatternedInfo&, const CDamageInfo&, float, const zeus::CVector3f&, float, EBodyType, float, float,
           const CActorParameters&);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;

  void Touch(CActor&, CStateManager&) override;
  void CollidedWith(TUniqueId, const CCollisionInfoList&, CStateManager&) override;
  void ThinkAboutMove(float) override {}
  bool Delay(CStateManager&, float) override { return x330_stateMachineState.GetTime() > x568_delay; }
  void Explode(CStateManager&, EStateMsg, float) override;
  void Flee(CStateManager&, EStateMsg, float) override;
  void Dead(CStateManager&, EStateMsg, float) override;
  void Attack(CStateManager&, EStateMsg, float) override;
  void Halt(CStateManager&, EStateMsg, float) override;
  void Active(CStateManager&, EStateMsg, float) override;
  void InActive(CStateManager&, EStateMsg, float) override;
  bool InRange(CStateManager&, float) override;
  bool ShouldAttack(CStateManager&, float) override;
};
} // namespace urde::MP1
