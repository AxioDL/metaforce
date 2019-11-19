#include "Runtime/MP1/World/CThardus.hpp"

#include "Runtime/CStateManager.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Collision/CCollisionActorManager.hpp"
#include "Runtime/World/CPatternedInfo.hpp"
#include "Runtime/World/CPlayer.hpp"

#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde::MP1 {
CThardus::CThardus(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                   CModelData&& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
                   std::vector<CModelData> mData1, std::vector<CModelData> mData2, CAssetId particle1,
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
      if (CEntity* ent = mgr.ObjectById(x610_[i]))
        ent->SetActive(false);
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

      } else if (TCastToConstPtr<CBomb> bomb = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
        u32 rand = mgr.GetActiveRandom()->Next();
        float f = 1.f;
        zeus::CVector3f vec;
        if (x644_ == 1) {
          f = 2.f;
          if ((rand & (1 ^ rand) >> 0x1f) == rand >> 31) {
            vec = zeus::skRight;
          } else
            vec = zeus::skLeft;
        }
      }
    }
    break;
  }
  case EScriptObjectMessage::Registered: {
    x610_.reserve(x5cc_.size());
    x6b0_.reserve(x5cc_.size());
    x6c0_.reserve(x5cc_.size());
    x90c_.reserve(x5cc_.size());
    /*
    for (const auto& mData : x5cc_) {

    }*/

    AddMaterial(EMaterialTypes::ScanPassthrough, mgr);
    AddMaterial(EMaterialTypes::CameraPassthrough, mgr);
    RemoveMaterial(EMaterialTypes::Orbit, mgr);
    RemoveMaterial(EMaterialTypes::Target, mgr);
    //sub801ddbe4(mgr);
    x450_bodyController->SetFallState(pas::EFallState::Two);
    x450_bodyController->Activate(mgr);
    x450_bodyController->BodyStateInfo().SetLocoAnimChangeAtEndOfAnimOnly(true);
    //sub801db560(0, mgr);
    //sub801dec80();
    AddMaterial(EMaterialTypes::RadarObject, mgr);
    break;
  }
  default:
    break;
  }
}
}