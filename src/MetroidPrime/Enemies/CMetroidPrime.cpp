#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Enemies/CMetroidPrime.hpp"
#include "Collision/CRayCastResult.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Weapons/CIceAttackProjectile.hpp"
#include "Kyoto/Animation/CInt32POINode.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CHUDBillboardEffect.hpp"
#include "MetroidPrime/Enemies/CMetroidPrimeRelay.hpp"
#include "MetroidPrime/Enemies/CEnergyBall.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CProjectedShadow.hpp"
#include "MetroidPrime/TCastTo.hpp"

#include "Kyoto/CResFactory.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Particles/CParticleSwoosh.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/math.hpp"

static const float skJumpDistances[] = {3.f, 7.f, 15.f};
static const float skHealthConstants[] = {2420.f, 1760.f, 880.f, 0.f};
static const char* const skDrillerLocators[] = {"driller_LCTR1", "driller_LCTR2"};
static const char* const skTractorBeamLocators[] = {"L_eye_3", "R_eye_3"};
static const char* const skFireEffectNames[] = {"Flame_Head", "Flame_HeadLockOn", "Flame_Lshoulder",
                                                "Flame_Rshoulder"};
static const char* const skHeadLockOnLocator = "Head_LockON_SDK";
static const char* const skJawLocator = "Jaw_1";
static const char* const skBottomToothLocator = "C_bottomtooth";
static const char* const skHeadUpAnimation = "B_headup_additive_body";
static const char* const skEyeTrackingNames[] = {"L_eye_1", "L_eye_2", "L_eye_3",
                                                 "R_eye_1", "R_eye_2", "R_eye_3"};
static const pas::ELocomotionType skLocomotions[] = {pas::kLT_Internal10, pas::kLT_Internal11,
                                                     pas::kLT_Internal12};
static const pas::ETauntType skTaunts[] = {pas::kTT_One, pas::kTT_Two, pas::kTT_Zero};
static const pas::EAnimationState skAttackAnimationStates[] = {
    pas::kAS_MeleeAttack,      pas::kAS_Locomotion,       pas::kAS_ProjectileAttack,
    pas::kAS_ProjectileAttack, pas::kAS_ProjectileAttack, pas::kAS_ProjectileAttack,
    pas::kAS_ProjectileAttack, pas::kAS_ProjectileAttack, pas::kAS_ProjectileAttack,
    pas::kAS_MeleeAttack,      pas::kAS_Locomotion,       pas::kAS_MeleeAttack,
    pas::kAS_Scripted,         pas::kAS_ProjectileAttack, pas::kAS_LoopAttack,
    pas::kAS_LieOnGround,      pas::kAS_GroundHit};
static const int skAttackSeverity[14][3] = {
    {3, 3, 3}, {-1, -1, -1}, {2, 2, 2}, {5, 5, 5},    {8, 8, 8},   {11, 11, 11}, {1, 1, 1},
    {4, 4, 4}, {7, 7, 7},    {4, 7, 1}, {-1, -1, -1}, {-1, 2, -1}, {-1, -1, -1}, {0, 0, 0},
};
static const pas::ELocomotionType skLocomotionSeverity[14][3] = {
    {pas::kLT_Invalid, pas::kLT_Invalid, pas::kLT_Invalid},
    {pas::kLT_Internal10, pas::kLT_Internal11, pas::kLT_Internal12},
    {pas::kLT_Invalid, pas::kLT_Invalid, pas::kLT_Invalid},
    {pas::kLT_Invalid, pas::kLT_Invalid, pas::kLT_Invalid},
    {pas::kLT_Invalid, pas::kLT_Invalid, pas::kLT_Invalid},
    {pas::kLT_Invalid, pas::kLT_Invalid, pas::kLT_Invalid},
    {pas::kLT_Invalid, pas::kLT_Invalid, pas::kLT_Invalid},
    {pas::kLT_Invalid, pas::kLT_Invalid, pas::kLT_Invalid},
    {pas::kLT_Invalid, pas::kLT_Invalid, pas::kLT_Invalid},
    {pas::kLT_Invalid, pas::kLT_Invalid, pas::kLT_Invalid},
    {pas::kLT_Internal8, pas::kLT_Internal8, pas::kLT_Internal8},
    {pas::kLT_Invalid, pas::kLT_Invalid, pas::kLT_Invalid},
    {pas::kLT_Invalid, pas::kLT_Invalid, pas::kLT_Invalid},
    {pas::kLT_Invalid, pas::kLT_Invalid, pas::kLT_Invalid},
};
static const float skInitialSwooshTime = 0.3f;

struct SPrimeOBBJointInfo {
  const char* from;
  const char* to;
  float bounds[3];
};

static const SSphereJointInfo skSphereJoints[] = {
    {"Sphere_LCTR", 1.5f}, {"Skeleton_Root", 2.3f}, {skHeadLockOnLocator, 0.92f}};
static const SPrimeOBBJointInfo skBodyJoints[] = {
    {"R_shoulder", "R_elbow", {0.6f, 0.6f, 0.6f}},
    {"R_elbow", "R_wrist", {0.3f, 0.3f, 0.3f}},
    {"R_wrist", "R_hand_LCTR", {0.3f, 0.3f, 0.3f}},
    {"R_hand_LCTR", "R_leg_LCTR", {0.4f, 1.2f, 0.4f}},
    {"R_front_1", "R_front_2", {0.2f, 0.2f, 0.2f}},
    {"R_front_2", "R_front_3", {0.2f, 0.2f, 0.2f}},
    {"R_front_3", "F_R_leg_LCTR", {0.2f, 0.2f, 0.7f}},
    {"R_stinger_1", "R_stinger_2", {0.2f, 0.2f, 0.2f}},
    {"R_stinger_2", "R_spike_LCTR", {0.2f, 0.2f, 0.2f}},
    {"L_shoulder", "L_elbow", {0.6f, 0.6f, 0.6f}},
    {"L_elbow", "L_wrist", {0.3f, 0.3f, 0.3f}},
    {"L_wrist", "L_hand_LCTR", {0.3f, 0.3f, 0.3f}},
    {"L_hand_LCTR", "L_leg_LCTR", {0.4f, 1.2f, 0.4f}},
    {"L_front_1", "L_front_2", {0.2f, 0.2f, 0.2f}},
    {"L_front_2", "L_front_3", {0.2f, 0.2f, 0.2f}},
    {"L_front_3", "F_L_leg_LCTR", {0.2f, 0.2f, 0.7f}},
    {"L_stinger_1", "L_stinger_2", {0.4f, 0.4f, 0.4f}},
    {"L_stinger_2", "L_spike_LCTR", {0.2f, 0.2f, 0.2f}},
    {"B_shoulder", "B_elbow", {0.8f, 0.8f, 0.8f}},
    {"B_elbow", "B_wrist", {0.7f, 0.7f, 0.7f}},
    {"B_wrist", "B_leg_LCTR", {0.6f, 0.1f, 0.6f}},
    {"Head_LCTR", "Horn_LCTR", {0.8f, 0.1f, 0.6f}},
    {skJawLocator, skBottomToothLocator, {2.f, 0.2f, 0.5f}},
};

static const char* const skElectricLocators[] = {
    "R_front_2",           "L_front_2",           "R_front_1",    "L_front_1",
    skEyeTrackingNames[2], skEyeTrackingNames[3], "R_elbow",      "L_elbow",
    skEyeTrackingNames[0], skEyeTrackingNames[1], "Head",         "Head_LCTR",
    skEyeTrackingNames[4], skEyeTrackingNames[5], "R_shoulder",   "L_shoulder",
    "R_stinger_2",         "L_stinger_2",         "R_spike_LCTR", "L_spike_LCTR"};

SCameraShakePoint BuildCameraShakePoint(const SPrimeCameraShakePoint& point) {
  return SCameraShakePoint(0, point.x0_attackTime, point.x4_sustainTime, point.x8_duration,
                           point.xc_magnitude);
}

CCameraShakerComponent BuildCameraShakerComponent(const SPrimeCameraShakerComponent& component) {
  const int flags = component.x0_useModulation ? 1 : 0;
  return CCameraShakerComponent(flags, BuildCameraShakePoint(component.x4_am),
                                BuildCameraShakePoint(component.x14_fm));
}

CCameraShakeData BuildCameraShakeData(const SPrimeCameraShakeData& data) {
  return CCameraShakeData(data.x4_duration, data.x8_sfxDist, data.x0_useSfx ? 1 : 0,
                          CVector3f::Zero(), BuildCameraShakerComponent(data.xc_shakerX),
                          BuildCameraShakerComponent(data.x30_shakerY),
                          BuildCameraShakerComponent(data.x54_shakerZ));
}

CMetroidPrime::CVulnerabilityEntry::CVulnerabilityEntry(CInputStream& in)
: x0_propertyCount(in.ReadLong()), x4_damageVulnerability(in), x6c_color(in) {
  x70_[0] = in.ReadLong();
  x70_[1] = in.ReadLong();
}

CMetroidPrime::CMetroidPrimeAttackWeights::CMetroidPrimeAttackWeights(CInputStream& in)
: mAttackWeights(LoadAttackWeights(in)) {}

float CMetroidPrime::CMetroidPrimeAttackWeights::GetAttackWeight(EAttackType attack) const {
  return mAttackWeights[attack];
}

rstl::reserved_vector< float, 14 >
CMetroidPrime::CMetroidPrimeAttackWeights::LoadAttackWeights(CInputStream& in) {
  const int count = rstl::min_val(static_cast< int >(in.ReadLong()), 14);
  rstl::reserved_vector< float, 14 > weights;
  for (int i = 0; i < count; ++i) {
    weights.push_back(in.ReadFloat());
  }
  return weights;
}

CMetroidPrime::CMetroidPrime(
    TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CModelData& mData, const CPatternedInfo& pInfo, const CActorParameters& actorParms,
    int pw1, const CCameraShakeData& shakeData1, const CCameraShakeData& shakeData2,
    const CCameraShakeData& shakeData3, const CMetroidPrimeIceAttack& iceAttack, CAssetId particle1,
    const rstl::reserved_vector< CMetroidPrimeParasiteQueenAttack, 4 >& breathAttacks,
    CAssetId weaponDesc1, const CDamageInfo& dInfo1, const CCameraShakeData& shakeData4,
    CAssetId weaponDesc2, const CDamageInfo& dInfo2, const CCameraShakeData& shakeData5,
    const CPoisonInfo& poisonInfo, const CDamageInfo& dInfo3, const CCameraShakeData& shakeData6,
    CAssetId particle2, CAssetId swoosh, CAssetId particle3, CAssetId particle4,
    const rstl::reserved_vector< CVulnerabilityEntry, 4 >& vulnerabilities)
: CPatterned(kC_MetroidPrimeExo, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_One,
             kBT_Flyer, actorParms, kCS_Large)
, x568_relayId(kInvalidUniqueId)
, x56c_collisionManager(nullptr)
, x570_(1)
, x574_(1)
, x578_(0)
, x57c_(0)
, x580_(0)
, x584_(false)
, x588_(vulnerabilities)
, x8c0_(150.f, 0.f)
, x8c8_(0.f)
, x8cc_headColActor(kInvalidUniqueId)
, x8d0_(3)
, x8d4_(3)
, x8d8_beamColor(CColor::Black())
, x8dc_(CColor::Black())
, x8e0_(CColor::Black())
, x8e4_(0.f)
, x8e8_headUpAdditiveBodyAnimIndex(
      GetModelData()->GetAnimationData()->GetCharacterInfo().GetAnimationIndex(
          rstl::string_l(skHeadUpAnimation)))
, x8ec_(0.f)
, x8f0_(0.f)
, x8f4_24_(false)
, x8f4_25_(false)
, x8f4_26_(false)
, x8f4_27_(false)
, x8f4_28_(false)
, x8f8_()
, x910_(5.f)
, x914_24_(false)
, x918_(-1)
, x91c_(pw1)
, x920_(0.f)
, x924_(4.f)
, x928_(5.f)
, x92c_(0)
, x930_(iceAttack)
, xb24_plasmaProjectileIds(kInvalidUniqueId)
, xc48_(gpSimplePool->GetObj(SObjectTag('PART', particle1)))
, xc50_(rs_new CElementGen(xc48_, CElementGen::kMOT_Normal, CElementGen::kOSF_One))
, xc58_curPlasmaProjectile(-1)
, xc5c_(0.f)
, xc60_(CVector3f::Zero())
, xc6c_(CVector3f::Zero())
, xc78_(weaponDesc1, dInfo1)
, xca0_(shakeData4)
, xd74_(weaponDesc2, dInfo2)
, xd9c_(shakeData5)
, xe70_(poisonInfo)
, xeac_(kInvalidUniqueId)
, xeb0_(0)
, xeb4_(dInfo3)
, xed0_(shakeData6)
, xfa4_(gpSimplePool->GetObj(*gpResourceFactory->GetResourceIdByName("Effect_Electric")))
, xfac_(rs_new CParticleElectric(xfa4_))
, xfb4_(0.f)
, xfb8_(0.f)
, xfbc_()
, xfc0_(false)
, xfc1_(false)
, x1014_(gpSimplePool->GetObj(SObjectTag('PART', particle3)))
, x101c_(gpSimplePool->GetObj(SObjectTag('PART', particle4)))
, x1024_(rs_new CElementGen(x1014_, CElementGen::kMOT_Normal, CElementGen::kOSF_One))
, x1044_billboardId(kInvalidUniqueId)
, x1046_(kInvalidUniqueId)
, x1048_(0.f)
, x104c_(75.f, 0.f)
, x1054_24_(false)
, x1054_25_(false)
, x1054_26_(false)
, x1054_27_(false)
, x1074_(0.f)
, x1078_(-1)
, x107c_(0.f)
, x1084_(0.f)
, x1088_(0.f)
, x108c_(shakeData1)
, x1294_(shakeData2)
, x1368_(shakeData3)
, x143c_(rs_new CProjectedShadow(128, 128, true))
, x1440_(-1)
, x1444_24_(false)
, x1444_25_(false) {
  for (int i = 0; i < breathAttacks.size(); ++i) {
    const CMetroidPrimeParasiteQueenAttack& attack = breathAttacks[i];
    x96c_.push_back(attack.x0_beamInfo);
    xb30_.push_back(attack.x64_struct5);
    xbc4_.push_back(rstl::pair< float, CDamageInfo >(attack.x88_, attack.x8c_dInfo2));
    xa80_.push_back(CProjectileInfo(attack.x44_, attack.x48_dInfo1));
    xa80_[i].Token().Lock();
  }
  x460_knockBackController.SetAutoResetImpulse(false);
  x460_knockBackController.SetEnableBurn(false);
  x460_knockBackController.SetEnableFreeze(false);
  xc78_.Token().Lock();
  xd74_.Token().Lock();
  xfc4_.push_back(gpSimplePool->GetObj(SObjectTag('PART', particle2)));
  xfc4_.push_back(gpSimplePool->GetObj(SObjectTag('PART', particle2)));
  xfd8_.push_back(gpSimplePool->GetObj(SObjectTag('SWHC', swoosh)));
  xfd8_.push_back(gpSimplePool->GetObj(SObjectTag('SWHC', swoosh)));
  xfec_.push_back(rs_new CElementGen(xfc4_[0]));
  xfec_.push_back(rs_new CElementGen(xfc4_[1]));
  x1000_.push_back(rs_new CParticleSwoosh(xfd8_[0], 0));
  x1000_.push_back(rs_new CParticleSwoosh(xfd8_[1], 0));
  x102c_.push_back(skInitialSwooshTime);
  x102c_.push_back(skInitialSwooshTime);
  x1038_.push_back(0.f);
  x1038_.push_back(2.f);
}

ENTITY_ACCEPT_IMPL(CMetroidPrime)

bool CMetroidPrime::CodeTrigger(CStateManager& mgr, float arg) { return x1444_24_; }

bool CMetroidPrime::InPosition(CStateManager& mgr, float arg) { return x1084_ <= 0.f; }

bool CMetroidPrime::ShouldFlinch(CStateManager& mgr, float arg) { return x8f4_27_; }

bool CMetroidPrime::ShouldRetreat(CStateManager& mgr, float arg) { return x8f4_28_; }

bool CMetroidPrime::StartAttack(CStateManager& mgr, float arg) { return x920_ <= 0.f; }

bool CMetroidPrime::ShouldMove(CStateManager& mgr, float arg) { return x1254_ == 1; }

bool CMetroidPrime::CoveringFire(CStateManager& mgr, float arg) { return x1254_ == 13; }

bool CMetroidPrime::ShouldDoubleSnap(CStateManager& mgr, float arg) {
  return x328_24_inPosition || x2dc_destObj == kInvalidUniqueId || !CanJump(mgr, 11.f);
}

bool CMetroidPrime::TooClose(CStateManager& mgr, float arg) {
  return CPatterned::TooClose(mgr, arg);
}

bool CMetroidPrime::InMaxRange(CStateManager& mgr, float arg) {
  return CPatterned::InMaxRange(mgr, arg) ||
         (!CanJump(mgr, skJumpDistances[0]) && !CanJump(mgr, skJumpDistances[1]) &&
          !CanJump(mgr, skJumpDistances[2]));
}

bool CMetroidPrime::ShouldSpecialAttack(CStateManager& mgr, float arg) {
  return x1254_ == 2 || x1254_ == 3 || x1254_ == 4 || x1254_ == 5;
}

bool CMetroidPrime::ShouldFire(CStateManager& mgr, float arg) {
  return x1254_ == 6 || x1254_ == 7 || x1254_ == 8;
}

bool CMetroidPrime::ShouldAttack(CStateManager& mgr, float arg) { return x1254_ == 9; }

bool CMetroidPrime::CoverFind(CStateManager& mgr, float arg) { return x1254_ == 12; }

bool CMetroidPrime::ShouldCrouch(CStateManager& mgr, float arg) { return x1254_ == 10; }

bool CMetroidPrime::AttackOver(CStateManager& mgr, float arg) {
  return x8f4_28_ || x8f4_27_ || x1054_25_;
}

bool CMetroidPrime::ShouldJumpBack(CStateManager& mgr, float arg) { return x1254_ == 11; }

bool CMetroidPrime::AIStage(CStateManager& mgr, float arg) {
  return (arg < 0.25f && x1078_ == 0) || (arg > 0.75f && x1078_ == 2) ||
         (x1078_ == 1 && arg >= 0.25f && arg <= 0.75f);
}

bool CMetroidPrime::AggressionCheck(CStateManager& mgr, float arg) {
  const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
  const CVector3f jawPos = GetLctrTransform(rstl::string_l(skJawLocator)).GetTranslation();
  return (playerPos - jawPos).MagSquared() < 324.f;
}

bool CMetroidPrime::CoverCheck(CStateManager& mgr, float arg) { return CanJump(mgr, -8.f); }

bool CMetroidPrime::PlayerSpot(CStateManager& mgr, float arg) {
  return mgr.GetPlayer()->GetFrozenState();
}

void CMetroidPrime::CreateCollisionActors(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  joints.reserve(26);
  const CAnimData& animData = *GetAnimationData();
  for (uint i = 0; i < 23; ++i) {
    const SPrimeOBBJointInfo& joint = skBodyJoints[i];
    const CJointCollisionDescription desc = CJointCollisionDescription::OBBAutoSizeCollision(
        animData.GetLocatorSegId(rstl::string_l(joint.from)),
        animData.GetLocatorSegId(rstl::string_l(joint.to)),
        CVector3f(joint.bounds[0], joint.bounds[1], joint.bounds[2]),
        CJointCollisionDescription::kOT_One, rstl::string_l(joint.from) + rstl::string_l(joint.to),
        200.f);
    joints.push_back(desc);
  }
  for (uint i = 0; i < 3; ++i) {
    const SSphereJointInfo& joint = skSphereJoints[i];
    const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
        animData.GetLocatorSegId(rstl::string_l(joint.name)), joint.radius,
        rstl::string_l(joint.name), 200.f);
    joints.push_back(desc);
  }
  x56c_collisionManager =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, GetActive());
  for (uint i = 0; i < x56c_collisionManager->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = x56c_collisionManager->GetCollisionDescFromIndex(i);
    if (desc.GetName() == rstl::string_l(skHeadLockOnLocator)) {
      x8cc_headColActor = desc.GetCollisionActorId();
    }
    if (CCollisionActor* const actor =
            TCastToPtr< CCollisionActor >(mgr.ObjectById(desc.GetCollisionActorId()))) {
      if (desc.GetCollisionActorId() != x1046_) {
        actor->SetDamageVulnerability(
            *static_cast< const CActor& >(*this).GetDamageVulnerability());
      }
    }
  }
  x56c_collisionManager->AddMaterial(mgr, CMaterialList(kMT_AIJoint, kMT_CameraPassthrough));
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid), CMaterialList(kMT_CollisionActor, kMT_AIPassthrough, kMT_Player)));
  AddMaterial(kMT_ProjectilePassthrough, mgr);
  RemoveMaterial(kMT_Solid, kMT_Orbit, kMT_Target, mgr);
  ResetFaceHealth(mgr);
}

void CMetroidPrime::SetupEyeTracking() {
  for (uint i = 0; i < 6; ++i) {
    x76c_.push_back(CBoneTracking(*GetAnimationData(), rstl::string_l(skEyeTrackingNames[i]),
                                  CMath::Deg2Rad(80.f), CMath::Deg2Rad(180.f),
                                  kBTF_NoParentOrigin));
  }
}

void CMetroidPrime::UpdateCollision(CStateManager& mgr, float dt) {
  x56c_collisionManager->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  const CVector3f scale = GetModelData()->GetScale();
  const CVector3f origin = GetLocatorTransform(rstl::string_l("Skeleton_Root")).GetTranslation();
  CVector3f offset = CVector3f::ByElementMultiply(scale, origin);
  offset = GetTransform().Rotate(offset);
  MoveCollisionPrimitive(offset);
}

void CMetroidPrime::ResetFaceHealth(CStateManager& mgr) {
  if (CCollisionActor* const actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(x8cc_headColActor))) {
    *actor->HealthInfo(mgr) = x8c0_;
  }
}

void CMetroidPrime::EnableEyeTracking(CStateManager& mgr, const bool active) {
  for (int i = 0; i < x76c_.size(); ++i) {
    x76c_[i].SetActive(active);
    x76c_[i].SetTarget(mgr.GetPlayer()->GetUniqueId());
  }
}

void CMetroidPrime::UpdateEyeTracking(CStateManager& mgr, float dt) {
  AnimationData()->PreRender();
  for (int i = 0; i < x76c_.size(); ++i) {
    x76c_[i].Update(dt);
    x76c_[i].PreRender(mgr, *AnimationData(), GetTransform(), GetModelData()->ScaleCopy(),
                       *x450_bodyController);
  }
  if (GetPreRenderClipped()) {
    SetPreRenderClipped(!x1054_24_);
  }
}

void CMetroidPrime::UpdateFaceHealth(CStateManager& mgr) {
  if (CCollisionActor* const actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(x8cc_headColActor))) {
    if (actor->HealthInfo(mgr)->GetHP() <= 0.f && !x8f4_28_) {
      x8f4_28_ = true;
      --x8d0_;
      if (x8d0_ == 0) {
        x400_24_hitByPlayerProjectile = true;
      }
    }
    if (x8f4_28_) {
      ResetFaceHealth(mgr);
    }
    if (x91c_ >= 0 && x91c_ < 4) {
      if (x914_24_) {
        HealthInfo(mgr)->SetHP(skHealthConstants[rstl::max_val(0, x91c_ - 1)]);
      } else {
        const float base = skHealthConstants[x91c_];
        const float remaining = (x8d0_ - 1) * x8c0_.GetHP();
        const float current = rstl::max_val(actor->HealthInfo(mgr)->GetHP(), 0.f);
        const float total = current + (base + remaining);
        HealthInfo(mgr)->SetHP(total);
      }
    }
  }
}

void CMetroidPrime::SetFaceVulnerable(CStateManager& mgr, const bool enable) {
  x8f4_25_ = enable;
  SetFaceVulnerability(mgr, static_cast< EVulnerabilities >(x570_), enable);
  AnimationData()->SetParticleEffectState(rstl::string_l("Eyes"), enable, mgr);
  if (enable) {
    SetTargetColor(x588_[x570_].x6c_color, mgr);
  } else {
    SetTargetColor(CColor::Black(), mgr);
  }
}

void CMetroidPrime::UpdateTargetColor(CStateManager& mgr, float dt) {
  if (x8f4_24_) {
    if (x8e4_ < 1.f) {
      x8e4_ = rstl::min_val(1.f, x8e4_ + dt / 0.3f);
      x8d8_beamColor = CColor::Lerp(x8dc_, x8e0_, x8e4_);
    } else {
      x8d8_beamColor = x8e0_;
      x8f4_24_ = false;
      AnimationData()->SetParticleEffectState(rstl::string_l("ColorChange"), false, mgr);
    }
  }
}

void CMetroidPrime::SetFaceVulnerability(CStateManager& mgr, EVulnerabilities vulnerability,
                                         const bool enable) {
  if (x570_ != vulnerability) {
    AnimationData()->SetParticleEffectState(rstl::string_l("ColorChange"), true, mgr);
    const TAreaId areaId = GetCurrentAreaId();
    CAudioSys::C3DEmitterParmData emitter(1000.f, 0.1f, 1, 127, 20);
    emitter.x0_pos = GetTranslation();
    emitter.xc_dir = CVector3f::Zero();
    emitter.x24_sfxId = 0xb9a;
    CSfxManager::AddEmitter(emitter, true, CSfxManager::kMedPriority, false, areaId.Value());
  }
  x570_ = vulnerability;
  SetTargetColor(x588_[x570_].x6c_color, mgr);
  if (CCollisionActor* const actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(x8cc_headColActor))) {
    if (enable) {
      actor->SetDamageVulnerability(x588_[x570_].x4_damageVulnerability);
      actor->AddMaterial(kMT_Target, kMT_Orbit, mgr);
    } else {
      actor->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerability());
      mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
      actor->RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
    }
  }
}

int CParticleSwoosh::GetAliveParticleSystemCount() { return mSwooshAliveCount; }

void CMetroidPrime::UpdateTractorBeams(float dt, CStateManager& mgr) {
  float pull = 0.f;
  if (x450_bodyController->GetPercentageFrozen() > 0.f && x1054_24_) {
    EnableTractorBeams(mgr, false);
    x1054_25_ = true;
  }
  for (int i = 0; i < 2; ++i) {
    x102c_[i] -= dt;
    x1038_[i] += 5.f * dt;
    const float time = x102c_[i] / 0.3f;
    const CVector3f eye =
        GetLctrTransform(rstl::string_l(skTractorBeamLocators[i])).GetTranslation();
    const float ballHalfExtent = gpTweakPlayer->GetPlayerBallHalfExtent();
    const CVector3f playerCenter =
        mgr.GetPlayer()->GetTranslation() + ballHalfExtent * CVector3f::Up();
    const CVector3f target = playerCenter + ballHalfExtent * (eye - playerCenter).AsNormalized();
    if (x1054_24_) {
      const float extension = CMath::Clamp(0.f, 2.f * time - 1.f, 1.f);
      const float width = CMath::Clamp(0.f, 0.5f + time, 1.f);
      const CVector3f end = CVector3f::Lerp(target, eye, extension);
      UpdateTractorBeamFX(eye, end, static_cast< ETractorbeamPosition >(i), 2.f * width,
                          0.5f * width, x1038_[i]);
      if (width <= 0.f) {
        pull += 1.f;
        if (!mgr.RayCollideWorld(
                eye, end,
                CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid),
                                                    CMaterialList(kMT_Player, kMT_CollisionActor)),
                this)) {
          x1054_25_ = true;
        }
      } else if (extension <= 0.f) {
        pull += 1.f - width;
      }
      x1024_->SetTranslation(end);
    } else if (CParticleSwoosh::GetAliveParticleSystemCount() != 0) {
      const float extension = CMath::Clamp(0.f, 3.f * time - 2.f, 1.f);
      const float width = CMath::Clamp(0.f, 0.5f + time, 1.f);
      const CVector3f end = CVector3f::Lerp(eye, target, extension);
      UpdateTractorBeamFX(eye, end, static_cast< ETractorbeamPosition >(i), width, width,
                          x1038_[i]);
      x1000_[i]->Update(dt);
    }
    xfec_[i]->Update(dt);
  }
  x1024_->Update(dt);
  if (0.5f * pull > 0.9f && x1054_24_) {
    x1024_->SetParticleEmission(true);
    EnableTractorBeamVisorEffect(mgr, true);
  }
  if (close_enough(pull, 0.f)) {
    return;
  }
  if (mgr.GetPlayer()->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
    if (mgr.GetPlayer()->GetMorphBall()->IsBoosting()) {
      x1054_25_ = true;
    }
    if (1.f == mgr.GetPlayer()->GetAttachedActorStruggle()) {
      x1054_25_ = true;
    }
  }
  CPlayer& player = *mgr.Player();
  const CVector3f jaw = GetLctrTransform(rstl::string_l(skJawLocator)).GetTranslation();
  const CVector3f direction = (jaw - player.GetTranslation()).AsNormalized();
  const float acceleration = player.GetMorphballTransitionState() == CPlayer::kMS_Morphed
                                 ? 31.499998f
                             : player.GetOrbitState() == CPlayer::kOS_NoOrbit ? 136.5f
                                                                              : 20.474998f;
  if ((x1054_26_ && pull > 0.75f) || x1048_ > 0.f) {
    const float mass = player.GetMass();
    const float gravity = GravityConstant();
    if (x1054_26_) {
      x1054_26_ = false;
      x1048_ = CMath::SqrtF(1.5f / gravity);
    } else {
      x1048_ -= dt;
    }
    const CVector3f impulse = mass * CMath::SqrtF(1.5f * gravity) * CVector3f::Up();
    player.ApplyImpulseWR(dt * impulse, CAxisAngle::Identity());
    player.SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  }
  const CVector3f impulse = (0.5f * pull) * (acceleration * (player.GetMass() * direction));
  player.ApplyImpulseWR(dt * impulse, CAxisAngle::Identity());
  player.UseCollisionImpulses();
  player.SetAccelerationChangeTimer(2.f * dt);
}

void CMetroidPrime::UpdateTractorBeamFX(const CVector3f& from, const CVector3f& to,
                                        ETractorbeamPosition index, float width, float height,
                                        float phase) {
  CElementGen& particles = *xfec_[index];
  CParticleSwoosh& swoosh = *x1000_[index];
  CVector3f delta = to - from;
  const float distance = delta.Magnitude();
  CVector3f swooshDelta = delta;
  if (!close_enough(delta, CVector3f::Zero(), 0.0001f)) {
    CVector3f position = from;
    const CTransform4f orientation = CTransform4f::LookAt(CVector3f::Zero(), delta);
    int count = static_cast< int >(2.f * distance + 1.f);
    const float particleCount = count;
    const float step = 1.f / particleCount;
    delta *= step;
    particles.SetParticleEmission(true);
    const float particleStep = 1.f / count;
    for (int i = 0; i < count; ++i) {
      const float fi = i;
      const float t = fi * particleStep;
      const float x = width * (t * CMath::FastCosR(fi + phase));
      const float z = height * (t * CMath::FastSinR(fi));
      particles.SetTranslation(position +
                               (i > 0 ? orientation * CVector3f(x, 0.f, z) : CVector3f::Zero()));
      particles.ForceParticleCreation(1);
      position += delta;
    }
    particles.SetParticleEmission(false);
    rstl::vector< CParticleSwoosh::SSwooshData >& segments = swoosh.Swooshes();
    const int last = segments.size() - 1;
    CVector3f swooshPosition = from;
    float rotation = segments.back().mInitialRot;
    const float swooshStep = 1.f / last;
    swooshDelta *= swooshStep;
    for (int i = 0; i < segments.size(); ++i) {
      const float fi = i;
      const float t = fi * swooshStep;
      const float x = width * (t * CMath::FastCosR(fi + phase));
      const float z = height * (t * CMath::FastSinR(fi));
      const CVector3f& offset = i > 0 ? orientation * CVector3f(x, 0.f, z) : CVector3f::Zero();
      const CVector3f translation = swooshPosition + offset;
      CParticleSwoosh::SSwooshData& segment = segments[i];
      segment.mTranslation = translation;
      segment.mOrientation = orientation;
      const float nextRotation = segment.mInitialRot;
      segment.mInitialRot = rotation;
      rotation = nextRotation;
      swooshPosition += swooshDelta;
    }
  }
}

void CMetroidPrime::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                    EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile:
    switch (x92c_) {
    case 6: {
      const CTransform4f locator = GetLctrTransform(node.GetLocatorName());
      CTransform4f transform = CTransform4f::LookAt(
          locator.GetTranslation() + CVector3f(0.f, 0.f, 1.5f), mgr.GetPlayer()->GetTranslation());
      transform.RotateLocalZ(CRelAngle::FromDegrees(mgr.Random()->Range(-20.f, 20.f)));
      CIceAttackProjectile* projectile = rs_new CIceAttackProjectile(
          gpSimplePool->GetObj(SObjectTag('PART', x930_.x4_particle1)),
          gpSimplePool->GetObj(SObjectTag('PART', x930_.x8_particle2)),
          gpSimplePool->GetObj(SObjectTag('PART', x930_.xc_particle3)), mgr.AllocateUniqueId(),
          GetCurrentAreaId(), mgr.GetPlayer()->GetUniqueId(), true, transform, x930_.x10_dInfo,
          CAABox(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f)), x930_.x2c_,
          CRelAngle::FromDegrees(x930_.x30_).AsRadians(), x930_.x34_texture, x930_.x38_, x930_.x3a_,
          kInvalidAssetId);
      projectile->SetUseWorldRay(true);
      mgr.AddObject(projectile);
      break;
    }
    case 5:
      if ((xeb0_++ & 3) == 0) {
        const CTransform4f locator = GetLctrTransform(node.GetLocatorName());
        CTransform4f transform =
            CTransform4f::LookAt(locator.GetTranslation() + 0.5f * locator.GetForward(),
                                 locator.GetTranslation() + 1.5f * GetTransform().GetForward());
        transform.RotateLocalZ(CRelAngle::FromDegrees(mgr.Random()->Range(-30.f, 30.f)));
        CEnergyProjectile* projectile = rs_new CPoisonProjectile(
            true, ProjectileInfo()->Token(), kWT_AI, transform, kMT_Character,
            ProjectileInfo()->GetDamage(), mgr.AllocateUniqueId(), GetCurrentAreaId(),
            GetUniqueId(), xe70_, xeac_, CWeapon::kPA_BigProjectile | CWeapon::kPA_PlayerUnFreeze,
            CVector3f(1.f, 1.f, 1.f), rstl::optional_object< TLockedToken< CGenDescription > >(),
            CSfxManager::kInternalInvalidSfxId, false);
        if (projectile) {
          mgr.AddObject(projectile);
          CProjectileWeapon& weapon = projectile->Projectile();
          const float speed = mgr.Random()->Range(0.95f, 1.2f);
          weapon.SetVelocity(speed * weapon.GetVelocity());
          projectile->SetCameraShake(xca0_);
        }
        break;
      }
    case 2:
    case 3:
    case 4: {
      const CTransform4f locator = GetLctrTransform(node.GetLocatorName());
      CTransform4f transform =
          CTransform4f::LookAt(locator.GetTranslation() + 0.5f * locator.GetForward(),
                               locator.GetTranslation() + 1.5f * GetTransform().GetForward());
      transform.RotateLocalZ(CRelAngle::FromDegrees(mgr.Random()->Range(-30.f, 30.f)));
      CEnergyProjectile* projectile = rs_new CEnergyProjectile(
          true, ProjectileInfo()->Token(), kWT_AI, transform, kMT_Character,
          ProjectileInfo()->GetDamage(), mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(),
          xeac_, CWeapon::kPA_BigProjectile | CWeapon::kPA_PlayerUnFreeze, false,
          CVector3f(1.f, 1.f, 1.f), rstl::optional_object< TLockedToken< CGenDescription > >(),
          CSfxManager::kInternalInvalidSfxId, false);
      if (projectile) {
        mgr.AddObject(projectile);
        CProjectileWeapon& weapon = projectile->Projectile();
        const float speed = mgr.Random()->Range(0.95f, 1.55f);
        weapon.SetVelocity(speed * weapon.GetVelocity());
        projectile->SetCameraShake(xca0_);
      }
      break;
    }
    case 12:
      LaunchEnergyBall(mgr);
      break;
    }
    handled = true;
    break;
  case kUE_DamageOn:
    switch (x92c_) {
    case 7:
      EnableParasiteQueenProjectile(mgr, true);
      break;
    case 11:
      EnableTractorBeams(mgr, true);
      x1054_26_ = true;
      break;
    }
    handled = true;
    break;
  case kUE_DamageOff:
    switch (x92c_) {
    case 7:
      EnableParasiteQueenProjectile(mgr, false);
      break;
    case 11:
      EnableTractorBeams(mgr, false);
      break;
    }
    break;
  case kUE_ScreenShake: {
    CCameraShakeData shake = x108c_;
    switch (x92c_) {
    case 0:
      shake = x108c_;
      break;
    case 9:
      shake = x1368_;
      break;
    case 10:
      shake = x1294_;
      break;
    case 1: {
      const float distance = (mgr.GetPlayer()->GetTranslation() -
                              GetLctrTransform(node.GetLocatorName()).GetTranslation())
                                 .Magnitude();
      if (distance < xeb4_.GetRadius()) {
        const float scale = 1.f - distance / xeb4_.GetRadius();
        const CDamageInfo damage(xeb4_.GetWeaponMode(), scale * xeb4_.GetDamage(),
                                 scale * xeb4_.GetRadiusDamage(),
                                 scale * xeb4_.GetKnockBackPower());
        mgr.ApplyDamage(
            GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), damage,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
      }
      shake = xed0_;
      shake.SetSfxPositionAndDistance(xeb4_.GetRadius(),
                                      GetLctrTransform(node.GetLocatorName()).GetTranslation());
      break;
    }
    }
    if (!mgr.GetCameraManager()->IsInCinematicCamera()) {
      mgr.CameraManager()->AddCameraShaker(shake, true);
    }
    break;
  }
  case kUE_EffectOn:
    if (x92c_ == 7) {
      xc50_->SetParticleEmission(true);
      if (CPlasmaProjectile* projectile =
              static_cast< CPlasmaProjectile* >(mgr.ObjectById(xb24_plasmaProjectileIds[x570_]))) {
        CColor color = projectile->GetInnerColor();
        color.SetAlpha(1.f);
        xc50_->SetModulationColor(color);
      }
    }
    break;
  case kUE_EffectOff:
    xc50_->SetParticleEmission(false);
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

void CMetroidPrime::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId other, CStateManager& mgr) {
  bool handled = false;
  switch (msg) {
  case kSM_Activate:
    x56c_collisionManager->SetActive(mgr, true);
    break;
  case kSM_Deactivate:
    x56c_collisionManager->SetActive(mgr, false);
    break;
  case kSM_Registered:
    SetDrawShadow(false);
    x450_bodyController->Activate(mgr);
    x450_bodyController->SetLocomotionType(skLocomotions[1]);
    SetupEyeTracking();
    CreateCollisionActors(mgr);
    SetFaceVulnerable(mgr, true);
    EnableEyeTracking(mgr, true);
    CreateParasiteQueenProjectile(mgr);
    CreateMissileTarget(mgr);
    EnableTractorBeams(mgr, false);
    CreateTractorBeamVisorObject(mgr);
    mgr.Player()->SetFrozenTimeoutBias(2.f);
    break;
  case kSM_InitializedInArea:
    RemoveMaterial(kMT_AIBlock, mgr);
    AttachToRelayInRoom(mgr, GetCurrentAreaId());
    if (GetCurrentAreaId() == mgr.GetWorld()->GetCurrentAreaId()) {
      ForwardScriptMsgs(kSS_MaxReached, mgr);
    }
    if (xfac_.get()) {
      xfac_->SetParticleEmission(false);
    }
    break;
  case kSM_Deleted:
    x56c_collisionManager->Destroy(mgr);
    RemoveParasiteQueenProjectile(mgr);
    DeleteMissileTarget(mgr);
    DeleteTractorBeamVisorObject(mgr);
    mgr.Player()->SetFrozenTimeoutBias(0.f);
    break;
  case kSM_Start:
    x1444_24_ = true;
    break;
  case kSM_Touched:
    DoContactDamage(other, mgr);
    break;
  case kSM_Damage:
    DoFaceHitCheck(other, mgr);
    handled = true;
    break;
  case kSM_InvulnDamage:
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::AcceptScriptMsg(msg, other, mgr);
  }
}

void CMetroidPrime::DoFaceHitCheck(const TUniqueId uid, CStateManager& mgr) {
  if (uid == x8cc_headColActor) {
    const CCollisionActor* const actor = TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(uid));
    if (actor && IsAlive()) {
      const TUniqueId touched = actor->GetLastTouchedObject();
      const CWeapon* const weapon = TCastToConstPtr< CWeapon >(mgr.GetObjectById(touched));
      if (weapon &&
          actor->GetDamageVulnerability()->WeaponHurts(
              weapon->GetCurrentDamageInfo().GetWeaponMode(), CDamageVulnerability::kRD_No)) {
        x428_damageCooldownTimer = skDamageHitTime;
        if (uid == x8cc_headColActor) {
          if (weapon->GetCurrentDamageInfo().GetWeaponMode().GetType() == kWT_Ice) {
            if (CCollisionActor* const hitActor =
                    TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
              if (CHealthInfo* health = hitActor->HealthInfo(mgr)) {
                health->SetHP(health->GetHP() - 0.5f * (x8c8_ - health->GetHP()));
              }
            }
          }
          if (weapon->GetCurrentDamageInfo().GetWeaponMode().GetType() == kWT_Wave &&
              (weapon->GetCurrentDamageInfo().GetWeaponMode().IsComboed() ||
               weapon->GetCurrentDamageInfo().GetWeaponMode().IsCharged())) {
            xfb4_ = 1.5f;
            EnableElectrocution(mgr, true);
          }
          if (weapon->GetCurrentDamageInfo().GetWeaponMode().GetType() == kWT_Plasma &&
              (weapon->GetCurrentDamageInfo().GetWeaponMode().IsComboed() ||
               weapon->GetCurrentDamageInfo().GetWeaponMode().IsCharged())) {
            xfb8_ = 1.5f;
            EnableFire(mgr, true);
            xfc1_ = true;
          }
          if (weapon->GetCurrentDamageInfo().GetWeaponMode().GetType() == kWT_Ice &&
              weapon->GetCurrentDamageInfo().GetWeaponMode().IsComboed()) {
            const CVector3f position = CVector3f::Zero();
            const CUnitVector3f direction(actor->GetTranslation() - weapon->GetTranslation());
            Freeze(mgr, position, direction, 2.f);
          }
        }
      }
    }
  }
}

void CMetroidPrime::UpdateUnderbodyDamage(CStateManager& mgr) {
  const CModelData* modelData = GetModelData();
  const CPlayer* player = mgr.GetPlayer();
  const float scale = 0.57735026f * modelData->ScaleCopy().Magnitude();
  CAABox bounds(GetTranslation() + scale * CVector3f(-6.f, -6.f, 2.f),
                GetTranslation() + scale * CVector3f(6.f, 6.f, 5.5f));
  x8f8_ = bounds;
  if (player->GetTouchBounds()->DoBoundsOverlap(bounds) && 0.f >= x420_curDamageRemTime) {
    mgr.ApplyDamage(GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
                    CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
                    CVector3f::Zero());
    x420_curDamageRemTime = x424_damageWaitTime;
  }
}

void CMetroidPrime::UpdateDataFromRelay(CStateManager& mgr) {
  x1160_.clear();
  if (const CMetroidPrimeRelay* const relay =
          TCastToConstPtr< CMetroidPrimeRelay >(mgr.GetObjectById(x568_relayId))) {
    x1160_ = relay->GetRoomParms();
    x8c0_ = relay->GetHealthInfo1();
    x104c_ = relay->GetHealthInfo2();
    x924_ = relay->GetFloat1();
    x928_ = relay->GetFloat2();
    x1080_ = relay->GetFloat3();
    x1440_ = relay->GetW1();
    x918_ = relay->GetW3();
    x584_ = relay->GetB1();
    x574_ = relay->GetW2();
    x8d4_ = relay->GetW4();
    x57c_ = relay->GetW5();
    x580_ = 0;
    ResetAttackDistribution(mgr);
  }
}

void CMetroidPrime::AttachToRelayInRoom(CStateManager& mgr, const TAreaId& areaId) {
  if (x568_relayId != kInvalidUniqueId) {
    if (CMetroidPrimeRelay* const relay =
            TCastToPtr< CMetroidPrimeRelay >(mgr.ObjectById(x568_relayId))) {
      relay->SetMetroidPrimeExoId(kInvalidUniqueId);
    }
  }
  CObjectList& list = mgr.ObjectListById(kOL_All);
  TEditorId editorId = kInvalidEditorId;
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    CEntity* entity = list[i];
    if (CMetroidPrimeRelay* const relay = TCastToPtr< CMetroidPrimeRelay >(entity)) {
      if (relay->GetActive() && relay->GetCurrentAreaId() == areaId) {
        editorId = relay->GetEditorId();
      }
    }
  }
  x568_relayId = kInvalidUniqueId;
  if (editorId != kInvalidEditorId) {
    x568_relayId = mgr.GetIdForScript(editorId);
    CMetroidPrimeRelay* const relay =
        TCastToPtr< CMetroidPrimeRelay >(mgr.ObjectById(x568_relayId));
    if (relay) {
      relay->SetMetroidPrimeExoId(GetUniqueId());
    }
  }
  UpdateDataFromRelay(mgr);
  FindAttachedEnergyBalls(mgr);
}

void CMetroidPrime::SetActorAreaId(CStateManager& mgr, TUniqueId uid, TAreaId areaId) {
  if (CActor* actor = static_cast< CActor* >(mgr.ObjectById(uid))) {
    mgr.SetActorAreaId(*actor, areaId);
  }
}

bool CMetroidPrime::RoomHasRelay(const CStateManager& mgr, const TAreaId& areaId) const {
  const CObjectList& list = mgr.GetObjectListById(kOL_All);
  TEditorId editorId = kInvalidEditorId;
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    const CEntity* entity = list[i];
    const CMetroidPrimeRelay* const relay = TCastToConstPtr< CMetroidPrimeRelay >(entity);
    if (relay && relay->GetActive() && relay->GetCurrentAreaId() == areaId) {
      editorId = relay->GetEditorId();
    }
  }
  return editorId != kInvalidEditorId;
}

void CMetroidPrime::UpdateRoomTransition(CStateManager& mgr) {
  if (!x914_24_) {
    return;
  }
  const TAreaId areaId = mgr.GetWorld()->GetCurrentAreaId();
  if (GetCurrentAreaId() != areaId) {
    if (RoomHasRelay(mgr, areaId)) {
      SetActorAreaId(mgr, GetUniqueId(), areaId);
      for (uint i = 0; i < x56c_collisionManager->GetNumCollisionActors(); ++i) {
        const CJointCollisionDescription& desc =
            x56c_collisionManager->GetCollisionDescFromIndex(i);
        SetActorAreaId(mgr, desc.GetCollisionActorId(), areaId);
      }
      for (int i = 0; i < xb24_plasmaProjectileIds.size(); ++i) {
        SetActorAreaId(mgr, xb24_plasmaProjectileIds[i], areaId);
      }
      SetActorAreaId(mgr, xeac_, areaId);
      AttachToRelayInRoom(mgr, GetCurrentAreaId());
      ForwardScriptMsgs(kSS_MaxReached, mgr);
    } else {
      x1444_25_ = true;
    }
  } else if (x1444_25_) {
    x1444_25_ = false;
    ForwardScriptMsgs(kSS_MaxReached, mgr);
  }
}

void CMetroidPrime::PreThink(float dt, CStateManager& mgr) {
  CPatterned::PreThink(dt, mgr);
  if (!GetActive()) {
    return;
  }
  if (const CCollisionActor* const actor =
          TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(x8cc_headColActor))) {
    if (const CHealthInfo* health = actor->GetHealthInfo(mgr)) {
      x8c8_ = health->GetHP();
    }
  }
}

void CMetroidPrime::Think(float dt, CStateManager& mgr) {
  CPatterned::Think(dt, mgr);
  if (!GetActive()) {
    return;
  }
  UpdateRoomTransition(mgr);
  UpdateEyeTracking(mgr, dt);
  UpdateCollision(mgr, dt);
  UpdateFaceHealth(mgr);
  UpdateTargetColor(mgr, dt);
  UpdateAdditiveHeadAnimation(dt);
  UpdateParasiteQueenProjectile(mgr, dt);
  UpdateTractorBeams(dt, mgr);
  UpdateEnergyBall(mgr, dt);
  UpdateMissileTarget(mgr, dt);
  UpdateUnderbodyDamage(mgr);
  UpdateTimers(dt);
  UpdateFire(mgr, dt);
  UpdateElectricity(mgr, dt);
}

void CMetroidPrime::ChangeFaceVulnerability(CStateManager& mgr) {
  int vulnerability = 0;
  if (x584_) {
    vulnerability = x588_[x570_].x70_[mgr.Random()->Next() & 1];
  } else {
    vulnerability = x570_;
    int attempts = 0;
    do {
      ++attempts;
      if (attempts < 10) {
        vulnerability = x588_[vulnerability].x70_[mgr.Random()->Next() & 1];
      } else if (attempts < 20) {
        vulnerability = x588_[vulnerability].x70_[1];
      } else {
        break;
      }
    } while ((x57c_ & (1 << vulnerability)) == 0 || (x580_ & (1 << vulnerability)) != 0);
    x580_ |= 1 << vulnerability;
  }
  SetFaceVulnerability(mgr, static_cast< EVulnerabilities >(vulnerability), x8f4_25_);
  ResetAttackDistribution(mgr);
}

void CMetroidPrime::ForwardScriptMsgs(EScriptObjectState state, CStateManager& mgr) const {
  if (CMetroidPrimeRelay* const relay =
          TCastToPtr< CMetroidPrimeRelay >(mgr.ObjectById(x568_relayId))) {
    relay->SendScriptMsgs(state, mgr, kSM_None);
  }
}

void CMetroidPrime::PushPlayer(CStateManager& mgr, const CVector3f& direction, float dt) {
  const CVector3f horizontal(direction.GetX(), direction.GetY(), 0.f);
  CPlayer& player = *mgr.Player();
  if (!mgr.RayCollideWorld(
          player.GetTranslation(), player.GetTranslation() + 0.2f * CVector3f::Down(),
          CMaterialFilter::MakeInclude(CMaterialList(kMT_Floor, kMT_Platform)), this)) {
    player.ApplyImpulseWR((10.f * player.GetMass()) * CVector3f::Up(), CAxisAngle::Identity());
    player.SetMoveState(NPlayer::kMS_ApplyJump, mgr);
  }
  if (!close_enough(horizontal, CVector3f::Zero(), 0.0001f)) {
    const float mass = player.GetMass();
    const CVector3f force = mass * (120.f * horizontal.AsNormalized());
    player.ApplyImpulseWR(dt * force, CAxisAngle::Identity());
    player.UseCollisionImpulses();
    player.SetAccelerationChangeTimer(2.f * dt);
  }
}

void CMetroidPrime::CreateTractorBeamVisorObject(CStateManager& mgr) {
  x1044_billboardId = mgr.AllocateUniqueId();
  CHUDBillboardEffect* effect = rs_new CHUDBillboardEffect(
      x101c_, rstl::optional_object< TToken< CElectricDescription > >(), x1044_billboardId, true,
      rstl::string_l(""), CHUDBillboardEffect::GetNearClipDistance(mgr),
      CHUDBillboardEffect::GetScaleForPOV(mgr), CColor(1.f, 1.f, 1.f, 1.f),
      CVector3f(1.f, 1.f, 1.f), CVector3f(0.f, 0.f, 0.f));
  mgr.AddObject(effect);
  effect->SetRunIndefinitely(true);
  effect->GetParticleGen()->SetParticleEmission(false);
}

CMetroidPrime::CMissileTarget::CMissileTarget(TUniqueId uid, bool active, const rstl::string& name,
                                              const CEntityInfo& info)
: CPhysicsActor(uid, active, name, info, CTransform4f::Identity(), CModelData::CModelDataNull(),
                CMaterialList(kMT_Target, kMT_ExcludeFromRadar),
                CAABox(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f)), SMoverData(1.f),
                CActorParameters::None(), 0.3f, 0.1f) {}

CMetroidPrime::CMissileTarget::~CMissileTarget() {}

ENTITY_ACCEPT_IMPL(CMetroidPrime::CMissileTarget)

void CMetroidPrime::CreateMissileTarget(CStateManager& mgr) {
  xeac_ = mgr.AllocateUniqueId();
  CMissileTarget* const target =
      rs_new CMissileTarget(xeac_, true, rstl::string_l(""),
                            CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList));
  mgr.AddObject(target);
}

void CMetroidPrime::LaunchEnergyBall(CStateManager& mgr) {
  int type = -1;
  switch (x570_) {
  case 0:
    type = 1;
    break;
  case 1:
    type = 2;
    break;
  case 2:
    type = 3;
    break;
  case 3:
    type = 0;
    break;
  }
  static uint locator = 0;
  for (AUTO(it, x1058_.begin()); it != x1058_.end(); ++it) {
    const CEnergyBall* const original = PATTERNED_CAST_TO(CEnergyBall, 
      const_cast< CEntity* >(mgr.GetObjectById(mgr.GetIdForScript(*it)))
    );
    if (original && original->GetBallType() == type) {
      const TUniqueId id = mgr.GenerateObject(*it).second;
      CEnergyBall* const ball = PATTERNED_CAST_TO(CEnergyBall, mgr.ObjectById(id));
      if (ball) {
        ball->SetTransform(GetLctrTransform(rstl::string_l(skDrillerLocators[locator++ & 1])));
        ball->AcceptScriptMsg(kSM_Activate, GetUniqueId(), mgr);
        x106c_energyBallIds.push_back(id);
        x1074_ = 0.7f;
        return;
      }
    }
  }
}

void CMetroidPrime::FindAttachedEnergyBalls(CStateManager& mgr) {
  const CMetroidPrimeRelay* const relay =
      TCastToConstPtr< CMetroidPrimeRelay >(mgr.GetObjectById(x568_relayId));
  x1058_.clear();
  if (relay) {
    rstl::reserved_vector< TUniqueId, 8 > ids;
    for (AUTO(it, relay->GetConnectionList().begin()); it != relay->GetConnectionList().end();
         ++it) {
      if (it->x0_state == kSS_Patrol) {
        const TUniqueId id = mgr.GetIdForScript(it->x8_objId);
        CEntity* const entity = mgr.ObjectById(id);
        if (entity) {
          if (entity->GetActive()) {
            entity->AcceptScriptMsg(kSM_Deactivate, GetUniqueId(), mgr);
          }
          x1058_.push_back(it->x8_objId);
          if (x1058_.capacity() - x1058_.size() <= 0) {
            break;
          }
        }
      }
    }
  }
}

void CMetroidPrime::UpdateEnergyBall(CStateManager& mgr, float dt) {
  if (x1074_ <= 0.f) {
    x106c_energyBallIds.clear();
  } else {
    x1074_ -= dt;
    int locator = 0;
    for (AUTO(it, x106c_energyBallIds.begin()); it != x106c_energyBallIds.end(); ++it) {
      CEnergyBall* const ball = PATTERNED_CAST_TO(CEnergyBall, mgr.ObjectById(*it));
      if (ball) {
        ball->SetTransform(GetLctrTransform(rstl::string_l(skDrillerLocators[locator++])));
      }
    }
  }
}

void CMetroidPrime::UpdateMissileTarget(CStateManager& mgr, float dt) {
  CPhysicsActor* const target = TCastToPtr< CPhysicsActor >(mgr.ObjectById(xeac_));
  if (target) {
    const CVector3f delta = mgr.GetPlayer()->GetTranslation() - target->GetTranslation();
    if (!close_enough(delta, CVector3f::Zero(), 0.0001f)) {
      const CVector3f direction = delta.AsNormalized();
      const float speed = CVector3f::Dot(GetTransform().GetForward(), direction) > 0.f ? 7.5f : 5.f;
      target->SetVelocityWR(speed * direction);
    } else {
      target->SetVelocityWR(CVector3f::Zero());
    }
  }
}

void CMetroidPrime::UpdateParasiteQueenProjectile(CStateManager& mgr, float dt) {
  const CTransform4f jawXf = GetLctrTransform(rstl::string_l(skJawLocator));
  xc50_->SetTranslation(jawXf.GetTranslation());
  xc50_->SetOrientation(jawXf.GetRotation());
  xc50_->Update(dt);
  if (xc58_curPlasmaProjectile < 0 || xc58_curPlasmaProjectile > 3) {
    return;
  }
  CPlasmaProjectile* const projectile = static_cast< CPlasmaProjectile* >(
      mgr.ObjectById(xb24_plasmaProjectileIds[xc58_curPlasmaProjectile]));
  if (projectile && projectile->GetActive()) {
    if (x450_bodyController->GetPercentageFrozen() > 0.f) {
      EnableParasiteQueenProjectile(mgr, false);
    }
    CTransform4f xf = CTransform4f::Identity();
    xc5c_ = CMath::Clamp(0.f, xc5c_ + dt, 1.4f);
    const float t = xc5c_ / 1.4f;
    const CVector3f target = CVector3f::Lerp(xc60_, xc6c_, t);
    const CVector3f forward = GetTransform().GetForward();
    const CVector3f direction = (target - jawXf.GetTranslation()).AsNormalized();
    if (CVector3f::Dot(forward, direction) > 0.7071067f) {
      xf = CTransform4f::LookAt(jawXf.GetTranslation(), target, CVector3f::Up());
    } else {
      xf = (CQuaternion::LookAt(CUnitVector3f(forward.GetX(), forward.GetY(), forward.GetZ()),
                                CUnitVector3f(direction.GetX(), direction.GetY(), direction.GetZ()),
                                CRelAngle::FromDegrees(45.f)) *
            CQuaternion::FromMatrix(GetTransform()))
               .BuildTransform4f(jawXf.GetTranslation());
    }
    projectile->UpdateFx(xf, dt, mgr);
  }
}

void CMetroidPrime::CreateParasiteQueenProjectile(CStateManager& mgr) {
  xc50_->SetParticleEmission(false);
  xc50_->SetGlobalScale(GetModelData()->ScaleCopy());
  for (int i = 0; i < x96c_.size(); ++i) {
    xb24_plasmaProjectileIds[i] = mgr.AllocateUniqueId();
    const CDamageInfo damage(CWeaponMode(kWT_PoisonWater), xa80_[i].GetDamage().GetDamage(),
                             xa80_[i].GetDamage().GetRadius(),
                             xa80_[i].GetDamage().GetKnockBackPower(), true);
    CPlasmaProjectile* const projectile = rs_new CPlasmaProjectile(
        xa80_[i].Token(), rstl::string_l(""), xa80_[i].GetDamage().GetWeaponMode().GetType(),
        x96c_[i], CTransform4f::Identity(), kMT_Character, damage, xb24_plasmaProjectileIds[i],
        GetCurrentAreaId(), GetUniqueId(), xb30_[i], true,
        xa80_[i].GetDamage().GetWeaponMode().GetType() == kWT_Ice ? CWeapon::kPA_None
                                                                  : CWeapon::kPA_PlayerUnFreeze);
    projectile->SetPlayerSustainedDamage(xbc4_[i].first, xbc4_[i].second);
    mgr.AddObject(*projectile);
  }
}

void CMetroidPrime::RemoveParasiteQueenProjectile(CStateManager& mgr) {
  for (int i = 0; i < x96c_.size(); ++i) {
    if (xb24_plasmaProjectileIds[i] != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(xb24_plasmaProjectileIds[i]);
      xb24_plasmaProjectileIds[i] = kInvalidUniqueId;
    }
  }
}

CVector3f CMetroidPrime::GetPlayerPosJumpCapped(CStateManager& mgr) {
  static const CMaterialFilter filter = CMaterialFilter::MakeIncludeExclude(
      CMaterialList(kMT_Solid),
      CMaterialList(kMT_Character, kMT_Player, kMT_ProjectilePassthrough));
  const CPlayer* const player = mgr.GetPlayer();
  const CVector3f position = player->GetTranslation();
  const CVector3f direction = CVector3f::Down();
  TEntityList nearList;
  TUniqueId id = kInvalidUniqueId;
  CVector3f target = 0.5f * (player->GetAimPosition(mgr, 0.f) + player->GetTranslation());
  mgr.BuildNearList(nearList, position, direction, 150.f, filter, this);
  const CRayCastResult result =
      mgr.RayWorldIntersection(id, position, direction, 150.f, filter, nearList);
  if (result.IsValid()) {
    target = result.GetPoint() + CVector3f(0.f, 0.f, target.GetZ() - position.GetZ());
  }
  return target;
}

void CMetroidPrime::EnableParasiteQueenProjectile(CStateManager& mgr, bool enable) {
  if (enable) {
    xc58_curPlasmaProjectile = x570_;
    CPlasmaProjectile* const projectile = static_cast< CPlasmaProjectile* >(
        mgr.ObjectById(xb24_plasmaProjectileIds[xc58_curPlasmaProjectile]));
    if (projectile && enable && !projectile->GetActive()) {
      if (projectile->GetCurrentDamageInfo().GetWeaponMode().GetType() == kWT_Power) {
        projectile->SetDamageDuration(1.4f);
      }
      const CTransform4f jawXf = GetLctrTransform(rstl::string_l(skJawLocator));
      const CVector3f target = GetPlayerPosJumpCapped(mgr);
      xc60_ = target;
      xc6c_ = target;
      xc5c_ = 0.f;
      const CTransform4f xf = CTransform4f::LookAt(jawXf.GetTranslation(), xc60_, CVector3f::Up());
      projectile->Fire(xf, mgr, false);
    }
  } else {
    for (int i = 0; i < x96c_.size(); ++i) {
      if (xb24_plasmaProjectileIds[i] != kInvalidUniqueId) {
        CPlasmaProjectile* const projectile =
            static_cast< CPlasmaProjectile* >(mgr.ObjectById(xb24_plasmaProjectileIds[i]));
        if (projectile && projectile->IsFiring()) {
          projectile->ResetBeam(mgr, false);
        }
      }
    }
  }
}

void CMetroidPrime::DoContactDamage(TUniqueId id, CStateManager& mgr) {
  const CCollisionActor* const collision =
      TCastToConstPtr< CCollisionActor >(mgr.GetObjectById(id));
  if (collision && IsAlive()) {
    const TUniqueId touched = collision->GetLastTouchedObject();
    const CDamageInfo& damage = GetContactDamage();
    if (touched == mgr.GetPlayer()->GetUniqueId()) {
      if (mgr.GetPlayer()->GetFrozenState()) {
        mgr.Player()->BreakFrozenState(mgr);
      }
      if (0.f >= x420_curDamageRemTime) {
        mgr.ApplyDamage(
            GetUniqueId(), touched, GetUniqueId(), damage,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        x420_curDamageRemTime = x424_damageWaitTime;
      }
    } else {
      const CActor* const actor = TCastToConstPtr< CActor >(mgr.GetObjectById(touched));
      if (actor && actor->GetMaterialList().HasMaterial(kMT_Platform)) {
        const CWeaponMode& mode = CWeaponMode(kWT_AI, false, false, true);
        mgr.ApplyDamage(
            GetUniqueId(), touched, GetUniqueId(), damage,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
      }
    }
  }
}

void CMetroidPrime::PickNextMajorAttack(CStateManager& mgr) {
  int flags = 0x13c1;
  const TUniqueId id = FindBestAttackWaypoint(mgr, true);
  if (id != kInvalidUniqueId) {
    const CScriptWaypoint* const waypoint =
        TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id));
    if (waypoint && (waypoint->GetTranslation() - GetTranslation()).MagSquared() > 16.f) {
      flags |= 2;
    }
  }
  if (!x1054_27_) {
    flags |= 0x400;
  }
  const CVector3f playerDelta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  if (CVector3f::Dot(GetTransform().GetForward(), playerDelta) > 30.f) {
    flags |= 0x3c;
  }
  if (GetActiveEnergyBallCount(mgr) == 0) {
    flags |= 0x2000;
  }
  if (CanJump(mgr, skJumpDistances[1])) {
    flags |= 0x800;
  }
  PickNextAttack(mgr, flags);
}

int CMetroidPrime::GetActiveEnergyBallCount(CStateManager& mgr) {
  int count = 0;
  CObjectList& list = mgr.ObjectListById(kOL_PhysicsActor);
  for (int i = list.GetFirstObjectIndex(); i != -1; i = list.GetNextObjectIndex(i)) {
    CEntity* entity = list[i];
    const CEnergyBall* const ball = PATTERNED_CAST_TO(CEnergyBall, entity);
    if (ball && ball->GetCurrentAreaId() == GetCurrentAreaId() && ball->GetActive()) {
      ++count;
    }
  }
  return count;
}

void CMetroidPrime::PickNextAttack(CStateManager& mgr, int attack) {
  float total = 0.f;
  const CMetroidPrimeAttackWeights& weights = x1160_[x570_];
  for (int i = 0; i <= 13; ++i) {
    if ((attack & (1 << i)) != 0 && CanTransitionFromReady(static_cast< EAttackType >(i))) {
      total += GetDistributedAttackWeight(weights, i);
    }
  }
  if (total > 30000.f) {
    NormalizeAttackDistribution();
  }
  const float selected = mgr.Random()->Range(0.f, total);
  x1254_ = -1;
  float accumulated = 0.f;
  for (int i = 0; i <= 13; ++i) {
    if ((attack & (1 << i)) != 0 && CanTransitionFromReady(static_cast< EAttackType >(i))) {
      const float weight = GetDistributedAttackWeight(weights, i);
      if (selected > accumulated && selected < accumulated + weight) {
        x1254_ = i;
        x1258_[i] += 3.f;
        break;
      }
      accumulated += weight;
    }
  }
}

float CMetroidPrime::GetDistributedAttackWeight(const CMetroidPrimeAttackWeights& weights,
                                                int attack) const {
  float weight = 0.f;
  if (close_enough(x1258_[attack], 0.f)) {
    return weight;
  } else {
    const float base = weights.GetAttackWeight(static_cast< EAttackType >(attack));
    weight = base * base / x1258_[attack];
  }
  return weight;
}

void CMetroidPrime::NormalizeAttackDistribution() {
  float total = 0.f;
  for (int i = 0; i < x1258_.size(); ++i) {
    total += x1258_[i];
  }
  if (close_enough(total, 0.f)) {
    return;
  }
  for (int i = 0; i < x1258_.size(); ++i) {
    x1258_[i] /= total;
  }
}

void CMetroidPrime::ResetAttackDistribution(CStateManager& mgr) {
  if (x570_ != -1) {
    x1258_.clear();
    const CMetroidPrimeAttackWeights& weights = x1160_[x570_];
    for (int i = 0; i <= 13; ++i) {
      x1258_.push_back(weights.GetAttackWeight(static_cast< EAttackType >(i)));
    }
    if (x1078_ != -1) {
      for (int i = 0; i < 40; ++i) {
        PickNextAttack(mgr, -1);
      }
    }
  }
}

int CMetroidPrime::SeverityForAttackType(EAttackType attack) const {
  return skAttackSeverity[attack][x1078_];
}

pas::ELocomotionType CMetroidPrime::SeverityForLocomotionType(EAttackType attack) const {
  return skLocomotionSeverity[attack][x1078_];
}

bool CMetroidPrime::CanTransitionFromReady(EAttackType attack) {
  switch (skAttackAnimationStates[attack]) {
  case pas::kAS_MeleeAttack:
  case pas::kAS_ProjectileAttack:
    return skAttackSeverity[attack][x1078_] != -1;
  case pas::kAS_Locomotion:
    return skLocomotionSeverity[attack][x1078_] != pas::kLT_Invalid;
  case pas::kAS_Scripted:
    return true;
  default:
    return false;
  }
}

void CMetroidPrime::TryScripted(CStateManager& mgr, int arg) {
  CBodyStateCmdMgr& cmdMgr = x450_bodyController->CommandMgr();
  cmdMgr.DeliverCmd(CBCScriptedCmd(arg, false, false, 0.f));
}

void CMetroidPrime::UpdateAdditiveHeadAnimation(float dt) {
  if (x8e8_headUpAdditiveBodyAnimIndex == -1) {
    return;
  }
  if (x8f4_25_) {
    if (x8ec_ < 1.f) {
      x8ec_ += dt / 0.5f;
      if (x8ec_ >= 1.f) {
        x8ec_ = 1.f;
      }
    }
  } else if (x8ec_ > 0.f) {
    x8ec_ -= dt / 0.1f;
    if (x8ec_ <= 0.f) {
      x8ec_ = 0.f;
    }
  }
  if (x8ec_ > 0.f || x8f4_26_) {
    if (x8ec_ > 0.0001f) {
      AnimationData()->AddAdditiveAnimation(x8e8_headUpAdditiveBodyAnimIndex, x8ec_, true, false);
      x8f4_26_ = true;
    } else {
      AnimationData()->DelAdditiveAnimation(x8e8_headUpAdditiveBodyAnimIndex);
      x8f4_26_ = false;
    }
  }
}

void CMetroidPrime::SetTargetColor(const CColor& color, CStateManager& mgr) {
  x8e4_ = 0.f;
  x8f4_24_ = true;
  x8e0_ = color;
  x8dc_ = x8d8_beamColor;
}

void CMetroidPrime::UpdateTimers(float dt) {
  if (x450_bodyController->GetPercentageFrozen() == 0.f) {
    x107c_ -= dt;
    x1084_ -= dt * GetAnimationData()->GetPlaybackRate();
    x920_ -= dt;
  }
}

void CMetroidPrime::RemoveFaceLockOn(CStateManager& mgr) {
  if (CCollisionActor* const actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(x8cc_headColActor))) {
    actor->RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
  }
}

void CMetroidPrime::UpdateElectricity(CStateManager& mgr, float dt) {
  if (xfac_.get()) {
    xfac_->SetGlobalOrientation(GetTransform().GetRotation());
    xfac_->SetGlobalTranslation(GetTranslation());
    xfac_->SetGlobalScale(GetModelData()->ScaleCopy());
    if (xfc0_) {
      if (xfbc_) {
        CSfxManager::UpdateEmitter(xfbc_, GetTranslation(), CVector3f::Zero(), 127);
      }
      const CAnimData* const animData = GetAnimationData();
      xfac_->SetParticleEmission(true);
      for (int i = 0; i < 4; ++i) {
        const CSegId from = animData->GetLocatorSegId(
            rstl::string_l(skElectricLocators[mgr.Random()->Range(0, 19)]));
        xfac_->SetOverrideIPos(animData->GetLocatorTransform(from, nullptr).GetTranslation());
        const CSegId to = animData->GetLocatorSegId(
            rstl::string_l(skElectricLocators[mgr.Random()->Range(0, 19)]));
        xfac_->SetOverrideFPos(animData->GetLocatorTransform(to, nullptr).GetTranslation());
        xfac_->ForceParticleCreation(1);
      }
      xfac_->SetParticleEmission(false);
      xfb4_ -= dt;
      if (xfb4_ <= 0.f) {
        EnableElectrocution(mgr, false);
      }
    }
    xfac_->Update(dt);
  }
}

void CMetroidPrime::EnableFire(CStateManager& mgr, const bool enable) {
  if (enable && xfc0_) {
    EnableElectrocution(mgr, false);
  }
  for (uint i = 0; i < 4; ++i) {
    AnimationData()->SetParticleEffectState(rstl::string_l(skFireEffectNames[i]), enable, mgr);
  }
  xfc1_ = enable;
  if (enable) {
    if (xfbc_) {
      CSfxManager::RemoveEmitter(xfbc_);
    }
    xfbc_ =
        CSfxManager::AddEmitter(0x51a, GetTranslation(), CVector3f::Zero(), uchar(127), true, true);
  } else {
    CSfxManager::RemoveEmitter(xfbc_);
    xfbc_.Clear();
  }
}

void CMetroidPrime::EnableElectrocution(CStateManager& mgr, bool enable) {
  if (enable && xfc1_) {
    EnableFire(mgr, false);
  }
  xfc0_ = enable;
  if (enable) {
    if (xfbc_) {
      CSfxManager::RemoveEmitter(xfbc_);
    }
    xfbc_ =
        CSfxManager::AddEmitter(0x519, GetTranslation(), CVector3f::Zero(), uchar(127), true, true);
    x450_bodyController->CommandMgr().DeliverCmd(
        CBCAdditiveReactionCmd(pas::kART_Electrocution, 1.f, true));
  } else {
    CSfxManager::RemoveEmitter(xfbc_);
    xfbc_.Clear();
    x450_bodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_StopReaction));
  }
}

void CMetroidPrime::UpdateFire(CStateManager& mgr, float dt) {
  if (xfc1_) {
    xfb8_ -= dt;
    if (xfb8_ <= 0.f) {
      EnableFire(mgr, false);
    }
    if (xfbc_) {
      CSfxManager::UpdateEmitter(xfbc_, GetTranslation(), CVector3f::Zero(), 127);
    }
  }
}

void CMetroidPrime::EnableTractorBeams(CStateManager& mgr, bool enable) {
  for (int i = 0; i < 2; ++i) {
    x1000_[i]->SetParticleEmission(enable);
    if (enable) {
      if (x1054_24_ != enable) {
        x102c_[i] = 0.3f;
        const rstl::vector< CParticleSwoosh::SSwooshData >& swooshes = x1000_[i]->GetSwooshes();
        for (int j = 0; j < swooshes.size(); ++j) {
          x1000_[i]->SetWarmUp();
          x1000_[i]->Update(0.0);
        }
      }
    } else {
      x1024_->SetParticleEmission(false);
      EnableTractorBeamVisorEffect(mgr, false);
    }
  }
  x1054_24_ = enable;
}

void CMetroidPrime::ResetFaceLockOn(CStateManager& mgr) {
  if (CCollisionActor* const actor =
          TCastToPtr< CCollisionActor >(mgr.ObjectById(x8cc_headColActor))) {
    actor->AddMaterial(kMT_Target, kMT_Orbit, mgr);
  }
}

void CMetroidPrime::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CPatterned::PreRender(mgr, frustum);
  x143c_->RenderShadowBuffer(mgr, *GetModelData(), GetTransform(), 1, CVector3f::Zero(), 1.f, 5.f);
  x143c_->SetOpacity(0.8f);
}

void CMetroidPrime::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  CPatterned::AddToRenderer(frustum, mgr);
  if (frustum.BoxInFrustumPlanes(xc50_->GetBounds())) {
    gpRender->AddParticleGen(*xc50_);
  }
  if (frustum.BoxInFrustumPlanes(xfac_->GetBounds())) {
    gpRender->AddParticleGen(*xfac_);
  }
  if (frustum.BoxInFrustumPlanes(x1024_->GetBounds())) {
    gpRender->AddParticleGen(*x1024_);
  }
  for (int i = 0; i < 2; ++i) {
    if (frustum.BoxInFrustumPlanes(xfec_[i]->GetBounds())) {
      gpRender->AddParticleGen(*xfec_[i]);
    }
    if (x1054_24_) {
      gpRender->AddParticleGen(*x1000_[i]);
    }
  }
}

void CMetroidPrime::Render(const CStateManager& mgr) const {
  gpRender->SetGXRegister1Color(x8d8_beamColor);
  CPatterned::Render(mgr);
}

bool CMetroidPrime::CanRenderUnsorted(const CStateManager& mgr) const {
  return mgr.GetPlayerState()->GetCurrentVisor() != CPlayerState::kPV_XRay;
}

CVector3f CMetroidPrime::GetRoomCenter(CStateManager& mgr) {
  TUniqueId id = GetConnectedRelayObject(mgr, kSS_Attack, kSM_Follow);
  float count = 0.f;
  CVector3f center = CVector3f::Zero();
  while (id != kInvalidUniqueId) {
    const CScriptWaypoint* const waypoint =
        TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id));
    if (waypoint) {
      center += waypoint->GetTranslation();
      count += 1.f;
    }
    id = waypoint->NextWaypoint(mgr);
  }
  if (count > 0.f) {
    return (1.f / count) * center;
  }
  return CVector3f::Zero();
}

void CMetroidPrime::DeleteTractorBeamVisorObject(CStateManager& mgr) {
  mgr.DeleteObjectRequest(x1044_billboardId);
}

void CMetroidPrime::EnableTractorBeamVisorEffect(CStateManager& mgr, bool enable) {
  if (CHUDBillboardEffect* const effect =
          TCastToPtr< CHUDBillboardEffect >(mgr.ObjectById(x1044_billboardId))) {
    effect->GetParticleGen()->SetParticleEmission(enable);
  }
}

void CMetroidPrime::DeleteMissileTarget(CStateManager& mgr) { mgr.DeleteObjectRequest(xeac_); }

void CMetroidPrime::ResetMissileTarget(CStateManager& mgr) {
  if (CActor* const target = TCastToPtr< CActor >(mgr.ObjectById(xeac_))) {
    target->SetTranslation(mgr.GetPlayer()->GetTranslation());
  }
}

TUniqueId CMetroidPrime::GetConnectedRelayObject(CStateManager& mgr, EScriptObjectState state,
                                                 EScriptObjectMessage msg) {
  if (const CMetroidPrimeRelay* const relay =
          TCastToConstPtr< CMetroidPrimeRelay >(mgr.GetObjectById(x568_relayId))) {
    rstl::reserved_vector< TUniqueId, 8 > ids;
    for (AUTO(it, relay->GetConnectionList().begin()); it != relay->GetConnectionList().end();
         ++it) {
      if (it->x0_state == state && it->x4_msg == msg) {
        const TUniqueId id = mgr.GetIdForScript(it->x8_objId);
        const CEntity* entity = mgr.GetObjectById(id);
        if (entity && entity->GetActive()) {
          ids.push_back(id);
          if (ids.capacity() - ids.size() <= 0) {
            break;
          }
        }
      }
    }
    if (!ids.empty()) {
      return ids[mgr.Random()->Next() % ids.size()];
    }
  }
  return kInvalidUniqueId;
}

TUniqueId CMetroidPrime::FindBestAttackWaypoint(CStateManager& mgr, bool forward) {
  TUniqueId id = GetConnectedRelayObject(mgr, kSS_Attack, kSM_Follow);
  TUniqueId bestId = kInvalidUniqueId;
  float bestDistance = 0.f;
  while (id != kInvalidUniqueId) {
    if (const CScriptWaypoint* const waypoint =
            TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
      const CVector3f delta = waypoint->GetTranslation() - GetTranslation();
      const float distance = CVector3f::Dot(GetTransform().GetForward(), delta);
      if (forward && distance > 0.f && distance > bestDistance) {
        bestDistance = distance;
        bestId = id;
      } else if (!forward && distance < 0.f && distance < bestDistance) {
        bestId = id;
        bestDistance = distance;
      }
      id = waypoint->NextWaypoint(mgr);
    } else {
      id = kInvalidUniqueId;
    }
  }
  return bestId;
}

void CMetroidPrime::ResetAttackTimeOut(CStateManager& mgr) {
  x920_ = mgr.Random()->Range(x924_, x928_);
}

bool CMetroidPrime::CanJump(CStateManager& mgr, float distance) {
  const TUniqueId id = FindBestAttackWaypoint(mgr, distance >= 0.f);
  if (const CScriptWaypoint* const waypoint =
          TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
    const CModelData* modelData = GetModelData();
    const CVector3f delta = waypoint->GetTranslation() - GetTranslation();
    const float projectedDistance = CVector3f::Dot(GetTransform().GetForward(), delta);
    const float scale = 0.57735026f * modelData->ScaleCopy().Magnitude();
    if (distance >= 0.f) {
      return projectedDistance > distance * scale;
    }
    return projectedDistance < distance * scale;
  }
  return false;
}

bool CMetroidPrime::ShouldTurn(CStateManager& mgr, float arg) {
  const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  const float projectedDistance = CVector3f::Dot(GetTransform().GetForward(), delta);
  const float scale = 0.57735026f * GetModelData()->ScaleCopy().Magnitude();
  return projectedDistance < 0.f * scale;
}

bool CMetroidPrime::Stuck(CStateManager& mgr, float arg) {
  const CVector3f playerDelta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  const float playerDistance = CVector3f::Dot(GetTransform().GetForward(), playerDelta);
  const float scale = 0.57735026f * GetModelData()->ScaleCopy().Magnitude();
  if (playerDistance < 5.f * scale) {
    return false;
  }
  const TUniqueId id = FindBestAttackWaypoint(mgr, true);
  if (const CScriptWaypoint* const waypoint =
          TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
    const CModelData* modelData = GetModelData();
    const CVector3f waypointDelta = waypoint->GetTranslation() - GetTranslation();
    const float waypointDistance = CVector3f::Dot(GetTransform().GetForward(), waypointDelta);
    const float waypointScale = 0.57735026f * modelData->ScaleCopy().Magnitude();
    return waypointDistance < 2.f * waypointScale;
  }
  return true;
}

CProjectileInfo* CMetroidPrime::ProjectileInfo() {
  switch (x92c_) {
  case 2:
  case 3:
  case 4:
    return &xc78_;
  case 5:
    return &xd74_;
  default:
    return nullptr;
  }
}

void CMetroidPrime::Touch(CActor& actor, CStateManager& mgr) {}

CMetroidPrime::~CMetroidPrime() {}

void CMetroidPrime::InActive(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x1084_ = x1080_;
    x1084_ = 0.2f;
    x400_24_hitByPlayerProjectile = false;
    x914_24_ = true;
    SetFaceVulnerable(mgr, false);
    x1078_ = 1;
    x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    ResetFaceHealth(mgr);
    x3b4_speed = 1.f;
    break;
  case kStateMsg_Update:
    if (x107c_ < 0.f && x1084_ < 0.f) {
      x107c_ = x1080_;
      x1084_ = 0.90000004f;
      x1078_ = mgr.Random()->Next() % 3;
      x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    }
    break;
  case kStateMsg_Deactivate:
    x914_24_ = false;
    x1444_24_ = false;
    SetFaceVulnerable(mgr, true);
    ResetAttackTimeOut(mgr);
    x1084_ = x1080_;
    x1088_ = x1084_;
    mgr.SetBossParams(GetUniqueId(), 2860.f, 91);
    x8d0_ = x8d4_;
    break;
  }
}

void CMetroidPrime::Active(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x107c_ = 0.4f;
    x1084_ = x1088_;
    x3b4_speed = 1.f;
    break;
  case kStateMsg_Update:
    if ((x570_ != 0 || x1078_ != 1) && x107c_ < 0.f && x1084_ < 0.f) {
      x107c_ = x1080_;
      x1084_ = 0.90000004f;
      x1078_ = mgr.Random()->Next() % 3;
      x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    }
    break;
  case kStateMsg_Deactivate:
    x1088_ = 0.2f;
    break;
  }
}

void CMetroidPrime::SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x330_stateMachineState.SetDelay(CMath::Clamp(0.2f, x924_ / 4.f, 1.f));
    break;
  case kStateMsg_Update:
    PickNextMajorAttack(mgr);
    break;
  case kStateMsg_Deactivate:
    ResetAttackTimeOut(mgr);
    x1054_27_ = false;
    break;
  }
}

void CMetroidPrime::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Taunt, &CPatterned::TryTaunt, skTaunts[x1078_]);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    break;
  }
}

void CMetroidPrime::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x92c_ = 7;
    x1084_ = 1.0999999f;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_ProjectileAttack, &CPatterned::TryProjectileAttack,
               SeverityForAttackType(static_cast< EAttackType >(x1254_)));
    if (x32c_animState == kAS_Repeat) {
      x1078_ = 1;
      x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    }
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    x92c_ = 0;
    ResetAttackTimeOut(mgr);
    x1088_ = 1.2166667f;
    xc50_->SetParticleEmission(false);
    EnableParasiteQueenProjectile(mgr, false);
    x1254_ = 2;
    break;
  }
}

void CMetroidPrime::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x92c_ = 6;
    x1084_ = 0.2f;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack,
               SeverityForAttackType(static_cast< EAttackType >(9)));
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    x92c_ = 0;
    ResetAttackTimeOut(mgr);
    x1254_ = 2;
    break;
  }
}

void CMetroidPrime::SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    switch (x1254_) {
    case 2:
      x92c_ = 2;
      break;
    case 3:
      x92c_ = 3;
      break;
    case 4:
      x92c_ = 4;
      break;
    case 5:
      x92c_ = 5;
      break;
    }
    x1084_ = 1.2666667f;
    ResetMissileTarget(mgr);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_ProjectileAttack, &CPatterned::TryProjectileAttack,
               SeverityForAttackType(static_cast< EAttackType >(x1254_)));
    if (x32c_animState == kAS_Repeat) {
      x1078_ = 1;
      x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    }
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    x92c_ = 0;
    ResetAttackTimeOut(mgr);
    break;
  }
}

void CMetroidPrime::Cover(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x92c_ = 12;
    x1084_ = 1.2666667f;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_ProjectileAttack, &CPatterned::TryProjectileAttack,
               SeverityForAttackType(static_cast< EAttackType >(13)));
    if (x32c_animState == kAS_Repeat) {
      x1078_ = 1;
      x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    }
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    x92c_ = 0;
    ResetAttackTimeOut(mgr);
    x1254_ = 2;
    break;
  }
}

void CMetroidPrime::CoverAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x92c_ = 1;
    x1084_ = 1.9666666f;
    break;
  case kStateMsg_Update: {
    TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack,
               SeverityForAttackType(static_cast< EAttackType >(0)));
    const CVector3f target = 16.f * GetTransform().GetForward() + GetTranslation();
    const CVector3f direction = (target - GetTranslation()).AsNormalized();
    const CVector3f playerDelta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    if (CVector3f::Dot(direction, playerDelta) < 15.f) {
      PushPlayer(mgr, target - mgr.GetPlayer()->GetTranslation(), arg);
    }
    break;
  }
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    x92c_ = 0;
    x1078_ = 1;
    x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    break;
  }
}

void CMetroidPrime::TurnAround(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x92c_ = 9;
    x32c_animState = kAS_Ready;
    break;
  case kStateMsg_Update: {
    TryCommand(mgr, pas::kAS_Step, &CPatterned::TryStep, 3);
    x1078_ = 1;
    x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    const CVector3f target = GetRoomCenter(mgr);
    const CVector3f direction = (target - GetTranslation()).AsNormalized();
    const CVector3f playerDelta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    if (CVector3f::Dot(direction, playerDelta) < 15.f) {
      PushPlayer(mgr, target - mgr.GetPlayer()->GetTranslation(), arg);
    }
    break;
  }
  case kStateMsg_Deactivate:
    x92c_ = 0;
    x32c_animState = kAS_NotReady;
    break;
  }
}

void CMetroidPrime::Run(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    x92c_ = 10;
    x1084_ = 1.9666666f;
    const TUniqueId id = FindBestAttackWaypoint(mgr, true);
    if (const CScriptWaypoint* const waypoint =
            TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
      x450_bodyController->SetLocomotionType(
          SeverityForLocomotionType(static_cast< EAttackType >(1)));
      x2dc_destObj = id;
      SetDestPos(waypoint->GetTranslation());
      x2ec_reflectedDestPos = GetTranslation();
      x328_24_inPosition = false;
    }
    SetFaceVulnerable(mgr, false);
    break;
  }
  case kStateMsg_Update:
    ApproachDest(mgr);
    break;
  case kStateMsg_Deactivate:
    x92c_ = 0;
    x1078_ = 1;
    x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    SetFaceVulnerable(mgr, true);
    ResetAttackTimeOut(mgr);
    break;
  }
}

void CMetroidPrime::Suck(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x450_bodyController->SetLocomotionType(
        SeverityForLocomotionType(static_cast< EAttackType >(10)));
    x92c_ = 11;
    x1054_25_ = false;
    x1084_ = 0.8f;
    mgr.Player()->AttachActorToPlayer(GetUniqueId(), false);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    mgr.Player()->DetachActorFromPlayer();
    x1078_ = 1;
    x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    x92c_ = 0;
    EnableTractorBeams(mgr, false);
    ResetAttackTimeOut(mgr);
    x1088_ = 0.6f;
    if (mgr.GetPlayer()->GetMorphballTransitionState() != CPlayer::kMS_Morphed) {
      mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
      x402_28_isMakingBigStrike = true;
      x504_damageDur = 0.35f;
      mgr.SendScriptMsgAlways(mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), kSM_Damage);
      const CVector3f playerDelta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
      const CVector3f direction(playerDelta.GetX(), playerDelta.GetY(), 0.f);
      mgr.Player()->ApplyImpulseWR(60.f * (mgr.Player()->GetMass() * direction.AsNormalized()),
                                   CAxisAngle::Identity());
    }
    x1054_27_ = true;
    break;
  }
}

void CMetroidPrime::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    SetFaceVulnerable(mgr, false);
    RemoveFaceLockOn(mgr);
    x8f4_28_ = false;
    x8f4_27_ = false;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_KnockBack, &CPatterned::TryKnockBack, 5);
    if (x428_damageCooldownTimer < 0.25f * skDamageHitTime) {
      x428_damageCooldownTimer = skDamageHitTime;
    }
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    SetFaceVulnerable(mgr, true);
    x1078_ = 1;
    x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    ResetFaceLockOn(mgr);
    break;
  }
}

void CMetroidPrime::Retreat(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    x32c_animState = kAS_Ready;
    ForwardScriptMsgs(kSS_Zero, mgr);
    const TUniqueId id = GetConnectedRelayObject(mgr, kSS_CloseIn, kSM_Follow);
    if (const CScriptWaypoint* const waypoint =
            TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
      SetTransform(waypoint->GetTransform());
    }
    x1078_ = 1;
    x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Scripted,
               static_cast< FTryCommandCallback >(&CMetroidPrime::TryScripted), x918_);
    break;
  case kStateMsg_Deactivate: {
    x32c_animState = kAS_NotReady;
    const TUniqueId id = GetConnectedRelayObject(mgr, kSS_Retreat, kSM_Follow);
    if (const CScriptWaypoint* const waypoint =
            TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(id))) {
      SetTransform(waypoint->GetTransform());
    }
    ++x91c_;
    break;
  }
  }
}

void CMetroidPrime::Crouch(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, 5);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    ResetAttackTimeOut(mgr);
    break;
  }
}

void CMetroidPrime::Dodge(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    x1078_ = 1;
    x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Step, &CPatterned::TryStep, 0);
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    break;
  }
}

void CMetroidPrime::Enraged(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    ChangeFaceVulnerability(mgr);
    break;
  }
}

void CMetroidPrime::Land(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == kStateMsg_Activate) {
    ChangeFaceVulnerability(mgr);
  }
}

void CMetroidPrime::Growth(CStateManager& mgr, EStateMsg msg, float arg) {
  if (msg == kStateMsg_Activate) {
    x3b4_speed = 1.4f;
  }
}

void CMetroidPrime::Approach(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x32c_animState = kAS_Ready;
    SetFaceVulnerable(mgr, false);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, 2);
    if (x32c_animState == kAS_Repeat) {
      x1078_ = 1;
      x450_bodyController->SetLocomotionType(skLocomotions[x1078_]);
    }
    break;
  case kStateMsg_Deactivate:
    x32c_animState = kAS_NotReady;
    SetFaceVulnerable(mgr, true);
    ResetAttackTimeOut(mgr);
    break;
  }
}
