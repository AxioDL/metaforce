#include "Runtime/MP1/World/CRidley.hpp"
#include "Runtime/Character/CPASAnimParmData.hpp"
#include "Runtime/Collision/CCollisionActor.hpp"
#include "Runtime/Graphics/CBooRenderer.hpp"
#include "Runtime/Particle/CElementGen.hpp"
#include "Runtime/Particle/CParticleElectric.hpp"
#include "Runtime/Particle/CParticleSwoosh.hpp"
#include "Runtime/Weapon/CGameProjectile.hpp"
#include "Runtime/World/CPlayer.hpp"
#include "Runtime/World/CScriptPlatform.hpp"
#include "Runtime/World/CScriptWaypoint.hpp"
#include "Runtime/CSimplePool.hpp"
#include "Runtime/CStateManager.hpp"
#include "Runtime/GameGlobalObjects.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
namespace MP1 {

namespace {
std::array skWingBones{
    "L_wingBone1_1"sv,    "L_wingBone1_2"sv,    "L_wingBone2_1"sv,    "L_wingBone2_2"sv,    "L_wingBone3_1"sv,
    "L_wingBone3_2"sv,    "L_wingFlesh1_1"sv,   "L_wingFlesh1_2"sv,   "L_wingFlesh2_1"sv,   "L_wingFlesh2_2"sv,
    "L_wingFlesh3_1"sv,   "L_wingFlesh3_2"sv,   "R_wingBone1_1"sv,    "R_wingBone1_2"sv,    "R_wingBone2_1"sv,
    "R_wingBone2_2"sv,    "R_wingBone3_1"sv,    "R_wingBone3_2"sv,    "R_wingFlesh1_1"sv,   "R_wingFlesh1_2"sv,
    "R_wingFlesh2_1"sv,   "R_wingFlesh2_2"sv,   "R_wingFlesh3_1"sv,   "R_wingFlesh3_2"sv,   "L_wingtip_1_LCTR"sv,
    "L_wingtip_2_LCTR"sv, "L_wingtip_3_LCTR"sv, "R_wingtip_1_LCTR"sv, "R_wingtip_2_LCTR"sv, "R_wingtip_3_LCTR"sv,
};

std::array skWingEffects{
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

std::array<SOBBRadiiJointInfo, 4> skTail{{{"Tail_1", "Tail_3", 0.66f},
                                          {"Tail_3", "Tail_5", 0.66f},
                                          {"Tail_5", "Tail_7", 0.66f},
                                          {"Tail_7", "Tail_9", 0.66f}}};

std::array<SSphereJointInfo, 10> skSphereJoints{{{"Skeleton_Root", 0.6f},
                                                 {"Spine_2", 0.6f},
                                                 {"breastPlate_LCTR", 0.6f},
                                                 {"Head_1", 0.6f},
                                                 {"L_wrist", 0.5f},
                                                 {"R_wrist", 0.5f},
                                                 {"L_ankle", 0.6f},
                                                 {"R_ankle", 0.6f},
                                                 {"L_pinky_1", 0.4f},
                                                 {"R_pinky_1", 0.4f}}};
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
, xa31_24_(true)
, xa31_25_(true)
, xa31_26_(false)
, xa31_27_(false)
, xa31_28_(false)
, xa31_29_(false)
, xa31_30_(false)
, xa31_31_(false)
, xa32_24_(false)
, xa32_25_(false)
, xa32_26_(false)
, xa32_28_(false)
, xa32_29_(false)
, xa32_31_(true)
, xa33_24_(false)
, xa33_25_(true)
, xa33_26_(false)
, xa33_27_(true)
, xa33_28_(false)
, xa33_29_(false)
, xa33_30_(false)
, xa33_31_(false)
, xa34_24_(false)
, xa34_25_(false)
, xa34_26_(false)
, xa38_(CStaticRes(x568_data.x24_, 4.f * GetModelData()->GetScale()))
, xadc_(44.f * GetModelData()->GetScale().z())
, xae0_(20.f * GetModelData()->GetScale().z())
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
, xc8c_(GetContactDamage())
, xcd0_(g_SimplePool->GetObj({SBIG('ELSC'), x568_data.x3f0_}))
, xce0_(new CParticleElectric(xcd0_))
, xd10_(new CProjectedShadow(128, 128, true)) {
  xe7_30_doTargetDistanceTest = true;
  xb68_.Token().Lock();
  xc14_.Token().Lock();
  xc3c_.Token().Lock();

  if (xce0_)
    xce0_->SetParticleEmission(true);

  const auto& animData = GetModelData()->GetAnimationData();
  for (size_t i = 0; i < skWingBones.size(); ++i) {
    xce4_.push_back(animData->GetLocatorSegId(skWingBones[i]));
  }

  xae4_ = GetAnimationDistance(CPASAnimParmData(7, CPASAnimParm::FromEnum(4), CPASAnimParm::FromEnum(3)));
  x460_knockBackController.SetAnimationStateRange(EKnockBackAnimationState::Flinch, EKnockBackAnimationState::Flinch);
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableFreeze(false);
  x460_knockBackController.SetEnableBurnDeath(false);
  x460_knockBackController.SetEnableLaggedBurnDeath(false);
  CreateShadow(false);
  SetActive(true);
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
  x980_ = std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, false);
  joints.clear();
  joints.reserve(skSphereJoints.size());
  for (const auto& jInfo : skSphereJoints) {
    joints.push_back(
        CJointCollisionDescription::SphereCollision(animData->GetLocatorSegId(jInfo.name), jInfo.radius,
                                                    std::string(GetName()) + " - CollisionActor " + jInfo.name, 10.f));
  }
  x984_ = std::make_unique<CCollisionActorManager>(mgr, GetUniqueId(), GetAreaIdAlways(), joints, false);
  x988_headId = x984_->GetCollisionDescFromIndex(3).GetCollisionActorId();
  x98a_breastPlateId = x984_->GetCollisionDescFromIndex(2).GetCollisionActorId();
  SetupCollisionActors(mgr);
  /* Something something material filter */
  AddMaterial(EMaterialTypes::ProjectilePassthrough);
  /* Flip Ridley around for debugging */
  x34_transform.rotateLocalZ(zeus::degToRad(180.f));
  /* Move ridley to more convenient spot */
  x34_transform.origin = zeus::CVector3f(-356.635315, 30.963602, -38.032295);
}

void CRidley::SetupCollisionActors(CStateManager& mgr) {
  for (size_t i = 0; i < x980_->GetNumCollisionActors(); ++i) {
    const auto& colDesc = x980_->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(colDesc.GetCollisionActorId())) {
      colAct->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerabilty());
      colAct->HealthInfo(mgr)->SetHP(1000.f);
      colAct->SetMaterialFilter(CMaterialFilter::MakeInclude({EMaterialTypes::Player, EMaterialTypes::Platform}));
      colAct->SetWeaponCollisionResponseType(EWeaponCollisionResponseTypes::EnemyNormal);
    }
  }

  x980_->AddMaterial(mgr, {EMaterialTypes::AIJoint});

  for (size_t i = 0; i < x984_->GetNumCollisionActors(); ++i) {
    const auto& colDesc = x984_->GetCollisionDescFromIndex(i);
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(colDesc.GetCollisionActorId())) {
      colAct->SetDamageVulnerability(CDamageVulnerability::NormalVulnerabilty());
      colAct->HealthInfo(mgr)->SetHP(1000.f);
      colAct->SetMaterialFilter(CMaterialFilter::MakeInclude({EMaterialTypes::Player, EMaterialTypes::Platform}));
      colAct->SetWeaponCollisionResponseType(EWeaponCollisionResponseTypes::EnemyNormal);
    }
  }
  x984_->AddMaterial(mgr, {EMaterialTypes::AIJoint});
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
    mgr.SetBossParams(GetUniqueId(), xb1c_ + (xb18_ + xcb8_), 90);
    HealthInfo(mgr)->SetHP(xb1c_ + (xb10_ + xb18_));
    mgr.GetPlayer().SetIsOverrideRadarRadius(true);
    mgr.GetPlayer().SetRadarXYRadiusOverride(350.f);
    mgr.GetPlayer().SetRadarZRadiusOverride(175.f);
    break;
  }
  case EScriptObjectMessage::Deactivate: {
    x984_->SetActive(mgr, false);
    x980_->SetActive(mgr, false);
    mgr.GetPlayer().SetIsOverrideRadarRadius(false);
    break;
  }
  case EScriptObjectMessage::Deleted: {
    x984_->Destroy(mgr);
    x980_->Destroy(mgr);
    if (xb64_ != kInvalidUniqueId) {
      mgr.FreeScriptObject(xb64_);
      xb64_ = kInvalidUniqueId;
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
        if (TCastToConstPtr<CScriptWaypoint> wpNextNext = mgr.GetObjectById(wp->NextWaypoint(mgr))) {
          xabc_ = (wpNextNext->GetTranslation() - xa84_.origin).toVec2f().magnitude();
          xac0_ = wpNextNext->GetTranslation().z() - xa84_.origin.z();
          xac4_ = zeus::CAABox(xa84_.origin - zeus::CVector3f(xabc_, xabc_, 10.f),
                               xa84_.origin + zeus::CVector3f(xabc_, xabc_, 100.f));
        }
      }
    }
    break;
  }
  case EScriptObjectMessage::Damage: {
    if (TCastToPtr<CCollisionActor> colAct = mgr.ObjectById(uid)) {
      float f31 = 1000.f - colAct->HealthInfo(mgr)->GetHP();
      colAct->HealthInfo(mgr)->SetHP(1000.f);
      bool r27 = false;
      if (xc64_aiStage == 2) {
        if (xa33_28_) {
          r27 = true;
        } else {
          bool r26 = false;
          xb10_ -= f31;
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
              r27 = true;
              xcb0_ += 1;
              u32 r0 = 4;
              if (xcb0_ < 5)
                r0 = xcb0_;

              xcb0_ = r0;
              r26 = true;
              xcbc_ = 2.f * (5 - xcb0_);
              xcc8_ = 2.f * 0.33f;
            } else if (xa33_26_ && xa31_31_ && f31 > x568_data.x3f4_) {
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
            r27 = true;
          } else {
            TakeDamage(zeus::skForward, 1.f);
            xb20_ = 0.33f;
            if (xa32_29_) {
              if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
                CWeaponMode wMode = proj->GetDamageInfo().GetWeaponMode();
                if (!wMode.IsCharged() && !wMode.IsComboed() && wMode.GetType() == EWeaponType::Missile)
                  xb14_ = 0.f;
                xb14_ -= f31;
                xb24_ = 0.33f;
                x430_damageColor = zeus::CColor(0.5f, 0.f, 0.f);
                if (xb10_ <= 0.f) {
                  xa32_29_ = false;
                  xa32_28_ = true;
                  xb14_ = x568_data.x38_;
                }
              }
            } else if (xa31_27_) {
              x430_damageColor = zeus::CColor(0.5f, 0.f, 0.f);
              if (xb18_ > 0.f) {
                xb18_ -= f31;
                if (xb18_ <= 0.f) {
                  xa31_26_ = true;
                  r27 = true;
                  xb18_ = 0.f;
                  xcbc_ = 0.6667 * x568_data.x40_;
                } else if (xb18_ < xcbc_) {
                  x450_bodyController->GetCommandMgr().DeliverCmd(
                      CBCKnockBackCmd(GetTransform().basis[1], pas::ESeverity::Six));
                  xcbc_ -= (0.33329999f * x568_data.x3c_);
                }
              }
            } else {
              xb1c_ -= f31;
              if (xb1c_ <= 0.f) {
                x401_30_pendingDeath = true;
                mgr.GetPlayer().SetIsOverrideRadarRadius(false);
                xb1c_ = 0.f;
              } else if (xb1c_ < xcbc_) {
                r27 = true;
                x450_bodyController->GetCommandMgr().DeliverCmd(
                    CBCKnockBackCmd(GetTransform().basis[1], pas::ESeverity::Six));
                xcbc_ -= (0.33329999f * x568_data.x3c_);
              }
            }
          }
        }
      }
      HealthInfo(mgr)->SetHP(xb1c_ + xb10_ + xb18_);

      if (r27) {
        if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(colAct->GetLastTouchedObject())) {
          KnockBack(proj->GetTranslation() - proj->GetPreviousPos(), mgr, proj->GetDamageInfo(), EKnockBackType::Radius,
                    false, proj->GetDamageInfo().GetKnockBackPower());
        }
      }
    }
    break;
  }
  case EScriptObjectMessage::InvulnDamage: {
    if (TCastToConstPtr<CGameProjectile> proj = mgr.GetObjectById(uid)) {
      TUniqueId tmpId = kInvalidUniqueId;
      bool r4 = false;
      if (xc64_aiStage == 2 && xa31_31_) {
        tmpId = x98a_breastPlateId;
        r4 = true;
      } else if (xc64_aiStage == 3) {
        if (xa32_29_) {
          tmpId = x988_headId;
          r4 = true;
        } else if (xa31_27_) {
          tmpId = x98a_breastPlateId;
          r4 = true;
        }
      }

      if (!r4) {
        mgr.ApplyDamage(uid, tmpId, proj->GetOwnerId(), proj->GetDamageInfo(),
                        CMaterialFilter::MakeIncludeExclude({EMaterialTypes::Solid}, {}), {});
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
  x984_->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
  x980_->Update(dt, mgr, CCollisionActorManager::EUpdateOptions::ObjectSpace);
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
  u32 r27 = xc74_;
  if (xcc8_ > 0.f) {
    r27 = (30.f * xcc8_) - GetModelData()->GetNumMaterialSets();
  }

  CPlayerState::EPlayerVisor r28 = mgr.GetPlayerState()->GetActiveVisor(mgr);
  u32 r3 = GetModelData()->GetNumMaterialSets();
  u32 r0 = r27 + 1;
  r0 = r3 - r0;
  bool r31 = zeus::PopCount(r0) >> 5;
}
void CRidley::Render(const CStateManager& mgr) const { CPatterned::Render(mgr); }
void CRidley::AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const {
  CPatterned::AddToRenderer(frustum, mgr);
  if (xce0_ && frustum.aabbFrustumTest(*xce0_->GetBounds())) {
    g_Renderer->AddParticleGen(*xce0_);
  }
}
void CRidley::sub80257650(CStateManager& mgr) {
  for (size_t i = 0; i < x984_->GetNumCollisionActors(); ++i) {
    const auto& colDesc = x984_->GetCollisionDescFromIndex(i);
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
  if (!xd0c_) {
    xd0c_ = CSfxManager::AddEmitter(
        CAudioSys::C3DEmitterParmData{GetTranslation(), zeus::skZero3f, 500.f, 0.1f, 1, 0, 127, 63, false, 127}, true,
        127, true, kInvalidAreaId);
  }
}

void CRidley::sub802560d0(float dt) {
  if (IsAlive()) {
    if (xaec_.isMagnitudeSafe()) {
      const float mag = xaec_.magnitude();
      xaec_ = mag - (zeus::clamp(0.f, dt * ((xaf8_.magSquared() == 0.f ? 0.2f : 0.2f * 3.f) * mag), 0.5f) * mag) *
                        (1.f / mag) * xaec_;
      ApplyImpulseWR(GetMass() * xaec_, {});
    }
  } else {
    xaec_.zeroOut();
  }

  xaf8_.zeroOut();
}

void CRidley::sub802563a8(float dt) {
  if (xc64_aiStage == 3 && !x328_25_verticalMovement) {
    SetTranslation({GetTranslation().x(), GetTranslation().y(), xa84_.origin.z()});
    zeus::CVector3f posDiff = GetTranslation() - xa84_.origin;
    float mag = posDiff.magnitude();
    posDiff *= zeus::CVector3f(1.f / mag);
    if (mag > xab4_ + (-6.f * zeus::clamp(-1.f, posDiff.dot(xa84_.basis[1]), 0.f)) && GetVelocity().dot(posDiff) > 0.f)
      Stop();

    MoveToInOneFrameWR(GetTranslation() - posDiff, dt);
  }
}
void CRidley::sub80256b14(float dt, CStateManager& mgr) {}
void CRidley::sub80256624(float dt, CStateManager& mgr) {}
void CRidley::Patrol(CStateManager& mgr, EStateMsg msg, float arg) { CPatterned::Patrol(mgr, msg, arg); }
void CRidley::Dead(CStateManager& mgr, EStateMsg msg, float arg) { CPatterned::Dead(mgr, msg, arg); }
void CRidley::Generate(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Generate(mgr, msg, arg); }
void CRidley::Attack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Attack(mgr, msg, arg); }
void CRidley::LoopedAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == EStateMsg::Activate) {
    if (xa32_31_) {
      xa33_24_ = false;
      xbc4_ = GetTranslation();
      xbd0_ = GetTransform().basis[1];
    } else {
      xa33_24_ = (GetTranslation() - xa84_.origin).magSquared() < 0.f;
    }
  } else if (msg == EStateMsg::Update) {

  } else if (msg == EStateMsg::Deactivate) {
    x450_bodyController->SetLocomotionType(pas::ELocomotionType::Relaxed);
    if (xa32_25_) {
      //sub80256a6c(mgr, 1);
    }

    if (xa32_24_) {
      xcac_ = 0;
    }
    xa32_24_ = false;
    xa32_25_ = false;
    GetModelData()->SetScale(xc68_);
    xa31_31_ = false;
    x55c_moveScale.splat(1.f);
    SetupCollisionActors(mgr);
    //sub802575ac(0.5f, mgr);
    xa32_31_ = false;
  }
}
void CRidley::JumpBack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::JumpBack(mgr, msg, arg); }
void CRidley::DoubleSnap(CStateManager& mgr, EStateMsg msg, float arg) { CAi::DoubleSnap(mgr, msg, arg); }
void CRidley::CoverAttack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::CoverAttack(mgr, msg, arg); }
void CRidley::FadeOut(CStateManager& mgr, EStateMsg msg, float arg) { CAi::FadeOut(mgr, msg, arg); }
void CRidley::Taunt(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Taunt(mgr, msg, arg); }
void CRidley::Flee(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Flee(mgr, msg, arg); }
void CRidley::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::ProjectileAttack(mgr, msg, arg); }
void CRidley::Flinch(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Flinch(mgr, msg, arg); }
void CRidley::Hurled(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Hurled(mgr, msg, arg); }
void CRidley::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::TelegraphAttack(mgr, msg, arg); }
void CRidley::Jump(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Jump(mgr, msg, arg); }
void CRidley::Explode(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Explode(mgr, msg, arg); }
void CRidley::Dodge(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Dodge(mgr, msg, arg); }
void CRidley::Retreat(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Retreat(mgr, msg, arg); }
void CRidley::Approach(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Approach(mgr, msg, arg); }
void CRidley::Enraged(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Enraged(mgr, msg, arg); }
void CRidley::SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) { CAi::SpecialAttack(mgr, msg, arg); }
void CRidley::Land(CStateManager& mgr, EStateMsg msg, float arg) { CAi::Land(mgr, msg, arg); }
bool CRidley::Attacked(CStateManager& mgr, float arg) { return CPatterned::Attacked(mgr, arg); }
bool CRidley::TooClose(CStateManager& mgr, float arg) { return CPatterned::TooClose(mgr, arg); }
bool CRidley::InRange(CStateManager& mgr, float arg) { return CPatterned::InRange(mgr, arg); }
bool CRidley::ShouldAttack(CStateManager& mgr, float arg) { return CAi::ShouldAttack(mgr, arg); }
bool CRidley::ShouldDoubleSnap(CStateManager& mgr, float arg) { return CAi::ShouldDoubleSnap(mgr, arg); }
bool CRidley::ShouldTurn(CStateManager& mgr, float arg) { return CAi::ShouldTurn(mgr, arg); }
bool CRidley::HitSomething(CStateManager& mgr, float arg) { return CAi::HitSomething(mgr, arg); }
bool CRidley::AttackOver(CStateManager& mgr, float arg) { return CAi::AttackOver(mgr, arg); }
bool CRidley::ShouldTaunt(CStateManager& mgr, float arg) { return CAi::ShouldTaunt(mgr, arg); }
bool CRidley::ShouldFire(CStateManager& mgr, float arg) { return CAi::ShouldFire(mgr, arg); }
bool CRidley::ShouldDodge(CStateManager& mgr, float arg) { return CAi::ShouldDodge(mgr, arg); }
bool CRidley::ShouldRetreat(CStateManager& mgr, float arg) { return CAi::ShouldRetreat(mgr, arg); }
bool CRidley::ShouldCrouch(CStateManager& mgr, float arg) { return CAi::ShouldCrouch(mgr, arg); }
bool CRidley::ShotAt(CStateManager& mgr, float arg) { return CAi::ShotAt(mgr, arg); }
bool CRidley::SetAIStage(CStateManager& mgr, float arg) { return CAi::SetAIStage(mgr, arg); }
bool CRidley::AIStage(CStateManager& mgr, float arg) { return CAi::AIStage(mgr, arg); }
bool CRidley::ShouldStrafe(CStateManager& mgr, float arg) { return CAi::ShouldStrafe(mgr, arg); }
bool CRidley::IsDizzy(CStateManager& mgr, float arg) { return CAi::IsDizzy(mgr, arg); }
} // namespace MP1
} // namespace urde
