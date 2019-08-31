#pragma once

#include "Character/CSteeringBehaviors.hpp"
#include "World/CDamageInfo.hpp"
#include "World/CPatterned.hpp"

namespace urde::MP1 {
class CEnergyBall : public CPatterned {
  CSteeringBehaviors x568_steeringBehaviors;
  float x56c_ = 0.f;
  s32 x570_;
  float x574_;
  CDamageInfo x578_;
  float x594_initialTurnSpeed;
  float x598_;
  CAssetId x59c_;
  s16 x5a0_;
  CAssetId x5a4_;
  TToken<CElectricDescription> x5a8_; // originally an rstl::optional_object
  s16 x5b4_;
  float x5b8_;
  float x5bc_;
  TToken<CGenDescription> x5c0_;
  CDamageInfo x5cc_;
  float x5e8_;

  void sub8029f4a8(CStateManager& mgr);
public:
  DEFINE_PATTERNED(EnergyBall)
  CEnergyBall(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
              CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo, s32 w1, float f1,
              const CDamageInfo& dInfo1, float f2, const CAssetId& a1, s16 sfxId1, const CAssetId& a2,
              const CAssetId& a3, s16 sfxId2, float f3, float f4, const CAssetId& a4, const CDamageInfo& dInfo2,
              float f5);


  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Death(CStateManager& mgr, const zeus::CVector3f& direction, EScriptObjectState state) override;
  void Generate(CStateManager& mgr, EStateMsg msg, float arg) override;
  void Attack(CStateManager& mgr, EStateMsg msg, float arg) override;
};
}
