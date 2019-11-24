#include "Runtime/MP1/World/CThardus.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/World/CActorParameters.hpp"
#include "Runtime/World/CDestroyableRock.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
CThardus::CThardus(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                   CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
                   const std::vector<CStaticRes>& mData1, const std::vector<CStaticRes>& mData2, CAssetId particle1,
                   CAssetId particle2, CAssetId particle3, float f1, float f2, float f3, float f4, float f5, float f6,
                   CAssetId stateMachine, CAssetId particle4, CAssetId particle5, CAssetId particle6,
                   CAssetId particle7, CAssetId particle8, CAssetId particle9, CAssetId texture, u32 sfxId1,
                   CAssetId particle10, u32 sfxId2, u32 sfxId3, u32 sfxId4)
: CPatterned(ECharacter::Thardus, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo,
             EMovementType::Ground, EColliderType::One, EBodyType::BiPedal, actParms, EKnockBackVariant::Large)
, x5cc_(std::move(mData1))
, x5dc_(std::move(mData2))
, x600_(particle1)
, x604_(particle2)
, x608_(particle3)
, x630_(stateMachine)
, x694_(f1)
, x698_(f2)
, x69c_(f3)
, x6a0_(f4)
, x6a4_(f5)
, x6a8_(f6)
, x6d0_(particle4)
, x6d4_(particle5)
, x6d8_(particle6)
, x6dc_(particle7)
, x6e0_(particle8)
, x6e4_(particle9)
, x6e8_(texture)
, x6ec_(CSfxManager::TranslateSFXID(sfxId1))
, x6f0_(particle10)
, x758_(sfxId2)
, x75c_(sfxId3)
, x760_(sfxId4)
, x7f0_(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f) {
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  exclude.Add(EMaterialTypes::Player);
  exclude.Add(EMaterialTypes::Character);
  exclude.Add(EMaterialTypes::CollisionActor);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  std::vector<CAssetId> gens;
  gens.reserve(6);
  gens.push_back(particle4);
  gens.push_back(particle5);
  gens.push_back(particle6);
  gens.push_back(particle7);
  gens.push_back(particle8);
  gens.push_back(particle9);
  GetModelData()->GetAnimationData()->GetParticleDB().CacheParticleDesc(
      CCharacterInfo::CParticleResData(gens, {}, {}, {}));
  x798_.reserve(6);
  x7a8_.reserve(16);
  UpdateThermalFrozenState(true);
  xd0_damageMag = 0.f;
  x50c_baseDamageMag = 0.f;
  x8f4_.reserve(16);
  x91c_.Lock();
  x403_26_stateControlledMassiveDeath = false;
  x460_knockBackController.SetAutoResetImpulse(false);
  SetMass(100000.f);
}

void CThardus::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case EScriptObjectMessage::Reset: {
    x95c_ = true;
    x450_bodyController->SetFallState(pas::EFallState::Zero);
    x450_bodyController->SetState(pas::EAnimationState::Locomotion);
    x93d_ = false;
    break;
  }
  case EScriptObjectMessage::SetToMax: {
    for (size_t i = x648_; i < x610_.size() - 1; ++i) {
      if (CEntity* ent = mgr.ObjectById(x610_[i])) {
        ent->SetActive(false);
      }
      ++x648_;
    }
    break;
  }
  case EScriptObjectMessage::Stop: {
    Death(mgr, {}, EScriptObjectState::DeathRattle);
    break;
  }
  case EScriptObjectMessage::Action: {
    if (!x5c8_)
      x5c8_ = true;
    break;
  }
  case EScriptObjectMessage::Touched: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      if (TCastToPtr<CPlayer> pl = mgr.ObjectById(colAct->GetLastTouchedObject())) {
        if (x420_curDamageRemTime > 0.f)
          break;
        u32 rand = mgr.GetActiveRandom()->Next();
        float damageMult = 1.f;
        zeus::CVector3f knockBack = zeus::skForward;
        if (x644_ == 1) {
          damageMult = 2.f;
          knockBack = (rand % 2) ? zeus::skRight : zeus::skLeft;
        }

        if (mgr.GetPlayer().GetFrozenState())
          mgr.GetPlayer().UnFreeze(mgr);

        knockBack = GetTransform().buildMatrix3f() * knockBack;
        CDamageInfo dInfo = GetContactDamage();
        dInfo.SetDamage(damageMult * dInfo.GetDamage());
        mgr.ApplyDamage(GetUniqueId(), pl->GetUniqueId(), GetUniqueId(), dInfo,
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}),
                        x644_ == 1 ? knockBack : zeus::skZero3f);
        x420_curDamageRemTime = x424_damageWaitTime;
      } else if (TCastToConstPtr<CBomb>(mgr.GetObjectById(colAct->GetLastTouchedObject()))) {
#if 0
        if (x644_ == 1 && x93c_)
          sub801dae2c(mgr, x648_);
#endif
      }
    }

    break;
  }
  case EScriptObjectMessage::Registered: {
    x610_.reserve(x5cc_.size());
    x6b0_.reserve(x5cc_.size());
    x6c0_.reserve(x5cc_.size());
    x90c_.reserve(x5cc_.size());
    for (size_t i = 0; i < x5cc_.size(); ++i) {
      float dVar24 = (i == x5cc_.size() - 1) ? 2.f * x6a8_ : x6a8_;
      TUniqueId rockId = mgr.AllocateUniqueId();
      CModelData mData1(x5cc_[i]);
      CModelData mData2(x5dc_[i]);
      mgr.AddObject(new CDestroyableRock(
          rockId, true, "", CEntityInfo(GetAreaIdAlways(), NullConnectionList), {}, std::move(mData1), 0.f,
          CHealthInfo(dVar24, 0.f),
          CDamageVulnerability(
              EVulnerability::Normal, EVulnerability::Deflect, EVulnerability::Normal, EVulnerability::Normal,
              EVulnerability::Normal, EVulnerability::Normal, EVulnerability::Normal, EVulnerability::Deflect,
              EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect,
              EVulnerability::Deflect, EVulnerability::Deflect, EVulnerability::Deflect, EDeflectType::One),
          GetMaterialList(), x630_,
          CActorParameters(CLightParameters(false, 0.f, CLightParameters::EShadowTesselation::Invalid, 0.f, 0.f,
                                            zeus::skWhite, false, CLightParameters::EWorldLightingOptions::NoShadowCast,
                                            CLightParameters::ELightRecalculationOptions::LargeFrameCount,
                                            zeus::skZero3f, -1, -1, 0, 0),
                           {}, {}, {}, {}, true, true, false, false, 0.f, 0.f, 1.f),
          std::move(mData2), 0));
    }

    AddMaterial(EMaterialTypes::ScanPassthrough, mgr);
    AddMaterial(EMaterialTypes::CameraPassthrough, mgr);
    RemoveMaterial(EMaterialTypes::Orbit, mgr);
    RemoveMaterial(EMaterialTypes::Target, mgr);
    // sub801ddbe4(mgr);
    x450_bodyController->SetFallState(pas::EFallState::Two);
    x450_bodyController->Activate(mgr);
    x450_bodyController->BodyStateInfo().SetLocoAnimChangeAtEndOfAnimOnly(true);
    // sub801db560(0, mgr);
    // sub801dec80();
    AddMaterial(EMaterialTypes::RadarObject, mgr);
    break;
  }
  default:
    break;
  }
}
void CThardus::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x5ec_ = 0;
  } else if (msg == EStateMsg::Update) {
    if (x5ec_ == 0) {
      if (x450_bodyController->GetCurrentStateId() == pas::EAnimationState::Getup) {
        x5ec_ = 2;
      } else {
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCGetupCmd(pas::EGetupType::Zero));
      }
    } else if (x5ec_ == 2 && x450_bodyController->GetCurrentStateId() != pas::EAnimationState::Getup) {
      x5ec_ = 3;
    }
  } else if (msg == EStateMsg::Deactivate) {
    x93d_ = false;
  }
}
void CThardus::GetUp(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Activate)
    return;
  RemoveMaterial(EMaterialTypes::RadarObject, mgr);
}
}