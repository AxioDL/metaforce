#include "Runtime/MP1/World/CRidley.hpp"

#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/Weapon/CEnergyProjectile.hpp"
#include "Runtime/Weapon/CPlasmaProjectile.hpp"
#include "Runtime/World/CExplosion.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptPlatform.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
namespace MP1 {

namespace {
struct SSomeRidleyStruct {
  u32 x0_;
  u32 x4_;
  float x8_;
  float xc_;
  float x10_;
  float x14_;
  float x18_;
  u32 x1c_;
  u8 x20_;
  u8 x21_;
  u8 x22_;
  u8 x23_;
};

struct SSomeRidleyStruct2 {
  s32 x0_;
  float x4_;
  s32 x8_;
};

constexpr std::array<std::array<SSomeRidleyStruct2, 12>, 5> skSomeRidleyStruct{{
    {{
        {0, 100.f, -1},
        {3, 100.f, -1},
        {2, 100.f, -1},
        {3, 50.f, 4},
        {0, 100.f, -1},
        {3, 100.f, -1},
        {2, 100.f, -1},
        {3, 50.f, 4},
        {5, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
    }},
    {{
        {5, 100.f, -1},
        {1, 100.f, -1},
        {1, 100.f, -1},
        {1, 100.f, -1},
        {1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
    }},
    {{
        {5, 100.f, -1},
        {0, 100.f, -1},
        {0, 50.f, 4},
        {2, 100.f, -1},
        {3, 50.f, 4},
        {2, 100.f, -1},
        {3, 50.f, 4},
        {0, 100.f, -1},
        {2, 50.f, 3},
        {2, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
    }},
    {{
        {5, 100.f, -1},
        {1, 100.f, -1},
        {1, 100.f, -1},
        {1, 100.f, -1},
        {1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
    }},
    {{
        {5, 100.f, -1},
        {0, 100.f, -1},
        {0, 50.f, 3},
        {3, 100.f, -1},
        {0, 50.f, 3},
        {2, 100.f, -1},
        {2, 50.f, 4},
        {0, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
        {-1, 100.f, -1},
    }},
}};

constexpr std::array<SSomeRidleyStruct, 5> skSomeStruct{{{4, 6, 50.f, 50.f, 0.f, 33.f, 0.f, 1, 0, 0, 0, 0},
                                                         {4, 6, 20.f, 20.f, 60.f, 50.f, 0.f, 2, 0, 0, 0, 0},
                                                         {4, 6, 40.f, 40.f, 20.f, 50.f, 50.f, 2, 1, 0, 0, 0},
                                                         {3, 5, 10.f, 15.f, 75.f, 100.f, 25.f, 2, 0, 0, 0, 0},
                                                         {3, 5, 30.f, 30.f, 40.f, 50.f, 50.f, 2, 1, 0, 0, 0}}};
constexpr std::array skWingBones{
    "L_wingBone1_1"sv,    "L_wingBone1_2"sv,    "L_wingBone2_1"sv,    "L_wingBone2_2"sv,    "L_wingBone3_1"sv,
    "L_wingBone3_2"sv,    "L_wingFlesh1_1"sv,   "L_wingFlesh1_2"sv,   "L_wingFlesh2_1"sv,   "L_wingFlesh2_2"sv,
    "L_wingFlesh3_1"sv,   "L_wingFlesh3_2"sv,   "R_wingBone1_1"sv,    "R_wingBone1_2"sv,    "R_wingBone2_1"sv,
    "R_wingBone2_2"sv,    "R_wingBone3_1"sv,    "R_wingBone3_2"sv,    "R_wingFlesh1_1"sv,   "R_wingFlesh1_2"sv,
    "R_wingFlesh2_1"sv,   "R_wingFlesh2_2"sv,   "R_wingFlesh3_1"sv,   "R_wingFlesh3_2"sv,   "L_wingtip_1_LCTR"sv,
    "L_wingtip_2_LCTR"sv, "L_wingtip_3_LCTR"sv, "R_wingtip_1_LCTR"sv, "R_wingtip_2_LCTR"sv, "R_wingtip_3_LCTR"sv,
};

constexpr std::array skWingEffects{
    "WingSmokeSmall1"sv,
    "WingSmokeSmall2"sv,
    "WingSmokeSmall3"sv,
    "WingSmokeSmall4"sv,
    "WingSmokeSmall5"sv,
    "WingSmokeSmall6"sv,
    "WingSmokeSmall7"sv,
    "WingSmokeSmall8"sv
    "WingFire1"sv,
    "WingFire2"sv,
    "WingFire3"sv,
    "WingFire4"sv,
    "WingFire5"sv,
    "WingFire6"sv,
    "WingFire7"sv,
    "WingFire8"sv,
    "WingSparks1"sv,
    "WingSparks2"sv,
    "WingSparks3"sv,
    "WingSparks4"sv,
    "WingSparks5"sv,
    "WingSparks6"sv,
    "WingSparks7"sv,
    "WingSparks8"sv,
};

constexpr std::array<SOBBRadiiJointInfo, 4> skTail{{
    {"Tail_1", "Tail_3", 0.66f},
    {"Tail_3", "Tail_5", 0.66f},
    {"Tail_5", "Tail_7", 0.66f},
    {"Tail_7", "Tail_9", 0.66f},
}};

constexpr std::array<SSphereJointInfo, 10> skSphereJoints{{
    {"Skeleton_Root", 0.6f},
    {"Spine_2", 0.6f},
    {"breastPlate_LCTR", 0.6f},
    {"Head_1", 0.6f},
    {"L_wrist", 0.5f},
    {"R_wrist", 0.5f},
    {"L_ankle", 0.6f},
    {"R_ankle", 0.6f},
    {"L_pinky_1", 0.4f},
    {"R_pinky_1", 0.4f},
}};

struct SSomeRidleyStruct3 {
  float x0_;
  float x4_;
  float x8_;
  float xc_;
  float x10_;
  float x14_;
};

constexpr std::array<SSomeRidleyStruct3, 6> skFloats{{{0.0, 20.0, 40.0, 0.0, 0.0, 40.0},
                                                      {0.0, 0.0, 70.0, 0.0, 0.0, 30.0},
                                                      {0.0, 60.0, 0.0, 0.0, 0.0, 40.0},
                                                      {0.0, 40.0, 30.0, 0.0, 0.0, 30.0},
                                                      {0.0, 0.0, 50.0, 0.0, 0.0, 50.0},
                                                      {0.0, 40.0, 60.0, 0.0, 0.0, 0.0}}};

const CDamageVulnerability skDirectNormal{EVulnerability::DirectNormal, EVulnerability::DirectNormal,
                                          EVulnerability::DirectNormal, EVulnerability::DirectNormal,
                                          EVulnerability::DirectNormal, EVulnerability::DirectNormal,
                                          EVulnerability::DirectNormal, EVulnerability::DirectNormal,
                                          EVulnerability::DirectNormal, EVulnerability::DirectNormal,
                                          EVulnerability::DirectNormal, EVulnerability::DirectNormal,
                                          EVulnerability::DirectNormal, EVulnerability::DirectNormal,
                                          EVulnerability::DirectNormal, EDeflectType::None};
const CDamageVulnerability skIceWeakness{EVulnerability::DirectNormal, EVulnerability::DirectWeak,
                                         EVulnerability::DirectNormal, EVulnerability::DirectNormal,
                                         EVulnerability::DirectNormal, EVulnerability::DirectNormal,
                                         EVulnerability::DirectNormal, EVulnerability::DirectNormal,
                                         EVulnerability::DirectNormal, EVulnerability::DirectNormal,
                                         EVulnerability::DirectNormal, EVulnerability::DirectNormal,
                                         EVulnerability::DirectNormal, EVulnerability::DirectNormal,
                                         EVulnerability::DirectNormal, EDeflectType::None};
} // namespace
CRidleyData::CRidleyData(CInputStream& in, u32 propCount)
: x0_(in)
, x4_(in)
, x8_(in)
, xc_(in)
, x10_(in)
, x14_(in)
, x18_(in)
, x1c_(in)
, x20_(in)
, x24_(in)
, x28_(in)
, x2c_(in)
, x30_(in)
, x34_(in.readFloatBig())
, x38_(in.readFloatBig())
, x3c_(in.readFloatBig())
, x40_(in.readFloatBig())
, x44_(in)
, x48_(in)
, x64_(in)
, xa8_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, xac_(in)
, xb0_(in)
, xcc_(in)
, x1a0_(in)
, x1a4_(in)
, x1c0_(in)
, x294_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x298_(in)
, x2b4_(in)
, x388_(in.readFloatBig())
, x38c_(in.readFloatBig())
, x390_(in)
, x3ac_(in.readFloatBig())
, x3b0_(in)
, x3cc_(in.readFloatBig())
, x3d0_(in)
, x3ec_(in.readFloatBig())
, x3f0_(in)
, x3f4_(in.readFloatBig())
, x3f8_(CSfxManager::TranslateSFXID(in.readUint32Big()))
, x3fc_(propCount > 47 ? CDamageInfo(in) : x48_) {}

CRidley::CRidley(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                 CModelData&& mData, const CPatternedInfo& pInfo, const CActorParameters& actParms, CInputStream& in,
                 u32 propCount)
: CPatterned(ECharacter::Ridley, uid, name, EFlavorType::Zero, info, xf, std::move(mData), pInfo, EMovementType::Flyer,
             EColliderType::Zero, EBodyType::Flyer, actParms, EKnockBackVariant::Large)
, x568_data(in, propCount)
, x98c_(g_SimplePool->GetObj({SBIG('PART'), x568_data.x30_}))
, x998_(CStaticRes(x568_data.x28_, GetModelData()->GetScale()))
, x9e4_(CStaticRes(x568_data.x2c_, GetModelData()->GetScale()))
, xa30_breastPlateSegId(GetModelData()->GetAnimationData()->GetLocatorSegId("breastPlate_LCTR"sv))
, xa38_(CStaticRes(x568_data.x24_, 4.f * GetModelData()->GetScale()))
, xadc_(44.f * GetModelData()->GetScale().z())
, xae0_(20.f * GetModelData()->GetScale().x())
, xae8_(9.f * GetModelData()->GetScale().z())
, xb14_(x568_data.x38_)
, xb18_(x568_data.x3c_)
, xb1c_(x568_data.x40_)
, xb28_(GetModelData()->GetAnimationData()->GetLocatorSegId("Skeleton_Root"sv))
, xb2c_(*GetModelData()->GetAnimationData(), "Head_1"sv, zeus::degToRad(40.f), zeus::degToRad(180.f),
        EBoneTrackingFlags::NoParentOrigin | EBoneTrackingFlags::ParentIk)
, xb68_(x568_data.x44_, x568_data.x48_)
, xb90_headSegId(GetModelData()->GetAnimationData()->GetLocatorSegId("Head_1"sv))
, xb91_mouthSegId(GetModelData()->GetAnimationData()->GetLocatorSegId("mouth_LCTR"sv))
, xb94_(zeus::CTransform::RotateX(zeus::degToRad(-40.f)))
, xc14_(x568_data.xac_, x568_data.xb0_)
, xc3c_(x568_data.x1a0_, x568_data.x1a4_)
, xc8c_(CPatterned::GetContactDamage())
, xcd0_(g_SimplePool->GetObj({SBIG('ELSC'), x568_data.x3f0_}))
, xce0_(std::make_unique<CParticleElectric>(xcd0_))
, xd10_(std::make_unique<CProjectedShadow>(128, 128, true)) {
  xe7_30_doTargetDistanceTest = true;
  xb68_.Token().Lock();
  xc14_.Token().Lock();
  xc3c_.Token().Lock();

  if (xce0_)
    xce0_->SetParticleEmission(false);

  const auto& animData = GetModelData()->GetAnimationData();
  for (size_t i = 0; i < skWingBones.size(); ++i) {
    xce4_wingBoneIds.push_back(animData->GetLocatorSegId(skWingBones[i]));
  }

  xae4_ = GetModelData()->GetScale().x() *
          GetAnimationDistance(CPASAnimParmData(7, CPASAnimParm::FromEnum(4), CPASAnimParm::FromEnum(3)));
  x460_knockBackController.SetAnimationStateRange(EKnockBackAnimationState::Flinch, EKnockBackAnimationState::Flinch);
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableFreeze(false);
  x460_knockBackController.SetEnableBurnDeath(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  CreateShadow(false);
}

void CRidley::SetupCollisionActorManager(urde::CStateManager& mgr) {
  const auto& animData = GetModelData()->GetAnimationData();
  std::vector<CJointCollisionDescription> joints;
  joints.reserve(skTail.size());
  for (const auto& jInfo : skTail) {
    joints.push_back(CJointCollisionDescription::OBBAutoSizeCollision(
        animData->GetLocatorSegId(jInfo.to), animData->GetLocatorSegId(jInfo.from),
        zeus::CVector3f(GetModelData()->GetScale().z() * jInfo.radius),
        CJointCollisionDescription::EOrientationType::One, std::string(GetName()) + " - CollisionActor " + jInfo.from,
        10.f));
  }
  x980_tailCollision = std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, false);
  joints.clear();
  joints.reserve(skSphereJoints.size());
  for (const auto& jInfo : skSphereJoints) {
    joints.push_back(
        CJointCollisionDescription::SphereCollision(animData->GetLocatorSegId(jInfo.name), jInfo.radius,
                                                    std::string(GetName()) + " - CollisionActor " + jInfo.name, 10.f));
  }
  x984_bodyCollision = std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, false);
  x988_headId = x984_bodyCollision->GetCollisionDescFromIndex(3).GetCollisionActorId();
  x98a_breastPlateId = x984_bodyCollision->GetCollisionDescFromIndex(2).GetCollisionActorId();
  SetupCollisionActors(mgr);
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  exclude.Add(EMaterialTypes::Solid);
  include.Remove(EMaterialTypes::Solid);
  exclude.Add(EMaterialTypes::CollisionActor);
  include.Remove(EMaterialTypes::CollisionActor);
  exclude.Add(EMaterialTypes::AIPassthrough);
  include.Remove(EMaterialTypes::AIPassthrough);
  exclude.Add(EMaterialTypes::Player);
  include.Remove(EMaterialTypes::Player);
  exclude.Add(EMaterialTypes::Platform);
  include.Remove(EMaterialTypes::Platform);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  AddMaterial(EMaterialTypes::ProjectilePassthrough);
  // mgr.SendScriptMsg(this, GetUniqueId(), EScriptObjectMessage::Reset);
}

void CRidley::SetupCollisionActors(CStateManager& mgr) {
  for (size_t i = 0; i < x980_tailCollision->GetNumCollisionActors(); ++i) {
    const auto& colDesc = x980_tailCollision->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(colDesc.GetCollisionActorId())) {
      colAct->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
      colAct->HealthInfo(mgr)->SetHP(1000.f);
      colAct->SetMaterialFilter(CMaterialFilter::MakeInclude({EMaterialTypes::Player, EMaterialTypes::Platform}));
      colAct->SetWeaponCollisionResponseType(EWeaponCollisionResponseTypes::EnemyNormal);
    }
  }

  x980_tailCollision->AddMaterial(mgr, {EMaterialTypes::AIJoint});

  for (size_t i = 0; i < x984_bodyCollision->GetNumCollisionActors(); ++i) {
    const auto& colDesc = x984_bodyCollision->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(colDesc.GetCollisionActorId())) {
      colAct->SetDamageVulnerability(skDirectNormal);
      colAct->HealthInfo(mgr)->SetHP(1000.f);
      colAct->SetMaterialFilter(CMaterialFilter::MakeInclude({EMaterialTypes::Player, EMaterialTypes::Platform}));
      colAct->SetWeaponCollisionResponseType(EWeaponCollisionResponseTypes::EnemyNormal);
    }
  }
  x984_bodyCollision->AddMaterial(mgr, {EMaterialTypes::AIJoint});
}

void CRidley::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);

  switch (msg) {
  case EScriptObjectMessage::Registered: {
    x450_bodyController->Activate(mgr);
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    SetupCollisionActorManager(mgr);
    xb10_ = xcb8_ = HealthInfo(mgr)->GetHP();
    xcbc_ = 0.8f * xcb8_;
    break;
  }
  case EScriptObjectMessage::Reset: {
    xa34_26_ = true;
    if (!GetActive()) {
      AcceptScriptMsg(EScriptObjectMessage::Activate, uid, mgr);
    }
    break;
  }
  case EScriptObjectMessage::Activate: {
    mgr.SetBossParams(GetUniqueId(), xb1c_ + xcb8_ + xb18_, 90);
    HealthInfo(mgr)->SetHP(xb1c_ + xb10_ + xb18_);
    mgr.GetPlayer().SetIsOverrideRadarRadius(true);
    mgr.GetPlayer().SetRadarXYRadiusOverride(350.f);
    mgr.GetPlayer().SetRadarZRadiusOverride(175.f);
    break;
  }
  case EScriptObjectMessage::Deactivate: {
    x984_bodyCollision->SetActive(mgr, false);
    x980_tailCollision->SetActive(mgr, false);
    mgr.GetPlayer().SetIsOverrideRadarRadius(false);
    break;
  }
  case EScriptObjectMessage::Deleted: {
    x984_bodyCollision->Destroy(mgr);
    x980_tailCollision->Destroy(mgr);
    if (xb64_plasmaProjectile != kInvalidUniqueId) {
      mgr.FreeScriptObject(xb64_plasmaProjectile);
      xb64_plasmaProjectile = kInvalidUniqueId;
    }
    break;
  }
  case EScriptObjectMessage::InitializedInArea: {
    TUniqueId wpId = GetWaypointForState(mgr, EScriptObjectState::Patrol, EScriptObjectMessage::Follow);
    if (wpId == kInvalidUniqueId)
      break;

    if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(wpId)) {
      xa84_ = wp->GetTransform();
      if (TCastToConstPtr<CScriptWaypoint> wpNext = mgr.GetObjectById(wp->NextWaypoint(mgr))) {
        xab4_ = (wpNext->GetTranslation() - wp->GetTranslation()).toVec2f().magnitude();
        xab8_ = wpNext->GetTranslation().z() - xa84_.origin.z();
        if (TCastToConstPtr<CScriptWaypoint> wpNextNext = mgr.GetObjectById(wpNext->NextWaypoint(mgr))) {
          xabc_ = (wpNextNext->GetTranslation().toVec2f() - xa84_.origin.toVec2f()).magnitude();
          xac0_ = wpNextNext->GetTranslation().z() - xa84_.origin.z();
          zeus::CVector3f min(xa84_.origin.x() - xabc_, xa84_.origin.y() - xabc_, xa84_.origin.z() - 10.f);
          zeus::CVector3f max(xa84_.origin.x() + xabc_, xa84_.origin.y() + xabc_, xa84_.origin.z() + 100.f);
          xac4_ = zeus::CAABox(min, max);
        }
      }
    }
    break;
  }
  case EScriptObjectMessage::Damage: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      float lowHealth = 1000.f - colAct->HealthInfo(mgr)->GetHP();
      colAct->HealthInfo(mgr)->SetHP(1000.f);
      bool dontKnockback = false;
      if (xc64_aiStage == 2) {
        if (xa33_28_) {
          dontKnockback = true;
        } else {
          bool r26 = false;
          xb10_ -= lowHealth;
          xb24_ = 0.33f;
          x430_damageColor = zeus::CColor(0.5f, 0.f, 0.f);
          if (xb10_ <= 0.f) {
            xc64_aiStage = 3;
            xa31_25_ = false;
            sub80257650(mgr);
            xcbc_ = 0.6667f * x568_data.x3c_;
            if (x450_bodyController->GetLocomotionType() != pas::ELocomotionType::Combat) {
              for (const auto& effect : skWingEffects) {
                GetModelData()->GetAnimationData()->SetParticleEffectState(effect, true, mgr);
              }
            }

            xcc8_ = 2.f * 0.33f;
            xb10_ = 0.f;
          } else {
            if (xb10_ < xcbc_) {
              xa32_27_ = true;
              dontKnockback = true;
              xcb0_ += 1;
              xcb0_ = xcb0_ < 5 ? xcb0_ : 4;
              r26 = true;
              xcbc_ = .2f * (5 - (xcb0_ + 1)) * xcb8_;
              xcb4_ = 0;
              xcc8_ = 2.f * 0.33f;
            } else if (xa33_26_ && !xa31_31_ && lowHealth > x568_data.x3f4_) {
              dontKnockback = true;
              x450_bodyController->GetCommandMgr().DeliverCmd(
                  CBCKnockBackCmd(GetTransform().basis[1], pas::ESeverity::Zero));
            }
          }
          sub80256914(2.f * 0.33f, r26);
        }
      } else if (xc64_aiStage == 3) {
        if (xa32_29_) {
          zeus::CTransform xf = GetLctrTransform(xb90_headSegId);
          if (xf.basis[1].dot(mgr.GetPlayer().GetTranslation() - xf.origin) < 0.5f) {
            dontKnockback = true;
          } else {
            TakeDamage(zeus::skForward, 1.f);
            xb20_ = 0.33f;
            if (xa32_29_) {
              if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
                CWeaponMode wMode = proj->GetDamageInfo().GetWeaponMode();
                if (!wMode.IsCharged() && !wMode.IsComboed() && wMode.GetType() == EWeaponType::Missile)
                  xb14_ = 0.f;
                xb14_ -= lowHealth;
                xb24_ = 0.33f;
                x430_damageColor = zeus::CColor(0.5f, 0.f, 0.f);
                if (xb10_ <= 0.f) {
                  xa32_29_ = false;
                  dontKnockback = true;
                  xa32_28_shotAt = true;
                  xb14_ = x568_data.x38_;
                }
              }
            } else if (xa31_27_) {
              x430_damageColor = zeus::CColor(0.5f, 0.f, 0.f);
              if (xb18_ > 0.f) {
                xb18_ -= lowHealth;
                if (xb18_ <= 0.f) {
                  xa31_26_ = true;
                  dontKnockback = true;
                  xb18_ = 0.f;
                  xcbc_ = 0.6667 * x568_data.x40_;
                } else if (xb18_ < xcbc_) {
                  x450_bodyController->GetCommandMgr().DeliverCmd(
                      CBCKnockBackCmd(GetTransform().basis[1], pas::ESeverity::Six));
                  xcbc_ -= (0.333f * x568_data.x3c_);
                }
              }
            } else {
              xb1c_ -= lowHealth;
              if (xb1c_ <= 0.f) {
                x401_30_pendingDeath = true;
                mgr.GetPlayer().SetIsOverrideRadarRadius(false);
                xb1c_ = 0.f;
              } else if (xb1c_ < xcbc_) {
                dontKnockback = true;
                x450_bodyController->GetCommandMgr().DeliverCmd(
                    CBCKnockBackCmd(GetTransform().basis[1], pas::ESeverity::Six));
                xcbc_ -= (0.333f * x568_data.x40_);
              }
            }
          }
        }
      }
      HealthInfo(mgr)->SetHP(xb1c_ + xb10_ + xb18_);

      if (!dontKnockback) {
        if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
          KnockBack(proj->GetTranslation() - proj->GetPreviousPos(), mgr, proj->GetDamageInfo(), EKnockBackType::Radius,
                    false, proj->GetDamageInfo().GetKnockBackPower());
        }
      }
    }
    break;
  }
  case EScriptObjectMessage::InvulnDamage: {
    /* This code never executes, should have a `TCastTo<CCollisionActor>` followed by `GetLastTouchedObject` */
    if (TCastToConstPtr<CCollisionActor> colAct = mgr.GetObjectById(uid)) {
      if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
        TUniqueId tmpId = kInvalidUniqueId;
        bool doDamage = false;
        if (xc64_aiStage == 3) {
          if (!xa32_29_ && xa31_27_) {
            tmpId = x98a_breastPlateId;
            doDamage = true;
          } else {
            tmpId = x988_headId;
            doDamage = true;
          }
        } else if (xc64_aiStage == 2 && !xa31_31_) {
          tmpId = x98a_breastPlateId;
          doDamage = true;
        }

        if (doDamage) {
          CDamageInfo info = proj->GetDamageInfo();
          info.SetRadius(0.f);
          mgr.ApplyDamage(uid, tmpId, proj->GetOwnerId(), info,
                          CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
        }
      }
    }
    break;
  }
  case EScriptObjectMessage::Touched: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      if (colAct->GetMaterialFilter().Passes({EMaterialTypes::Platform})) {
        if (TCastToConstPtr<CScriptPlatform> plat = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
          mgr.ApplyDamage(GetUniqueId(), plat->GetUniqueId(), GetUniqueId(),
                          CDamageInfo(CWeaponMode(EWeaponType::AI), 1.f + plat->GetHealthInfo(mgr)->GetHP(), 0.f, 1.f),
                          CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
        }

        if (mgr.GetPlayer().GetUniqueId() == colAct->GetLastTouchedObject()) {
          mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), xc8c_,
                          CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
          x420_curDamageRemTime = x424_damageWaitTime;
        }
      }
    }
    break;
  }
  default:
    break;
  }
}

void CRidley::Think(float dt, CStateManager& mgr) {
  if (!GetActive())
    return;

  sub802560d0(dt);
  CPatterned::Think(dt, mgr);
  sub802563a8(dt);
  x984_bodyCollision->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  x980_tailCollision->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  xb20_ = std::max(0.f, xb20_ - dt);
  xb24_ = std::max(0.f, xb24_ - dt);
  xcc8_ = std::max(0.f, xcc8_ - dt);
  sub80256b14(dt, mgr);
  sub80256624(dt, mgr);
  xb2c_.Update(dt);
}
void CRidley::PreRender(CStateManager& mgr, const zeus::CFrustum& frustum) {
  CPatterned::PreRender(mgr, frustum);
  xb2c_.PreRender(mgr, *GetModelData()->GetAnimationData(), x34_transform, GetModelData()->GetScale(),
                  *x450_bodyController);
  u32 matSet = xc74_;
  if (xcc8_ > 0.f) {
    u32 numMaterialSets = GetModelData()->GetNumMaterialSets();
    u32 tmp = (30.f * xcc8_);
    matSet = tmp - (tmp / numMaterialSets) * numMaterialSets;
  }

  CPlayerState::EPlayerVisor r28 = mgr.GetPlayerState()->GetActiveVisor(mgr);
  bool atLastMat = GetModelData()->GetNumMaterialSets() == (matSet + 1);
  if (r28 == CPlayerState::EPlayerVisor::Thermal && atLastMat)
    xb4_drawFlags.x2_flags |= 0x40;
  else
    xb4_drawFlags.x2_flags &= 0x40;
  xb4_drawFlags.x1_matSetIdx = matSet;

  if (xa33_27_) {
    float zDiff = std::max(0.f, GetTranslation().z() - xa84_.origin.z());
    xccc_ = 1.f + zeus::clamp(0.f, zDiff - 20.f, 1.f);
    zeus::CVector3f extents = x9c_renderBounds.max - x9c_renderBounds.min;
    zeus::CVector3f something = 0.5f * (xccc_ * extents - extents);
    zeus::CAABox box(GetRenderBounds().min - something, GetRenderBounds().max + something);
    if ((r28 != CPlayerState::EPlayerVisor::Combat && r28 != CPlayerState::EPlayerVisor::Scan) ||
        !xac4_.intersects(box) || zDiff <= -10.f) {
      xd10_->Unset_X80();
    } else {
      xd10_->RenderShadowBuffer(mgr, *GetModelData(), GetTransform(), atLastMat, {}, xccc_, 10.f + zDiff);
    }
  } else {
    xd10_->Unset_X80();
  }
}

void CRidley::Render(CStateManager& mgr) {
  zeus::CColor multiplyColor = zeus::skBlack;
  if (xb24_ > 0.f) {
    multiplyColor = zeus::CColor::lerp(zeus::skWhite, x430_damageColor, xb24_ / 0.33f);
  }
  g_Renderer->SetWorldLightMultiplyColor(multiplyColor);

  const zeus::CTransform xf = GetLctrTransform(xa30_breastPlateSegId);

  if (xa31_25_) {
    if (xb24_ > 0.f) {
      x9e4_.Render(mgr, xf, GetActorLights(),
                   CModelFlags(2, 0, 3, zeus::CColor::lerp(zeus::skBlack, x430_damageColor, xb24_ / 0.33f)));
    } else {
      x9e4_.Render(mgr, xf, GetActorLights(), CModelFlags(0, 0, 3, zeus::skWhite));
    }
  } else if (xa31_24_) {
    if (xb20_ > 0.f) {
      x998_.Render(mgr, xf, GetActorLights(),
                   CModelFlags(2, 0, 3, zeus::CColor::lerp(zeus::skBlack, x430_damageColor, xb20_ / 0.33f)));
    } else {
      x998_.Render(mgr, xf, GetActorLights(), CModelFlags(0, 0, 3, zeus::skWhite));
    }
  }

  CPatterned::Render(mgr);
}

void CRidley::AddToRenderer(const zeus::CFrustum& frustum, CStateManager& mgr) {
  CPatterned::AddToRenderer(frustum, mgr);
  if (xce0_ && frustum.aabbFrustumTest(*xce0_->GetBounds())) {
    g_Renderer->AddParticleGen(*xce0_);
  }
}
void CRidley::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type, float dt) {
  switch (type) {
  case EUserEventType::Projectile: {
    if (xc64_aiStage != 2)
      return;

    if (!xc14_.Token().IsLoaded()) {
      xc14_.Token().GetObj();
      return;
    }

    if (!mgr.CanCreateProjectile(GetUniqueId(), EWeaponType::AI, 9))
      return;
    zeus::CTransform xf = GetLctrTransform(xa30_breastPlateSegId) * zeus::CTransform::RotateX(zeus::degToRad(-90.f));
    xf = xf * zeus::CTransform::RotateY(
                  std::atan2(mgr.GetActiveRandom()->Range(-1.f, 1.f), mgr.GetActiveRandom()->Range(-1.f, 1.f)));
    xf.origin = xf * zeus::CVector3f(0.f, 1.f, 1.f);
    CEnergyProjectile* proj =
        new CEnergyProjectile(true, xc14_.Token(), EWeaponType::AI, xf, EMaterialTypes::Character, xc14_.GetDamage(),
                              mgr.AllocateUniqueId(), GetAreaIdAlways(), GetUniqueId(), mgr.GetPlayer().GetUniqueId(),
                              EProjectileAttrib::None, false, zeus::skOne3f, {}, -1, false);
    mgr.AddObject(proj);
    proj->SetCameraShake(x568_data.xcc_);
    return;
  }
  case EUserEventType::EggLay: {
    if (xa32_24_) {
      if (!xc3c_.Token().IsLoaded()) {
        xc3c_.Token().GetObj();
        break;
      }

      if (!mgr.CanCreateProjectile(GetUniqueId(), EWeaponType::AI, 6))
        break;

      zeus::CVector3f vec =
          zeus::CVector3f(mgr.GetActiveRandom()->Range(-1.f, 1.f), 1.f, mgr.GetActiveRandom()->Range(-1.f, 1.f));
      vec = GetLctrTransform(xa30_breastPlateSegId) * vec;

      CEnergyProjectile* proj = new CEnergyProjectile(
          true, xc3c_.Token(), EWeaponType::AI, zeus::lookAt(vec + mgr.GetPlayer().GetTranslation(), vec),
          EMaterialTypes::Character, xc3c_.GetDamage(), mgr.AllocateUniqueId(), GetAreaIdAlways(), GetUniqueId(),
          mgr.GetPlayer().GetUniqueId(), EProjectileAttrib::None, false, zeus::skOne3f, {}, -1, false);

      mgr.AddObject(proj);
      proj->SetCameraShake(x568_data.xcc_);
    } else if (xc64_aiStage == 3) {
      xa31_24_ = false;
    }
    break;
  }
  case EUserEventType::DamageOn: {
    if (xc64_aiStage == 3) {
      sub8025784c(mgr);
    } else if (xc64_aiStage == 2)
      xa33_28_ = false;

    break;
  }
  case EUserEventType::DamageOff: {
    if (xc64_aiStage == 3) {
      sub80257650(mgr);
    } else if (xc64_aiStage == 2 && !xa33_31_)
      xa33_28_ = true;

    break;
  }
  case EUserEventType::Landing: {
    SetVelocityWR(zeus::skDown);
    xaec_.zeroOut();
    xaf8_.zeroOut();
    break;
  }
  case EUserEventType::FadeOut: {
    xc74_ = std::min(GetModelData()->GetNumMaterialSets(), xc74_ + 1);
    return;
  }
  case EUserEventType::ScreenShake: {
    if ((mgr.GetPlayer().GetTranslation() - GetTranslation()).magnitude() >= x568_data.x388_)
      break;
    mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer().GetUniqueId(), GetUniqueId(), x568_data.x298_,
                    CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
    break;
  }
  case EUserEventType::BeginAction: {
    if (xa32_25_ && !xa31_29_)
      FirePlasma(mgr);

    if (!xa31_31_ || !xa32_26_)
      break;

    xbf0_ = xa84_.basis[0];
    zeus::CVector3f ourPos = GetTranslation();
    if ((ourPos - xa84_.origin).dot(xbf0_) >= 30.f) {
      xbf0_ *= zeus::CVector3f(-1.f);
    }
    xbfc_ = xbf0_;
    xbe4_ = xa84_.origin - xabc_ * xbf0_;
    xbe4_ *= zeus::CVector3f((mgr.GetPlayer().GetTranslation() - xa84_.origin).dot(xa84_.basis[1]));
    break;
  }
  case EUserEventType::EndAction: {
    if (xa31_29_)
      ResetPlasmaProjectile(mgr, false);
    break;
  }
  case EUserEventType::IkLock: {
    xa32_26_ = true;
    break;
  }
  case EUserEventType::IkRelease: {
    xa32_26_ = false;
    break;
  }
  case EUserEventType::BreakLockOn: {
    if (x400_25_alive) {
      RemoveMaterial(EMaterialTypes::Target, EMaterialTypes::Orbit, mgr);
      mgr.GetPlayer().SetOrbitRequestForTarget(GetUniqueId(), CPlayer::EPlayerOrbitRequest::ActivateOrbitSource, mgr);
      return;
    }
    break;
  }
  case EUserEventType::SoundPlay: {
    if (xa32_25_)
      break;

    xcac_ = CSfxManager::AddEmitter({GetTranslation(), {}, 1000.f, 0.1f, 1, x568_data.x294_, 127, 63, false, 127}, true,
                                    -1, false, kInvalidAreaId);
    break;
  }
  default:
    break;
  }

  CPatterned::DoUserAnimEvent(mgr, node, type, dt);
}

void CRidley::sub8025784c(CStateManager& mgr) {
  for (size_t i = 0; i < x984_bodyCollision->GetNumCollisionActors(); ++i) {
    const auto& colDesc = x984_bodyCollision->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(colDesc.GetCollisionActorId())) {
      colAct->SetDamageVulnerability(i == 3 ? skIceWeakness : CDamageVulnerability::ImmuneVulnerabilty());
      colAct->HealthInfo(mgr)->SetHP(1000.f);
      colAct->CreateShadow(true);
    }
  }

  xa32_29_ = true;
  xa31_27_ = false;
}

void CRidley::sub80255d58(urde::CStateManager& mgr) {
  xb04_ = skSomeRidleyStruct[xcb0_][xcb4_].x4_ < mgr.GetActiveRandom()->Range(0.f, 100.f)
              ? skSomeRidleyStruct[xcb0_][xcb4_].x8_
              : skSomeRidleyStruct[xcb0_][xcb4_].x0_;
  if (xb04_ == -1) {
    xcb4_ = 0;
    xb04_ = skSomeRidleyStruct[xcb0_][xcb4_].x4_ < mgr.GetActiveRandom()->Range(0.f, 100.f)
                ? skSomeRidleyStruct[xcb0_][xcb4_].x8_
                : skSomeRidleyStruct[xcb0_][xcb4_].x0_;
  }

  ++xcb4_;
  xcc4_ = 1;
}

void CRidley::sub80257744(urde::CStateManager& mgr) {
  for (size_t i = 0; i < x984_bodyCollision->GetNumCollisionActors(); ++i) {
    const auto& colDesc = x984_bodyCollision->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(colDesc.GetCollisionActorId())) {
      colAct->SetDamageVulnerability(i == 2 ? skDirectNormal : CDamageVulnerability::ImmuneVulnerabilty());
      colAct->HealthInfo(mgr)->SetHP(1000.f);
      colAct->CreateShadow(true);
    }
  }

  xa32_29_ = false;
  xa31_27_ = true;
}

void CRidley::FirePlasma(urde::CStateManager& mgr) {
  if (xb64_plasmaProjectile == kInvalidUniqueId) {
    xb64_plasmaProjectile = mgr.AllocateUniqueId();
    mgr.AddObject(new CPlasmaProjectile(xb68_.Token(), ""sv, EWeaponType::AI, x568_data.x64_, {},
                                        EMaterialTypes::Character, xb68_.GetDamage(), xb64_plasmaProjectile,
                                        GetAreaIdAlways(), GetUniqueId(), CPlasmaProjectile::PlayerEffectResoures(),
                                        false, EProjectileAttrib::KeepInCinematic));
  }

  if (CPlasmaProjectile* proj = static_cast<CPlasmaProjectile*>(mgr.ObjectById(xb64_plasmaProjectile))) {
    proj->Fire(GetLctrTransform(xb91_mouthSegId), mgr, false);
    if (!xca8_) {
      xca8_ = CSfxManager::AddEmitter({GetTranslation(), {}, 1000.f, 0.1f, 1, x568_data.xa8_, 127, 63, false, 127},
                                      true, -1, true, -1);
    }
  }
}

void CRidley::sub80257650(CStateManager& mgr) {
  for (size_t i = 0; i < x984_bodyCollision->GetNumCollisionActors(); ++i) {
    const auto& colDesc = x984_bodyCollision->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(colDesc.GetCollisionActorId())) {
      colAct->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
      colAct->HealthInfo(mgr)->SetHP(1000.f);
      colAct->CreateShadow(true);
    }
  }

  xa32_29_ = false;
  xa31_27_ = false;
}

void CRidley::sub80256914(float f31, bool r4) {
  if (xce0_) {
    xce0_->SetParticleEmission(true);
  }

  xd08_ = r4 ? 2.f * f31 : f31;
  xa32_30_ = r4;
  if (!xd0c_) {
    xd0c_ = CSfxManager::AddEmitter(
        CAudioSys::C3DEmitterParmData{GetTranslation(), zeus::skZero3f, 500.f, 0.1f, 1, 0, 127, 63, false, 127}, true,
        127, true, kInvalidAreaId);
  }
}

void CRidley::sub802560d0(float dt) {
  if (!IsAlive()) {
    xaec_.zeroOut();
  } else if (xaec_.isMagnitudeSafe()) {
    const float mag = xaec_.magnitude();
    float magScale = 0.2f;
    if (xaf8_.magSquared() == 0.f)
      magScale *= 3.f;
    xaec_ = -((zeus::clamp(0.f, dt * (magScale * mag), 0.5f) * mag) - mag) * ((1.f / mag) * xaec_);
    ApplyImpulseWR(GetMass() * xaec_, {});
  }
  xaf8_.zeroOut();
}

void CRidley::sub802563a8(float dt) {
  if (xc64_aiStage == 3 && !x328_25_verticalMovement) {
    SetTranslation({GetTranslation().x(), GetTranslation().y(), xa84_.origin.z()});
    zeus::CVector3f posDiff = GetTranslation() - xa84_.origin;
    float mag = posDiff.magnitude();
    posDiff *= zeus::CVector3f(1.f / mag);
    if (xab4_ + -6.f * zeus::clamp(-1.f, posDiff.dot(xa84_.basis[1]), 0.f) < mag)
      if (GetVelocity().dot(posDiff) > 0.f)
        Stop();

    MoveToInOneFrameWR(GetTranslation() - posDiff, dt);
  }
}

void CRidley::sub80256b14(float dt, CStateManager& mgr) {
  if (CPlasmaProjectile* proj = static_cast<CPlasmaProjectile*>(mgr.ObjectById(xb64_plasmaProjectile))) {
    if (!proj->GetActive())
      return;

    zeus::CTransform mouthXf = GetLctrTransform(xb91_mouthSegId);
    if (xc64_aiStage == 3) {
      proj->UpdateFx(mouthXf, dt, mgr);
    } else {
      zeus::CTransform xf = zeus::lookAt(xf.origin, xbe4_);
      proj->UpdateFx(xf, dt, mgr);
      float d;
      if (xbf0_.cross(zeus::skUp).dot(mgr.GetPlayer().GetTranslation() + zeus::skUp) <= 0.f)
        d = xc10_;
      else
        d = -xc10_;

      zeus::CQuaternion quat;
      quat.rotateZ(zeus::degToRad(dt * d));
      zeus::CVector3f vec = quat.transform(xbf0_);
      float dist = xbfc_.dot(vec);
      if (dist > 0.5f || xbfc_.dot(xbfc_) < dist) {
        xbf0_ = vec;
      }
    }
  }

  if (xca8_) {
    CSfxManager::UpdateEmitter(xca8_, GetTranslation(), {}, 1.f);
  }
}

void CRidley::sub80256624(float dt, CStateManager& mgr) {
  if (xce0_) {
    xce0_->SetGlobalOrientation(GetTransform().getRotation());
    xce0_->SetGlobalTranslation(GetTranslation());
    xce0_->SetGlobalScale(GetModelData()->GetScale());

    if (xce0_->GetParticleEmission()) {
      xd08_ -= dt;

      if (xd08_ <= 0.f) {
        xce0_->SetParticleEmission(false);
        if (xd0c_) {
          CSfxManager::RemoveEmitter(xd0c_);
          xd0c_.reset();
        }
      } else {
        xce0_->SetOverrideIPos(
            GetModelData()->GetAnimationData()->GetLocatorTransform(xa30_breastPlateSegId, nullptr).origin);
        s32 min = 0;
        s32 max = xce4_wingBoneIds.size() - 1;
        if (xa32_30_)
          max = xce4_wingBoneIds.size() - 6;
        else
          min = xce4_wingBoneIds.size() - 12;
        zeus::CTransform xf = GetModelData()->GetAnimationData()->GetLocatorTransform(
            xce4_wingBoneIds[mgr.GetActiveRandom()->Range(min, max)], nullptr);
        xce0_->SetOverrideFPos(xf.origin);
        xce0_->ForceParticleCreation(1);
      }
    }
    xce0_->Update(dt);
  } else if (xd0c_) {
    CSfxManager::RemoveEmitter(xd0c_);
    xd0c_.reset();
  }

  if (xd0c_)
    CSfxManager::UpdateEmitter(xd0c_, GetTranslation(), {}, 1.f);
}

void CRidley::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Activate)
    return;
  sub80255d58(mgr);
  xa32_27_ = false;
  xa33_26_ = true;
}
void CRidley::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Activate)
    return;
  mgr.SetBossParams(kInvalidUniqueId, 0.f, 0);
}
void CRidley::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Activate)
    return;

  xa34_26_ = false;
  zeus::CVector3f vec;
  vec.x() = 0.f;
  vec.y() = xabc_;
  vec.z() = xac0_ - xadc_;
  SetTranslation(xa84_ * vec);
  x450_bodyController->GetCommandMgr().DeliverCmd(CBodyStateCmd(EBodyStateCmd::NextState));
}

void CRidley::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    xbe4_ = (GetTranslation() + zeus::skUp) - (8.f * GetTransform().basis[0]);
    xbfc_ = xbf0_ = GetTransform().basis[0];
    xc08_ = xc0c_ = 0.f;
    xc10_ = 240.f;
    x32c_animState = EAnimState::Ready;
    xa32_25_ = true;
    --xcc4_;
  } else if (msg == EStateMsg::Update) {
    if (!xa31_29_) {
      FacePlayer(arg, mgr);
    } else {
      xc0c_ = std::min(0.5f, 0.5f * arg + xc0c_);
      xc08_ = arg * xc0c_ + xc08_;
      xbe4_ += xc08_ * xbf0_;
    }
    TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, 0);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    ResetPlasmaProjectile(mgr, true);
    xa32_25_ = false;
  }
}

void CRidley::LoopedAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    if (xa32_31_) {
      xa33_24_ = false;
      xbc4_ = GetTranslation();
      xbd0_ = GetTransform().basis[1];
    } else {
      xa33_24_ = (GetTranslation() - xa84_.origin).magSquared() < 0.f;

      for (const auto& conn : GetConnectionList()) {
        if (conn.x0_state != EScriptObjectState::Attack || conn.x4_msg != EScriptObjectMessage::Follow)
          continue;

        TUniqueId uid = mgr.GetIdForScript(conn.x8_objId);
        if (uid == kInvalidUniqueId)
          continue;

        if (TCastToConstPtr<CScriptWaypoint> wp = mgr.GetObjectById(uid)) {
          zeus::CVector3f wpPos = wp->GetTranslation();
          const float mag = xa84_.basis[0].dot(wpPos - xa84_.origin);
          if ((xa33_24_ && mag > 0.f) || (!xa33_24_ && mag <= 0.f)) {
            xbc4_ = wp->GetTranslation();
            xbd0_ = wp->GetTransform().basis[1];
            x2ec_reflectedDestPos = GetTranslation();
            break;
          }
        }
      }
    }

    xbdc_ = 0.f;
    xbe0_ = 800.f;
    xa31_30_ = false;
    xa31_31_ = true;
    xc10_ = 120.f;
    xa32_24_ = true;
    xcc0_ = skSomeStruct[xcb0_].x1c_;
    xc68_ = GetModelData()->GetScale();
    SetSphereCollisionRadius(2.f, mgr);
  } else if (msg == EStateMsg::Update) {
    zeus::CVector3f diffVec = GetTranslation() - xa84_.origin;
    float fVar22 = zeus::clamp(0.f, diffVec.magnitude() - 100.f, 250.f) / 250.f;
    float local_d0 = zeus::clamp(0.3f, 1.f - (0.7f * -(2.f * fVar22 - 3.f) * (fVar22 * fVar22)), 1.f);
    GetModelData()->SetScale(local_d0 * xc68_);
    x55c_moveScale = zeus::CVector3f((1.f / local_d0));
    x9e4_.SetScale(local_d0 * xc68_);
    if (x330_stateMachineState.GetTime() > 1.f && skSomeStruct[xcb0_].x20_ != 0) {
      xa32_25_ = true;
    }

    if (xa32_25_ && xa31_29_) {
      xbe4_ += (90.f * arg) * xbf0_;
    }

    if (xa32_24_ && xcac_ != 0) {
      CSfxManager::UpdateEmitter(xca8_, GetTranslation(), {}, 127);
    }
    if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Step)
      return;

    if (!xa31_30_) {
      zeus::CVector3f local_54 = xbc4_ - GetTranslation();
      float mag = local_54.magnitude();
      if (mag <= 2.f || local_54.dot(xbc4_ - x2ec_reflectedDestPos) <= 0.f) {
        pas::EStepDirection dir = pas::EStepDirection::Right;
        if (xa32_31_)
          dir = pas::EStepDirection::Left;
        if (xcc0_ == 3)
          dir = pas::EStepDirection::Up;
        else if (xcc0_ == 2)
          dir = pas::EStepDirection::Forward;

        pas::EStepType type = pas::EStepType::Normal;
        if (xa33_24_)
          type = pas::EStepType::BreakDodge;
        x450_bodyController->GetCommandMgr().DeliverCmd(CBCStepCmd(dir, type));
        xa31_30_ = true;
      } else {
        local_54 *= zeus::CVector3f(1.f / mag);
        xbdc_ += zeus::clamp(-100.f * arg, xbe0_ - xbdc_, 100.f * arg);
        sub80255fe8(xbdc_, arg, local_54);
        x450_bodyController->FaceDirection(xbd0_, arg * zeus::clamp(1.f, 10.f / mag, 10.f));
      }
    } else {
      x330_stateMachineState.SetCodeTrigger();
    }
  } else if (msg == EStateMsg::Deactivate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    if (xa32_25_) {
      ResetPlasmaProjectile(mgr, 1);
    }

    if (xa32_24_) {
      xcac_.reset();
    }
    xa32_24_ = false;
    xa32_25_ = false;
    GetModelData()->SetScale(xc68_);
    xa31_31_ = false;
    x55c_moveScale.splat(1.f);
    SetupCollisionActors(mgr);
    SetSphereCollisionRadius(0.5f, mgr);
    xa32_31_ = false;
  }
}

void CRidley::ResetPlasmaProjectile(CStateManager& mgr, bool b1) {
  if (CPlasmaProjectile* ent = static_cast<CPlasmaProjectile*>(mgr.ObjectById(xb64_plasmaProjectile))) {
    ent->ResetBeam(mgr, b1);
    xa31_29_ = false;
  }

  if (xca8_) {
    CSfxManager::RemoveEmitter(xca8_);
    xca8_.reset();
  }
}

void CRidley::SetSphereCollisionRadius(float f1, CStateManager& mgr) {
  for (size_t i = 0; i < x984_bodyCollision->GetNumCollisionActors(); ++i) {
    const auto& colDesc = x984_bodyCollision->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(colDesc.GetCollisionActorId())) {
      colAct->SetSphereRadius(f1 * colAct->GetSphereRadius());
    }
  }
}

void CRidley::JumpBack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    SetMomentumWR(zeus::skZero3f);
    x328_25_verticalMovement = true;
    SetDestPos(xa84_.origin + (xabc_ * xa84_.basis[1]) + zeus::CVector3f(0.f, 0.f, xac0_));
    RemoveMaterial(EMaterialTypes::Solid, mgr);
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    CMaterialList exclude = GetMaterialFilter().GetExcludeList();
    include.Remove(EMaterialTypes::Solid);
    exclude.Add(EMaterialTypes::Solid);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
    xa32_26_ = false;
    xc7c_ =
        zeus::CVector2f::getAngleDiff((x2e0_destPos - GetTranslation()).toVec2f(), GetTransform().basis[1].toVec2f());

    if (GetTransform().basis[0].dot(x2e0_destPos - GetTranslation()) > 0.f)
      xc7c_ = -xc7c_;
    xc78_ = 0.f;

  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Generate, &CPatterned::TryGenerate, 4);

    if (x32c_animState == EAnimState::Repeat) {
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
      if (xc78_ == 0.f) {
        CCharAnimTime ikLock =
            GetModelData()->GetAnimationData()->GetTimeOfUserEvent(EUserEventType::IkLock, CCharAnimTime::Infinity());
        CCharAnimTime ikRelease = GetModelData()->GetAnimationData()->GetTimeOfUserEvent(EUserEventType::IkRelease,
                                                                                         CCharAnimTime::Infinity());
        if (ikLock != CCharAnimTime::Infinity() && ikRelease != CCharAnimTime::Infinity()) {
          xc78_ = ikRelease.GetSeconds() - ikLock.GetSeconds();
        }
      }

      if (xa32_26_) {
        zeus::CQuaternion q;
        q.rotateZ((xc7c_ * arg) / xc78_);
        RotateInOneFrameOR(q, arg);
      }
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}

void CRidley::DoubleSnap(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x402_28_isMakingBigStrike = x568_data.x3cc_ > 0.f;
    x504_damageDur = x568_data.x3cc_;
    xc8c_ = x568_data.x3b0_;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 2);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x402_28_isMakingBigStrike = false;
    xc8c_ = GetContactDamage();
  }
}
void CRidley::CoverAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    x402_28_isMakingBigStrike = x568_data.x3ec_ > 0.f;
    x504_damageDur = x568_data.x3ec_;
    xc8c_ = x568_data.x3d0_;
    sub80256580();
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 1);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x402_28_isMakingBigStrike = false;
    x504_damageDur = 0.f;
    xc8c_ = GetContactDamage();
  }
}

void CRidley::Crouch(urde::CStateManager& mgr, urde::EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    SetMomentumWR(GetGravityConstant() * zeus::skDown);
    if (xc64_aiStage == 3) {
      sub80253710(mgr);
    }
  } else if (msg == EStateMsg::Update) {
    if (x450_bodyController->GetBodyStateInfo().GetCurrentStateId() == pas::EAnimationState::Turn)
      return;

    zeus::CVector3f faceDir = (mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized();
    if (faceDir.dot(GetTransform().basis[1]) < 0.9f)
      x450_bodyController->GetCommandMgr().DeliverCmd(CBCLocomotionCmd(zeus::skZero3f, faceDir, 1.f));
  }
}
void CRidley::FadeOut(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    for (const auto& effect : skWingEffects) {
      GetModelData()->GetAnimationData()->SetParticleEffectState(effect, false, mgr);
    }
    if (!xa34_24_)
      xa34_24_ = true;

    xb68_.SetDamage(x568_data.x3fc_);
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::KnockBack, &CPatterned::TryKnockBack, 5);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    xcbc_ = 0.6667f * x568_data.x3c_;
  }
}
void CRidley::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Taunt, &CPatterned::TryTaunt, 3);
    FacePlayer(arg, mgr);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}
void CRidley::Flee(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    zeus::CVector3f destPos =
        xa84_.origin.toVec2f() + (40.f + xabc_) * (GetTranslation().toVec2f() - xa84_.origin.toVec2f()).normalized();
    destPos.z() = xac0_ + xa84_.origin.z();
    SetDestPos(destPos);
  } else if (msg == EStateMsg::Update) {
    sub80255fe8(50.f, arg, (x2e0_destPos - GetTranslation()).normalized());
    x450_bodyController->FaceDirection((x2e0_destPos - GetTranslation()).normalized(), arg);
  }
}
void CRidley::Lurk(urde::CStateManager& mgr, urde::EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    if (!xa33_25_) {
      zeus::CVector3f vec = GetTranslation() - xa84_.origin;
      vec.z() = 0.f;
      arg = zeus::CVector3f::getAngleDiff(xa84_.basis[1], vec);
      if (vec.dot(xa84_.basis[0]) < 0.f)
        arg = -arg;

      if (std::fabs(mgr.GetActiveRandom()->Range(-1.2566371f, 1.2566371f) - arg) >= 0.39269909f) {
        /* CodeWarrior was drunk */
      }
    } else {
      x984_bodyCollision->SetActive(mgr, true);
      x980_tailCollision->SetActive(mgr, true);
    }
    xa33_25_ = false;
    float f30 = xac0_ - xadc_;
    float f31 = xabc_ * std::cos(0.f);
    float f0 = xabc_ * std::sin(0.f);
    zeus::CVector3f destPos = xa84_ * zeus::CVector3f(f0, f31, f30);
    SetDestPos(destPos);
    zeus::CVector3f vec = GetTransform().basis[1].toVec2f().normalized();
    zeus::CTransform xf(vec.cross(zeus::skUp), vec, zeus::skUp, GetTranslation());
    SetTransform(xf);
    xa33_27_ = false;
    xa34_26_ = false;
  } else if (msg == EStateMsg::Update) {
    sub80255fe8(50.f, arg, (x2e0_destPos - GetTranslation()).normalized());
    FacePlayer(10.f * arg, mgr);
  }
}
void CRidley::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    --xcc4_;
    xa33_31_ = true;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, 2);
    FacePlayer(arg, mgr);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    xa33_28_ = false;
    xa33_31_ = false;
  }
}
void CRidley::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    sub80257744(mgr);
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::KnockBack, &CPatterned::TryKnockBack, 3);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    sub80257650(mgr);
    xa32_28_shotAt = false;
  }
}
void CRidley::Hurled(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg != EStateMsg::Activate)
    return;

  sub80257650(mgr);
  x450_bodyController->GetCommandMgr().DeliverCmd(CBCKnockBackCmd(GetTransform().basis[1], pas::ESeverity::Four));
}

void CRidley::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    sub8025784c(mgr);
    x32c_animState = EAnimState::Ready;
    xa32_25_ = true;
    sub80256580();
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::ProjectileAttack, &CPatterned::TryProjectileAttack, 0);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    ResetPlasmaProjectile(mgr, true);
    sub80257650(mgr);
    xa32_25_ = false;
  }
}
void CRidley::Jump(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    xc88_ = (mgr.GetPlayer().GetTranslation() - GetTranslation()).magnitude() <= xae4_ ? 4 : 5;
    x402_28_isMakingBigStrike = x568_data.x3ac_ > 0.f;
    xc8c_ = x568_data.x390_;
    sub80256580();
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, xc88_);
    if (x330_stateMachineState.GetTime() < 1.75f)
      FacePlayer(arg, mgr);
  } else if (msg == EStateMsg::Deactivate) {
    AddMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
    x402_28_isMakingBigStrike = false;
    x504_damageDur = 0.f;
    xc8c_ = GetContactDamage();
    x32c_animState = EAnimState::NotReady;
    xa34_25_ = false;
  }
}
void CRidley::Explode(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    xa32_27_ = false;
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::KnockBack, &CPatterned::TryKnockBack, 2);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}
void CRidley::Dodge(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    xc84_ = 2;
    float dist = zeus::clamp(
        0.f, zeus::CVector3f(GetTranslation().toVec2f() - xa84_.origin.toVec2f()).normalized().dot(xa84_.basis[0]),
        1.f);
    if (dist < mgr.GetActiveRandom()->Float())
      xc84_ = 3;
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryDodge, xc84_);
    if (x32c_animState == EAnimState::Over) {
      zeus::CVector3f vec = zeus::CVector3f(GetTranslation().toVec2f() - xa84_.origin.toVec2f()).normalized();
      zeus::CVector3f someVec(((xa84_.origin.x() + xabc_) * vec.x()) - GetTranslation().x(),
                              ((xa84_.origin.y() + xabc_) * vec.y()) - GetTranslation().y(),
                              ((xa84_.origin.z() + xac0_) - GetTranslation().z()));
      if (someVec.magnitude() > 1.f)
        someVec.normalize();

      sub80255fe8(10.f, arg, someVec);
    } else {
      x450_bodyController->FaceDirection((xa84_.origin - GetTranslation()).normalized(), arg);
    }
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}
void CRidley::Retreat(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryDodge, 5);
  } else {
    x32c_animState = EAnimState::NotReady;
  }
}
void CRidley::Approach(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    zeus::CVector3f direction = (GetTranslation() - xa84_.origin).normalized();
    zeus::CVector3f destPos(xa84_.origin.x() + xab4_ * direction.x(), xa84_.origin.y() + xab4_ * direction.y(),
                            (xae8_ + xa84_.origin.z()) - 1.f);
    SetDestPos(destPos);
    xa33_26_ = false;
    if (xc64_aiStage == 3 && !xa34_24_) {
      xa34_24_ = true;
      SendScriptMsgs(EScriptObjectState::CameraPath, mgr, EScriptObjectMessage::None);
    }
  } else if (msg == EStateMsg::Update) {
    sub80255fe8(50.f, arg, (x2e0_destPos - GetTranslation()).normalized());
    FacePlayer(arg, mgr);
  }
}

void CRidley::Enraged(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
    AddMaterial(EMaterialTypes::Orbit, EMaterialTypes::Target, mgr);
    zeus::CVector3f vec = (mgr.GetPlayer().GetTranslation().toVec2f() - GetTranslation().toVec2f()).normalized();
    mgr.AddObject(new CExplosion(
        x98c_, mgr.AllocateUniqueId(), true, CEntityInfo(GetAreaIdAlways(), NullConnectionList), ""sv,
        zeus::CTransform(vec.cross(zeus::skUp), vec, zeus::skUp, mgr.GetPlayer().GetTranslation() - (20.f * vec)), 0,
        zeus::skOne3f, zeus::skWhite));
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Step, &CPatterned::TryDodge, 4);
    FacePlayer(arg, mgr);
  } else {
    x32c_animState = EAnimState::NotReady;
    xa33_24_ = true;
  }
}

void CRidley::SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    x32c_animState = EAnimState::Ready;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::Taunt, &CPatterned::TryTaunt, 0);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
  }
}
void CRidley::Land(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    zeus::CVector3f diff = zeus::CVector3f(mgr.GetPlayer().GetTranslation().x() - xa84_.origin.x(),
                                           mgr.GetPlayer().GetTranslation().y() - xa84_.origin.y(), 0.f);
    SetDestPos(xa84_.origin + std::min(5.f + xab4_, diff.magnitude()) * diff.normalized());
    x32c_animState = EAnimState::Ready;
    AddMaterial(EMaterialTypes::Solid, mgr);
    CMaterialList exclude = GetMaterialFilter().GetExcludeList();
    exclude.Remove(EMaterialTypes::Solid);
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    include.Add(EMaterialTypes::Solid);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));

    if (xc64_aiStage == 3)
      AddMaterial(EMaterialTypes::GroundCollider, EMaterialTypes::Solid, mgr);

    x402_28_isMakingBigStrike = x568_data.x38c_ > 0.f;
    x504_damageDur = x568_data.x38c_;
  } else if (msg == EStateMsg::Update) {
    TryCommand(mgr, pas::EAnimationState::MeleeAttack, &CPatterned::TryMeleeAttack, 8);

    if (x32c_animState == EAnimState::Repeat) {
      x450_bodyController->SetLocomotionType(pas::ELocomotionType::Combat);
    }
    sub80255e5c(mgr);
  } else if (msg == EStateMsg::Deactivate) {
    x32c_animState = EAnimState::NotReady;
    x402_28_isMakingBigStrike = false;
    x504_damageDur = 0.f;
    xa33_30_ = false;
    xa33_29_doStrafe = false;
    if (mgr.GetActiveRandom()->Range(0.f, 100.f) < 50.f) {
      if ((mgr.GetPlayer().GetTranslation() - GetTranslation()).magSquared() <= 0.f)
        xa33_29_doStrafe = true;
      else
        xa33_30_ = true;
    }
  }
}
bool CRidley::Attacked(CStateManager& mgr, float arg) { return xa31_24_ && xa31_26_; }
bool CRidley::TooClose(CStateManager& mgr, float arg) {
  if (xb0c_ == 4)
    return true;

  if (xb08_ == 4)
    return false;

  zeus::CVector3f diff = mgr.GetPlayer().GetTranslation() - GetTranslation();
  if (diff.magnitude() < x2fc_minAttackRange && 0.7f * diff.magnitude() < diff.dot(GetTransform().basis[1])) {
    xb0c_ = 4;
    return true;
  }

  return false;
}
bool CRidley::InRange(CStateManager& mgr, float arg) {
  float mag = (GetTranslation() - x2e0_destPos).magnitude();
  return mag < 2.f;
}
bool CRidley::ShouldAttack(CStateManager& mgr, float arg) {
  return (xc64_aiStage == 3 && xb0c_ == 2) || (xc64_aiStage == 2 && xb04_ == 0);
}
bool CRidley::ShouldDoubleSnap(CStateManager& mgr, float arg) { return xa33_30_; }
bool CRidley::ShouldTurn(CStateManager& mgr, float arg) { return xb04_ == 5; }
bool CRidley::HitSomething(CStateManager& mgr, float arg) { return xa32_27_ || xc64_aiStage == 3; }
bool CRidley::AttackOver(CStateManager& mgr, float arg) { return xcc4_ == 0; }
bool CRidley::ShouldTaunt(CStateManager& mgr, float arg) {
  return (xc64_aiStage == 3 && xb0c_ == 1) || (xc64_aiStage == 2 && xb04_ == 4);
}
bool CRidley::ShouldFire(CStateManager& mgr, float arg) { return xc64_aiStage == 2 && xb04_ == 2; }
bool CRidley::ShouldDodge(CStateManager& mgr, float arg) { return xb04_ == 3; }
bool CRidley::ShouldRetreat(CStateManager& mgr, float arg) { return xa34_26_; }
bool CRidley::ShouldCrouch(CStateManager& mgr, float arg) { return xb04_ == 1; }
bool CRidley::ShouldMove(urde::CStateManager& mgr, float arg) {
  if (xb0c_ == 5) {
    xa34_25_ = true;
    return true;
  }

  zeus::CVector3f diffVec = mgr.GetPlayer().GetTranslation() - GetTranslation();
  float mag = diffVec.magnitude();
  if (x300_maxAttackRange < mag && 0.8f * mag < diffVec.dot(GetTransform().basis[1]) && sub80253960()) {
    xa34_25_ = true;
    xb0c_ = 5;
    return true;
  }

  return false;
}
bool CRidley::ShotAt(CStateManager& mgr, float arg) { return xa32_28_shotAt; }
bool CRidley::SetAIStage(CStateManager& mgr, float arg) {
  xc64_aiStage = arg;
  return true;
}
bool CRidley::AIStage(CStateManager& mgr, float arg) { return xc64_aiStage >= arg; }
bool CRidley::ShouldStrafe(CStateManager& mgr, float arg) { return xa33_29_doStrafe; }
bool CRidley::IsDizzy(CStateManager& mgr, float arg) {
  if (xb0c_ == 3)
    return true;

  if (xb08_ != 3) {
    zeus::CVector3f diff = mgr.GetPlayer().GetTranslation() - GetTranslation();
    if (diff.magnitude() < x300_maxAttackRange && diff.dot(GetTransform().basis[1]) < 0.f) {
      xb0c_ = 3;
      return true;
    }
  }

  return false;
}
void CRidley::sub80255fe8(float f1, float f2, const zeus::CVector3f& vec) {
  xaf8_ = (0.2f * (f1 * f1)) * vec;
  xaec_ += f2 * xaf8_;
  if (xaec_.magnitude() > f1) {
    xaec_ = f1 * xaec_.normalized();
  }
}
void CRidley::sub80255e5c(CStateManager& mgr) {
  zeus::CVector3f posDiff = mgr.GetPlayer().GetTranslation() - GetTranslation();
  if (posDiff.magnitude() < 8.f) {
    float mag = mgr.GetPlayer().GetMass() * (8.f - posDiff.magnitude());
    zeus::CVector3f impulse = mag * posDiff.toVec2f().normalized();
    mgr.GetPlayer().ApplyImpulseWR(impulse, {});
  }
}
void CRidley::FacePlayer(float arg, CStateManager& mgr) {
  x450_bodyController->FaceDirection((mgr.GetPlayer().GetTranslation() - GetTranslation()).normalized(), arg);
}

void CRidley::sub80253710(urde::CStateManager& mgr) {
  xb08_ = xb0c_;
  float fVar1 = 100.f * mgr.GetActiveRandom()->Float();
  float fVar6 = 0.f + skFloats[xb08_].x0_;
  if (fVar6 <= fVar1) {
    fVar6 += skFloats[xb08_].x4_;
    if (fVar6 <= fVar1) {
      fVar6 += skFloats[xb08_].x8_;
      if (fVar6 <= fVar1) {
        fVar6 += skFloats[xb08_].xc_;
        if (fVar6 <= fVar1) {
          fVar6 += skFloats[xb08_].x10_;
          if (fVar6 <= fVar1) {
            if (fVar1 < skFloats[xb08_].x14_) {
              xb0c_ = 5;
            }
          } else {
            xb0c_ = 4;
          }
        } else {
          xb0c_ = 3;
        }
      } else {
        xb0c_ = 2;
      }
    } else {
      xb0c_ = 1;
    }
  } else {
    xb0c_ = 0;
  }

  if (xb0c_ == 5 && !sub80253960()) {
    xb0c_ = 2;
  }

  zeus::CVector3f diff = mgr.GetPlayer().GetTranslation() - GetTranslation();
  float diffMag = diff.magnitude();
  float frontMag = (diff * (1.f / diffMag)).dot(GetTransform().basis[1]);
  if ((xb0c_ == 2 && frontMag < 0.5f) || (xb0c_ == 5 && frontMag < 0.8f))
    xb0c_ = 0;

  if (frontMag < 0.f && diffMag < x300_maxAttackRange && xb08_ != 3)
    xb0c_ = 3;
  if (frontMag > 0.f && diffMag < x2fc_minAttackRange && xb08_ != 4)
    xb0c_ = 4;
}
} // namespace MP1
} // namespace urde
