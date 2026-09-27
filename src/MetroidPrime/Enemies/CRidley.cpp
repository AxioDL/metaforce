#include "MetroidPrime/Enemies/CRidley.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/CSimplePool.hpp"
#include "Kyoto/Math/CFrustumPlanes.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Particles/CParticleElectric.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "MetaRender/CCubeRenderer.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CExplosion.hpp"
#include "MetroidPrime/CProjectedShadow.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptPlatform.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"
#include "MetroidPrime/Weapons/CPlasmaProjectile.hpp"
#include "rstl/math.hpp"
#include <math.h>

#if VERSION >= VERSION_GM8P_00
const int CRidley::skNumProperties = 39;
#else
const int CRidley::skNumProperties = 47;
#endif

static const float skMinFlightScale = 0.3f;
static const float skFlightScaleDistance = 250.f;

const rstl::string CRidley::skBreastPlate = rstl::string_l("breastPlate_LCTR");
const rstl::string CRidley::skMouth = rstl::string_l("mouth_LCTR");
const rstl::string CRidley::skHead = rstl::string_l("Head_1");
const rstl::string CRidley::skRoot = rstl::string_l("Skeleton_Root");
static const rstl::string skWingEffects[24] = {
    rstl::string_l("WingSmokeSmall1"), rstl::string_l("WingSmokeSmall2"),
    rstl::string_l("WingSmokeSmall3"), rstl::string_l("WingSmokeSmall4"),
    rstl::string_l("WingSmokeSmall5"), rstl::string_l("WingSmokeSmall6"),
    rstl::string_l("WingSmokeSmall7"), rstl::string_l("WingSmokeSmall8"),
    rstl::string_l("WingFire1"),       rstl::string_l("WingFire2"),
    rstl::string_l("WingFire3"),       rstl::string_l("WingFire4"),
    rstl::string_l("WingFire5"),       rstl::string_l("WingFire6"),
    rstl::string_l("WingFire7"),       rstl::string_l("WingFire8"),
    rstl::string_l("WingSparks1"),     rstl::string_l("WingSparks2"),
    rstl::string_l("WingSparks3"),     rstl::string_l("WingSparks4"),
    rstl::string_l("WingSparks5"),     rstl::string_l("WingSparks6"),
    rstl::string_l("WingSparks7"),     rstl::string_l("WingSparks8"),
};
static const rstl::string skWingBones[30] = {
    rstl::string_l("L_wingBone1_1"),    rstl::string_l("L_wingBone1_2"),
    rstl::string_l("L_wingBone2_1"),    rstl::string_l("L_wingBone2_2"),
    rstl::string_l("L_wingBone3_1"),    rstl::string_l("L_wingBone3_2"),
    rstl::string_l("L_wingFlesh1_1"),   rstl::string_l("L_wingFlesh1_2"),
    rstl::string_l("L_wingFlesh2_1"),   rstl::string_l("L_wingFlesh2_2"),
    rstl::string_l("L_wingFlesh3_1"),   rstl::string_l("L_wingFlesh3_2"),
    rstl::string_l("R_wingBone1_1"),    rstl::string_l("R_wingBone1_2"),
    rstl::string_l("R_wingBone2_1"),    rstl::string_l("R_wingBone2_2"),
    rstl::string_l("R_wingBone3_1"),    rstl::string_l("R_wingBone3_2"),
    rstl::string_l("R_wingFlesh1_1"),   rstl::string_l("R_wingFlesh1_2"),
    rstl::string_l("R_wingFlesh2_1"),   rstl::string_l("R_wingFlesh2_2"),
    rstl::string_l("R_wingFlesh3_1"),   rstl::string_l("R_wingFlesh3_2"),
    rstl::string_l("L_wingtip_1_LCTR"), rstl::string_l("L_wingtip_2_LCTR"),
    rstl::string_l("L_wingtip_3_LCTR"), rstl::string_l("R_wingtip_1_LCTR"),
    rstl::string_l("R_wingtip_2_LCTR"), rstl::string_l("R_wingtip_3_LCTR"),
};

struct SStage2Attack {
  int mPrimary;
  float mProbability;
  int mAlternate;
  SStage2Attack(int primary = -1, float probability = 100.f, int alternate = -1)
  : mPrimary(primary), mProbability(probability), mAlternate(alternate) {}
};
static const SStage2Attack skStage2Attacks[5][12] = {
    {
        SStage2Attack(0, 100.f, -1),
        SStage2Attack(3, 100.f, -1),
        SStage2Attack(2, 100.f, -1),
        SStage2Attack(3, 50.f, 4),
        SStage2Attack(0, 100.f, -1),
        SStage2Attack(3, 100.f, -1),
        SStage2Attack(2, 100.f, -1),
        SStage2Attack(3, 50.f, 4),
        SStage2Attack(5, 100.f, -1),
        SStage2Attack(-1, 100.f, -1),
    },
    {
        SStage2Attack(5, 100.f, -1),
        SStage2Attack(1, 100.f, -1),
        SStage2Attack(1, 100.f, -1),
        SStage2Attack(1, 100.f, -1),
        SStage2Attack(1, 100.f, -1),
        SStage2Attack(-1, 100.f, -1),
    },
    {
        SStage2Attack(5, 100.f, -1),
        SStage2Attack(0, 100.f, -1),
        SStage2Attack(0, 50.f, 4),
        SStage2Attack(2, 100.f, -1),
        SStage2Attack(3, 50.f, 4),
        SStage2Attack(2, 100.f, -1),
        SStage2Attack(3, 50.f, 4),
        SStage2Attack(0, 100.f, -1),
        SStage2Attack(2, 50.f, 3),
        SStage2Attack(2, 100.f, -1),
        SStage2Attack(-1, 100.f, -1),
    },
    {
        SStage2Attack(5, 100.f, -1),
        SStage2Attack(1, 100.f, -1),
        SStage2Attack(1, 100.f, -1),
        SStage2Attack(1, 100.f, -1),
        SStage2Attack(1, 100.f, -1),
        SStage2Attack(-1, 100.f, -1),
    },
    {
        SStage2Attack(5, 100.f, -1),
        SStage2Attack(0, 100.f, -1),
        SStage2Attack(0, 50.f, 3),
        SStage2Attack(3, 100.f, -1),
        SStage2Attack(0, 50.f, 3),
        SStage2Attack(2, 100.f, -1),
        SStage2Attack(2, 50.f, 4),
        SStage2Attack(0, 100.f, -1),
        SStage2Attack(-1, 100.f, -1),
    },
};

#if VERSION >= VERSION_GM8P_00
static const SStage2Attack skStage3Attacks[4][7] = {
    {
        SStage2Attack(2, 100.f, -1),
        SStage2Attack(1, 100.f, -1),
        SStage2Attack(2, 100.f, -1),
        SStage2Attack(5, 100.f, 7),
        SStage2Attack(1, 100.f, -1),
        SStage2Attack(-1, 100.f, -1),
    },
    {
        SStage2Attack(5, 100.f, 2),
        SStage2Attack(6, 100.f, -1),
        SStage2Attack(5, 100.f, 2),
        SStage2Attack(2, 100.f, -1),
        SStage2Attack(1, 100.f, -1),
        SStage2Attack(-1, 100.f, -1),
    },
    {
        SStage2Attack(5, 100.f, -1),
        SStage2Attack(2, 50.f, 7),
        SStage2Attack(6, 100.f, -1),
        SStage2Attack(5, 100.f, 6),
        SStage2Attack(1, 100.f, -1),
        SStage2Attack(-1, 100.f, -1),
    },
    {
        SStage2Attack(2, 30.f, 7),
        SStage2Attack(5, 100.f, 5),
        SStage2Attack(5, 100.f, 5),
        SStage2Attack(5, 100.f, 5),
        SStage2Attack(1, 100.f, -1),
        SStage2Attack(6, 50.f, 7),
        SStage2Attack(-1, 100.f, -1),
    },
};
#endif

const CDamageVulnerability CRidley::skDirectNormal =
    CDamageVulnerability(kVN_DirectNormal, kVN_DirectNormal, kVN_DirectNormal, kVN_DirectNormal,
                         kVN_DirectNormal, kVN_DirectNormal, kVN_DirectNormal, kVN_DirectNormal,
                         kVN_DirectNormal, kVN_DirectNormal, kVN_DirectNormal, kVN_DirectNormal,
                         kVN_DirectNormal, kVN_DirectNormal, kVN_DirectNormal, kDT_None);
const CDamageVulnerability CRidley::skIceWeakness =
    CDamageVulnerability(kVN_DirectNormal, kVN_DirectWeak, kVN_DirectNormal, kVN_DirectNormal,
                         kVN_DirectNormal, kVN_DirectNormal, kVN_DirectNormal, kVN_DirectNormal,
                         kVN_DirectNormal, kVN_DirectNormal, kVN_DirectNormal, kVN_DirectNormal,
                         kVN_DirectNormal, kVN_DirectNormal, kVN_DirectNormal, kDT_None);

struct STailJoint {
  const char* mFrom;
  const char* mTo;
  float mRadius;
};
#if VERSION >= VERSION_GM8P_00
static const STailJoint skTailJoints[4] = {
    {"Tail_1", "Tail_3", 1.48f},
    {"Tail_3", "Tail_5", 1.48f},
    {"Tail_5", "Tail_7", 1.48f},
    {"Tail_7", "Tail_9", 1.48f},
};
#else
static const STailJoint skTailJoints[4] = {
    {"Tail_1", "Tail_3", 0.66f},
    {"Tail_3", "Tail_5", 0.66f},
    {"Tail_5", "Tail_7", 0.66f},
    {"Tail_7", "Tail_9", 0.66f},
};
#endif
struct SBodyJoint {
  const char* mName;
  float mRadius;
};
#if VERSION >= VERSION_GM8P_00
static const SBodyJoint skBodyJoints[10] = {
    {"Skeleton_Root", 1.35f}, {"Spine_2", 1.35f},  {"breastPlate_LCTR", 1.f}, {"Head_1", 1.35f},
    {"L_wrist", 1.1f},        {"R_wrist", 1.1f},   {"L_ankle", 1.35f},        {"R_ankle", 1.35f},
    {"L_pinky_1", 0.9f},      {"R_pinky_1", 0.9f},
};
#else
static const SBodyJoint skBodyJoints[10] = {
    {"Skeleton_Root", 0.6f}, {"Spine_2", 0.6f},   {"breastPlate_LCTR", 0.3f}, {"Head_1", 0.6f},
    {"L_wrist", 0.5f},       {"R_wrist", 0.5f},   {"L_ankle", 0.6f},          {"R_ankle", 0.6f},
    {"L_pinky_1", 0.4f},     {"R_pinky_1", 0.4f},
};
#endif
struct SStage2Settings {
#if VERSION < VERSION_GM8P_00
  uint x0_;
  uint x4_;
  float x8_;
  float xc_;
  float x10_;
  float x14_;
  float x18_;
#endif
  uint x1c_;
  uchar x20_;
  uchar x21_;
  uchar x22_;
  uchar x23_;
};
static const SStage2Settings skStage2Settings[5] = {
#if VERSION >= VERSION_GM8P_00
    {1, 0, 0, 0, 0}, {2, 0, 0, 0, 0}, {2, 1, 0, 0, 0}, {2, 0, 0, 0, 0}, {2, 1, 0, 0, 0},
#else
    {4, 6, 50.f, 50.f, 0.f, 33.f, 0.f, 1, 0, 0, 0, 0},
    {4, 6, 20.f, 20.f, 60.f, 50.f, 0.f, 2, 0, 0, 0, 0},
    {4, 6, 40.f, 40.f, 20.f, 50.f, 50.f, 2, 1, 0, 0, 0},
    {3, 5, 10.f, 15.f, 75.f, 100.f, 25.f, 2, 0, 0, 0, 0},
    {3, 5, 30.f, 30.f, 40.f, 50.f, 50.f, 2, 1, 0, 0, 0},
#endif
};
#if VERSION < VERSION_GM8P_00
static const float skStage3AttackWeights[6][6] = {
    {0.f, 20.f, 40.f, 0.f, 0.f, 40.f}, {0.f, 0.f, 70.f, 0.f, 0.f, 30.f},
    {0.f, 60.f, 0.f, 0.f, 0.f, 40.f},  {0.f, 40.f, 30.f, 0.f, 0.f, 30.f},
    {0.f, 0.f, 50.f, 0.f, 0.f, 50.f},  {0.f, 40.f, 60.f, 0.f, 0.f, 0.f},
};
#endif

CRidleyData::CRidleyData(CInputStream& in, int propCount)
#if VERSION < VERSION_GM8P_00
: x0_(in.ReadLong())
, x4_(in.ReadLong())
, x8_(in.ReadLong())
, xc_(in.ReadLong())
, x10_(in.ReadLong())
, x14_(in.ReadLong())
, x18_(in.ReadLong())
, x1c_(in.ReadLong())
, x20_(in.ReadLong())
, x24_(in.ReadLong())
, x28_(in.ReadLong())
#else
: x28_(in.ReadLong())
#endif
, x2c_(in.ReadLong())
, x30_(in.ReadLong())
, x34_(in.ReadFloat())
, x38_(in.ReadFloat())
, x3c_(in.ReadFloat())
, x40_(in.ReadFloat())
, x44_(in.ReadLong())
, x48_(in)
, x64_(in)
, xa8_(CSfxManager::TranslateSFXID(in.ReadLong()))
, xac_(in.ReadLong())
#if VERSION >= VERSION_GM8P_00
, x88_(in.ReadLong())
#endif
, xb0_(in)
, xcc_(CCameraShakeData::EatOldCameraShakerData(in))
, x1a0_(in.ReadLong())
, x1a4_(in)
, x1c0_(CCameraShakeData::EatOldCameraShakerData(in))
, x294_(CSfxManager::TranslateSFXID(in.ReadLong()))
, x298_(in)
, x2b4_(CCameraShakeData::EatOldCameraShakerData(in))
, x388_(in.ReadFloat())
, x38c_(in.ReadFloat())
, x390_(in)
, x3ac_(in.ReadFloat())
, x3b0_(in)
, x3cc_(in.ReadFloat())
, x3d0_(in)
, x3ec_(in.ReadFloat())
, x3f0_(in.ReadLong())
, x3f4_(in.ReadFloat())
, x3f8_(CSfxManager::TranslateSFXID(in.ReadLong()))
#if VERSION >= VERSION_GM8P_00
, x3fc_(in)
, mBounceDamage(propCount > 39 ? CDamageInfo(in) : x298_)
#else
, x3fc_(propCount > 47 ? CDamageInfo(in) : x48_)
#endif
{
}

CRidley::CRidley(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                 const CTransform4f& xf, const CModelData& modelDataIn, const CPatternedInfo& pInfo,
                 const CActorParameters& actParms, CInputStream& in, uint propCount)
: CPatterned(kC_Ridley, uid, name, kFT_Zero, info, xf, modelDataIn, pInfo, kMT_Flyer, kCT_One, kBT_Flyer,
             actParms, kCS_Large)
, mData(in, propCount)
, mTailCollision(nullptr)
, mBodyCollision(nullptr)
, mHeadId(kInvalidUniqueId)
, mBreastPlateId(kInvalidUniqueId)
, x98c_(gpSimplePool->GetObj(SObjectTag('PART', this->mData.x30_)))
, x998_(CStaticRes(mData.x28_, modelDataIn.ScaleCopy()))
, x9e4_(CStaticRes(mData.x2c_, modelDataIn.ScaleCopy()))
, mBreastPlateSegId(GetModelData()->GetAnimationData()->GetLocatorSegId(skBreastPlate))
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
, mShotAt(false)
, xa32_29_(false)
, xa32_31_(true)
, xa33_24_(false)
, xa33_25_(true)
, xa33_26_(false)
, xa33_27_(true)
, xa33_28_(false)
, mDoStrafe(false)
, xa33_30_(false)
, xa33_31_(false)
, xa34_24_(false)
, xa34_25_(false)
#if VERSION >= VERSION_GM8P_00
, xa34_27_(true)
, xa34_29_(false)
, mJumpCanBeInterrupted(false)
, mCanBreakLockOn(true)
, mHasPreviousBeamPos(false)
, mInFlinch(false)
#else
, xa34_26_(false)
, xa38_(CStaticRes(mData.x24_, 4.f * modelDataIn.ScaleCopy()))
#endif
, xa84_(CTransform4f::Identity())
, xab4_(20.f)
, xab8_(12.f)
, xabc_(40.f)
, xac0_(10.f)
, xac4_(CAABox::MakeMaxInvertedBox())
, xadc_(44.f * GetModelData()->ScaleCopy().GetZ())
, xae0_(20.f * GetModelData()->GetScale().GetX())
, xae8_(9.f * GetModelData()->ScaleCopy().GetZ())
, xaec_(CVector3f::Zero())
, xaf8_(CVector3f::Zero())
, xb04_(2)
, xb0c_(0)
, xb10_(0.f)
, xb14_(mData.x38_)
, xb18_(mData.x3c_)
, xb1c_(mData.x40_)
, xb20_(0.f)
, xb24_(0.f)
, xb28_(GetModelData()->GetAnimationData()->GetLocatorSegId(skRoot))
, xb2c_(*GetModelData()->GetAnimationData(), skHead, CRelAngle::FromDegrees(40.f).AsRadians(),
        CRelAngle::FromDegrees(180.f).AsRadians(),
        static_cast< EBoneTrackingFlags >(kBTF_NoParentOrigin | kBTF_ParentIk))
, mPlasmaProjectile(kInvalidUniqueId)
, xb68_(mData.x44_, mData.x48_)
, mHeadSegId(GetModelData()->GetAnimationData()->GetLocatorSegId(skHead))
, mMouthSegId(GetModelData()->GetAnimationData()->GetLocatorSegId(skMouth))
, xb94_(CTransform4f::RotateX(CRelAngle::FromDegrees(-40.f)))
, xbc4_(CVector3f::Zero())
, xbd0_(CVector3f::Zero())
, xbe4_(CVector3f::Zero())
, xbf0_(CVector3f::Forward())
, xbfc_(CVector3f::Forward())
, xc08_(0.f)
, xc0c_(0.f)
, xc10_(120.f)
#if VERSION >= VERSION_GM8P_00
, mProjectileInfo(mData.xac_, mData.xb0_)
, xc14_(mData.x88_, mData.xb0_)
#else
, xc14_(mData.xac_, mData.xb0_)
#endif
, xc3c_(mData.x1a0_, mData.x1a4_)
, mAiStage(2)
, xc68_(CVector3f::Zero())
, xc74_(0)
, xc78_(0.f)
, xc7c_(0.f)
, xc80_(0.f)
, xc88_(4)
#if VERSION >= VERSION_GM8P_00
, mMeleeAttack(2)
#endif
, xc8c_(GetContactDamage())
, xca8_()
, xcac_()
, xcb0_(0)
, xcb4_(0)
, xcb8_(0.f)
, xcbc_(0.f)
, xcc0_(1)
, xcc4_(1)
, xcc8_(0.f)
, xcd0_(gpSimplePool->GetObj(SObjectTag('ELSC', mData.x3f0_)))
, mElectric(rs_new CParticleElectric(xcd0_))
, xd08_(0.f)
, xd0c_()
, xd10_(rs_new CProjectedShadow(128, 128, true))
#if VERSION >= VERSION_GM8P_00
, mPreviousBeamPos(CVector3f::Zero())
#endif
{
  SetDoTargetDistanceTest(false);
  xb68_.Token().Lock();
#if VERSION >= VERSION_GM8P_00
  mProjectileInfo.Token().Lock();
#endif
  xc14_.Token().Lock();
  xc3c_.Token().Lock();
  if (!mElectric.null()) {
    mElectric->SetParticleEmission(false);
  }
  for (uint i = 0; i < 30; ++i) {
    mWingBoneIds.push_back(GetModelData()->GetAnimationData()->GetLocatorSegId(skWingBones[i]));
  }
  xae4_ = GetModelData()->ScaleCopy().GetY() *
          GetAnimationDistance(CPASAnimParmData(pas::kAS_MeleeAttack, CPASAnimParm::FromEnum(4),
                                                CPASAnimParm::FromEnum(3)));
  KnockBackCtrl().SetAnimationStateRange(kAR_Flinch, kAR_Flinch);
  KnockBackCtrl().SetEnableBurn(false);
  KnockBackCtrl().SetEnableFreeze(false);
  KnockBackCtrl().SetEnableShock(false);
  KnockBackCtrl().SetEnableLaggedBurnDeath(false);
  SetDrawShadow(false);
}

ENTITY_ACCEPT_IMPL(CRidley)

void CRidley::Touch(CActor& actor, CStateManager& mgr) { CPatterned::Touch(actor, mgr); }

void CRidley::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Registered:
    mBodyController->Activate(mgr);
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    SetupCollisionManagers(mgr);
    xcb8_ = HealthInfo(mgr)->GetHP();
    xb10_ = xcb8_;
    xcbc_ = 0.8f * xcb8_;
    break;
  case kSM_Reset:
#if VERSION >= VERSION_GM8P_00
    xa34_25_ = true;
#else
    xa34_26_ = true;
#endif
    if (GetActive()) {
      break;
    }
    CPatterned::AcceptScriptMsg(kSM_Activate, uid, mgr);
    // Fall through.
  case kSM_Activate: {
    mgr.SetBossParams(GetUniqueId(), xb1c_ + (xcb8_ + xb18_),
#if VERSION >= VERSION_GM8P_00
                      84);
#else
                      90);
#endif
    HealthInfo(mgr)->SetHP(xb1c_ + (xb10_ + xb18_));
    CPlayer& player = *mgr.Player();
    player.SetIsOverrideRadarRadius(true);
    player.SetRadarXYRadiusOverride(350.f);
    player.SetRadarZRadiusOverride(175.f);
    break;
  }
  case kSM_Deactivate:
    mBodyCollision->SetActive(mgr, false);
    mTailCollision->SetActive(mgr, false);
    mgr.Player()->SetIsOverrideRadarRadius(false);
    break;
  case kSM_Deleted:
    mBodyCollision->Destroy(mgr);
    mTailCollision->Destroy(mgr);
    if (mPlasmaProjectile != kInvalidUniqueId) {
      mgr.DeleteObjectRequest(mPlasmaProjectile);
      mPlasmaProjectile = kInvalidUniqueId;
    }
    break;
  case kSM_InitializedInArea: {
    const TUniqueId wpId = GetConnectedObject(mgr, kSS_Patrol, kSM_Follow);
    if (wpId != kInvalidUniqueId) {
      if (const CScriptWaypoint* wp = TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(wpId))) {
        xa84_ = wp->GetTransform();
        if (const CScriptWaypoint* next =
                TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(wp->NextWaypoint(mgr)))) {
          xab4_ = (next->GetTranslation() - wp->GetTranslation()).ToVec2f().Magnitude();
          xab8_ = next->GetTranslation().GetZ() - xa84_.GetTranslation().GetZ();
          if (const CScriptWaypoint* last =
                  TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(next->NextWaypoint(mgr)))) {
            xabc_ = (last->GetTranslation() - xa84_.GetTranslation()).ToVec2f().Magnitude();
            xac0_ = last->GetTranslation().GetZ() - xa84_.GetTranslation().GetZ();
            const CVector3f center = xa84_.GetTranslation();
            const CVector3f extent(xabc_, xabc_, 0.f);
            xac4_ = CAABox(center - extent - CVector3f(0.f, 0.f, 10.f),
                           center + extent + CVector3f(0.f, 0.f, 100.f));
          }
        }
      }
    }
    break;
  }
  case kSM_Damage:
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      const float damage = 1000.f - actor->HealthInfo(mgr)->GetHP();
      actor->HealthInfo(mgr)->SetHP(1000.f);
#if VERSION >= VERSION_GM8P_00
      bool interruptAttack = false;
#endif
      bool suppressKnockback = false;
      switch (mAiStage) {
      case 2:
        if (xa33_28_) {
          suppressKnockback = true;
        } else {
          bool spread = false;
          xb10_ -= damage;
          xb24_ = skDamageHitTime;
          mDamageColor = skDamageColor;
          if (xb10_ <= 0.f) {
            mAiStage = 3;
            xa31_25_ = false;
            SetStage3Immunity(mgr);
            xcbc_ = 0.6667f * mData.x3c_;
            if (mBodyController->GetLocomotionType() != pas::kLT_Combat) {
              for (uint i = 0; i < 24; ++i) {
                ModelData()->AnimationData()->SetParticleEffectState(skWingEffects[i], true, mgr);
              }
            }
            spread = true;
            xcc8_ = 2.f * skDamageHitTime;
            xb10_ = 0.f;
          } else if (xb10_ < xcbc_) {
            xa32_27_ = true;
            suppressKnockback = true;
            ++xcb0_;
            xcb0_ = static_cast< int >(xcb0_) < 5 ? xcb0_ : 4;
            spread = true;
            xcbc_ = 0.2f * static_cast< float >(5 - (static_cast< int >(xcb0_) + 1)) * xcb8_;
            xcb4_ = 0;
            xcc8_ = 2.f * skDamageHitTime;
          } else if (xa33_26_ && !xa31_31_ && damage > mData.x3f4_) {
            suppressKnockback = true;
            mBodyController->CommandMgr().DeliverCmd(
                CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_Zero));
          }
          ActivateWingElectricity(2.f * skDamageHitTime, spread);
        }
        break;
      case 3:
        if (xa32_29_) {
          const CTransform4f xf = GetLctrTransform(mHeadSegId);
          if (CVector3f::Dot(mgr.GetPlayer()->GetTranslation() - xf.GetTranslation(),
                             xf.GetForward()) < 0.5f) {
            suppressKnockback = true;
            break;
          }
        }
        TakeDamage(CVector3f::Forward(), 1.f);
        xb20_ = skDamageHitTime;
        if (xa32_29_) {
#if VERSION >= VERSION_GM8P_00
          if (mJumpCanBeInterrupted && damage >= 40.f) {
            interruptAttack = true;
            if (const CEntity* entity = mgr.GetObjectById(actor->GetLastTouchedObject())) {
              if (const CGameProjectile* projectile = TCastToConstPtr< CGameProjectile >(entity)) {
                if (projectile->GetCurrentDamageInfo().GetWeaponMode().IsCharged() &&
                    !projectile->GetUnkPalFlag()) {
                  interruptAttack = false;
                }
              }
            }
          }
#else
          if (const CEntity* entity = mgr.GetObjectById(actor->GetLastTouchedObject())) {
            if (const CGameProjectile* projectile = TCastToConstPtr< CGameProjectile >(entity)) {
              const CWeaponMode& mode = projectile->GetCurrentDamageInfo().GetWeaponMode();
              if (mode.IsCharged() || mode.IsComboed() || mode.GetType() == kWT_Missile) {
                xb14_ = 0.f;
              }
            }
          }
#endif
          xb14_ -= damage;
          mDamageColor = skFrozenDamageColor;
          if (xb14_ <= 0.f) {
            xa32_29_ = false;
            suppressKnockback = true;
#if VERSION >= VERSION_GM8P_00
            interruptAttack = false;
#endif
            mShotAt = true;
            xb14_ = mData.x38_;
          }
        } else if (xa31_27_) {
          mDamageColor = skDamageColor;
          if (xb18_ > 0.f) {
            xb18_ -= damage;
            if (xb18_ <= 0.f) {
              xa31_26_ = true;
              suppressKnockback = true;
              xb18_ = 0.f;
              xcbc_ = 0.6667f * mData.x40_;
#if VERSION >= VERSION_GM8P_00
              xcb0_ = 3;
#endif
            } else if (xb18_ < xcbc_) {
              suppressKnockback = true;
              mBodyController->CommandMgr().DeliverCmd(
                  CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_Six));
              xcbc_ -= 0.3333f * mData.x3c_;
            }
          } else {
            xb1c_ -= damage;
            if (xb1c_ <= 0.f) {
              mPendingDeath = true;
              mgr.Player()->SetIsOverrideRadarRadius(false);
              xb1c_ = 0.f;
            } else if (xb1c_ < xcbc_) {
              suppressKnockback = true;
              mBodyController->CommandMgr().DeliverCmd(
                  CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_Six));
              xcbc_ -= 0.3333f * mData.x40_;
            }
          }
        }
        break;
      }
      HealthInfo(mgr)->SetHP(xb1c_ + (xb10_ + xb18_));
#if VERSION >= VERSION_GM8P_00
      if (interruptAttack) {
        mBodyController->CommandMgr().DeliverCmd(CBCGenerateCmd(pas::kGType_Five, -1));
        mJumpCanBeInterrupted = false;
        SendScriptMsgs(kSS_Retreat, mgr, kSM_None);
      } else if (!suppressKnockback) {
#else
      if (!suppressKnockback) {
#endif
        const TUniqueId touchedId = actor->GetLastTouchedObject();
        if (const CGameProjectile* projectile =
                TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(touchedId))) {
#if VERSION >= VERSION_GM8P_00
          KnockBack(projectile->GetVelocity(), mgr, projectile->GetCurrentDamageInfo(),
                    projectile->GetCurrentDamageInfo().GetKnockBackPower(), true, false);
#else
          const float power = projectile->GetCurrentDamageInfo().GetKnockBackPower();
          const CVector3f projectilePos = projectile->GetTranslation();
          const CVector3f& direction = projectilePos - projectile->GetPreviousPos();
          KnockBack(direction, mgr, projectile->GetCurrentDamageInfo(), power, true, false);
#endif
        }
      }
    }
    break;
  case kSM_InvulnDamage:
    if (const CGameProjectile* projectile =
            TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(uid))) {
      TUniqueId target = kInvalidUniqueId;
      bool applyDamage = false;
      switch (mAiStage) {
      case 2:
        if (!xa31_31_) {
          target = mBreastPlateId;
          applyDamage = true;
        }
        break;
      case 3:
        if (xa32_29_) {
          target = mHeadId;
          applyDamage = true;
        } else if (xa31_27_) {
          target = mBreastPlateId;
          applyDamage = true;
        }
        break;
      }
      if (applyDamage) {
        CDamageInfo damage = projectile->GetCurrentDamageInfo();
        damage.SetRadius(0.f);
        mgr.ApplyDamage(
            uid, target, projectile->GetOwnerId(), damage,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
      }
    }
    break;
  case kSM_Touched:
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      if (actor->GetMaterialFilter().Passes(CMaterialList(kMT_Platform))) {
        if (const CScriptPlatform* platform = TCastToConstPtr< CScriptPlatform >(
                mgr.GetObjectById(actor->GetLastTouchedObject()))) {
          mgr.ApplyDamage(
              GetUniqueId(), platform->GetUniqueId(), GetUniqueId(),
              CDamageInfo(CWeaponMode(kWT_AI), 1.f + platform->GetHealthInfo(mgr)->GetHP(), 0.f,
                          1.f),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              CVector3f::Zero());
        }
      }
      if (actor->GetLastTouchedObject() == mgr.GetPlayer()->GetUniqueId() &&
          mCurDamageRemTime <= 0.f) {
        mgr.ApplyDamage(
            GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(), xc8c_,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        mCurDamageRemTime = mDamageWaitTime;
      }
    }
    break;
  }
}

void CRidley::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                              float dt) {
  bool handled = false;
  switch (type) {
  case kUE_BeginAction:
    if (xa32_25_ && !xa31_29_) {
      ActivateBeam(mgr);
#if VERSION >= VERSION_GM8P_00
      mHasPreviousBeamPos = false;
#endif
    }
    if (xa31_31_ && xa32_25_) {
      xbf0_ = xa84_.GetRight();
      if (CVector3f::Dot(GetTranslation() - xa84_.GetTranslation(), xbf0_) > 0.f) {
        xbf0_ *= -1.f;
      }
      xbfc_ = xbf0_;
      xbe4_ = xa84_.GetTranslation() - xabc_ * xbf0_;
      const CVector3f playerDelta = mgr.GetPlayer()->GetTranslation() - xa84_.GetTranslation();
      xbe4_ += CVector3f::Dot(playerDelta, xa84_.GetForward()) * xa84_.GetForward();
    }
    break;
  case kUE_EndAction:
    if (xa31_29_) {
      DeactivateBeam(mgr, false);
    }
    break;
  case kUE_SoundPlay:
    if (!xa32_25_) {
      CAudioSys::C3DEmitterParmData parms(1000.f, 0.1f, 1, 127, 63);
      parms.mPos = GetTranslation();
      parms.mDir = CVector3f::Zero();
      parms.mSfxId = mData.x294_;
      xcac_ = CSfxManager::AddEmitter(parms, true, CSfxManager::kMaxPriority, false);
    }
    break;
  case kUE_FadeOut:
    if (static_cast< int >(xc74_ + 1) < GetModelData()->GetNumMaterialSets()) {
      ++xc74_;
    }
    handled = true;
    break;
  case kUE_Projectile:
    switch (mAiStage) {
    case 2:
#if VERSION >= VERSION_GM8P_00
      if (mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 9)) {
#else
      if (xc14_.Token().TryCache() && mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 9)) {
#endif
        const float x = mgr.Random()->Range(-1.f, 1.f);
        const float angle = atan2f(mgr.Random()->Range(-1.f, 1.f), x);
        CTransform4f xf = GetLctrTransform(mBreastPlateSegId) *
                          CTransform4f::RotateX(CRelAngle::FromDegrees(-90.f));
#if VERSION >= VERSION_GM8P_00
        if (!xa34_27_) {
          xf = xf * CTransform4f::RotateY(CRelAngle(angle));
        }
#else
        xf = xf * CTransform4f::RotateY(CRelAngle(angle));
#endif
        const CVector3f offset(0.f, 1.f, 1.f);
        xf.SetTranslation(xf * offset);
        CEnergyProjectile* projectile = rs_new CEnergyProjectile(
            true,
#if VERSION >= VERSION_GM8P_00
            xa34_27_ ? xc14_.Token() : mProjectileInfo.Token(),
#else
            xc14_.Token(),
#endif
            kWT_AI, xf, kMT_Character,
#if VERSION >= VERSION_GM8P_00
            mProjectileInfo.GetDamage(),
#else
            xc14_.GetDamage(),
#endif
            mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(),
            mgr.GetPlayer()->GetUniqueId(), CWeapon::kPA_None, false, CVector3f(1.f, 1.f, 1.f),
            rstl::optional_object< TLockedToken< CGenDescription > >(),
            CSfxManager::kInternalInvalidSfxId, false);
        if (projectile) {
          mgr.AddObject(projectile);
          projectile->SetCameraShake(mData.xcc_);
          projectile->SetMinHomingDistance(20.f);
#if VERSION >= VERSION_GM8P_00
          xa34_27_ = false;
#endif
        }
      }
      break;
    }
    handled = true;
    break;
  case kUE_EggLay:
    if (xa32_24_) {
      if (xc3c_.Token().TryCache() && mgr.CanCreateProjectile(GetUniqueId(), kWT_AI, 8)) {
        const float x = mgr.Random()->Range(-1.f, 1.f);
        const float z = mgr.Random()->Range(-1.f, 1.f);
        const CVector3f offset(x, 1.f, z);
        const CVector3f position = GetLctrTransform(mBreastPlateSegId) * offset;
        CTransform4f xf = CTransform4f::LookAt(position, mgr.GetPlayer()->GetTranslation() +
                                                             CVector3f(10.f * x, 10.f * z, 0.f));
        xf.SetTranslation(position);
        CEnergyProjectile* projectile = rs_new CEnergyProjectile(
            true, xc3c_.Token(), kWT_AI, xf, kMT_Character, xc3c_.GetDamage(),
            mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(), kInvalidUniqueId,
            CWeapon::kPA_None, false, CVector3f(1.f, 1.f, 1.f),
            rstl::optional_object< TLockedToken< CGenDescription > >(),
            CSfxManager::kInternalInvalidSfxId, false);
        if (projectile) {
          mgr.AddObject(projectile);
          projectile->SetCameraShake(mData.x1c0_);
        }
      }
    } else if (mAiStage == 3) {
      xa31_24_ = false;
    }
#if VERSION >= VERSION_GM8P_00
    handled = true;
#endif
    break;
  case kUE_BreakLockOn:
#if VERSION >= VERSION_GM8P_00
    if (IsAlive() && mCanBreakLockOn) {
#else
    if (IsAlive()) {
#endif
      RemoveMaterial(kMT_Target, kMT_Orbit, mgr);
      mgr.Player()->TryToBreakOrbit(GetUniqueId(), CPlayer::kOB_ActivateOrbitSource, mgr);
#if VERSION < VERSION_GM8P_00
      handled = true;
#endif
    }
#if VERSION >= VERSION_GM8P_00
    handled = true;
#endif
    break;
  case kUE_Landing:
    SetVelocityWR(CVector3f::Down());
    xaec_ = CVector3f::Zero();
    xaf8_ = CVector3f::Zero();
    break;
  case kUE_ScreenShake:
    mVerticalMovement = false;
#if VERSION >= VERSION_GM8P_00
    if (mgr.GetPlayer()->GetTranslation().GetZ() - xa84_.GetTranslation().GetZ() < 1.f &&
        (mgr.GetPlayer()->GetTranslation() - GetTranslation()).Magnitude() < mData.x388_) {
#else
    if ((mgr.GetPlayer()->GetTranslation() - GetTranslation()).Magnitude() < mData.x388_) {
#endif
      mgr.ApplyDamage(
          GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(),
#if VERSION >= VERSION_GM8P_00
          mAiStage == 2 ? mData.x298_ : mData.mBounceDamage,
#else
          mData.x298_,
#endif
          CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
          CVector3f::Zero());
    }
    break;
  case kUE_IkLock:
    xa32_26_ = true;
    break;
  case kUE_IkRelease:
    xa32_26_ = false;
    break;
  case kUE_DamageOn:
    switch (mAiStage) {
    case 2:
      xa33_28_ = false;
      break;
    case 3:
      SetStage3ThroatVulnerability(mgr);
      break;
    }
    break;
  case kUE_DamageOff:
    switch (mAiStage) {
    case 2:
      if (xa33_31_) {
        xa33_28_ = true;
      }
      break;
    case 3:
      SetStage3Immunity(mgr);
      break;
    }
    break;
#if VERSION >= VERSION_GM8P_00
  case kUE_EventStart: {
    const float duration = mData.x3cc_;
    mIsMakingBigStrike = duration > 0.f;
    mDamageDur = duration;
    xc8c_ = mData.x3b0_;
    CCollisionActorManager& collision = *mBodyCollision;
    uint index = 9;
    if (node.GetLocatorName()[0] == 'L') {
      index = 8;
    }
    const TUniqueId id = collision.GetCollisionDescFromIndex(index).GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->SetExtendedTouchBounds(CVector3f(7.f, 7.f, 7.f));
    }
    break;
  }
#endif
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
}

const CDamageVulnerability* CRidley::GetDamageVulnerability() const {
  return &CDamageVulnerability::ImmuneVulnerability();
}

CAABox CRidley::GetSortingBounds(const CStateManager& mgr) const { return GetBoundingBox(); }

void CRidley::AddToRenderer(const CFrustumPlanes& frustum, const CStateManager& mgr) const {
  CPatterned::AddToRenderer(frustum, mgr);
  if (mElectric.get() && frustum.BoxInFrustumPlanes(mElectric->GetBounds())) {
    gpRender->AddParticleGen(*mElectric);
  }
}

void CRidley::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CPatterned::PreRender(mgr, frustum);
  xb2c_.PreRender(mgr, *AnimationData(), GetTransform(), GetModelScale(), *mBodyController);
  int matSet = xc74_;
  if (xcc8_ > 0.f) {
    const int count = GetModelData()->GetNumMaterialSets();
    matSet = static_cast< int >(30.f * xcc8_) % count;
  }
  const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetActiveVisor(mgr);
  const bool lastMaterial = GetModelData()->GetNumMaterialSets() == matSet + 1;
  const bool thermal = visor == CPlayerState::kPV_Thermal && lastMaterial;
  const CModelFlags& flags = GetModelFlags().UseShaderSet(matSet);
  SetModelFlags(
      CModelFlags(flags, thermal ? flags.GetOtherFlags() | CModelFlags::kF_ThermalUnsortedOnly
                                 : flags.GetOtherFlags() & ~CModelFlags::kF_ThermalUnsortedOnly));
  if (xa33_27_) {
    const float height =
        rstl::max_val(0.f, GetTranslation().GetZ() - xa84_.GetTranslation().GetZ());
    xccc_ = 1.f + CMath::Clamp(0.f, 0.05f * (height - 20.f), 1.f);
    const CVector3f& max = GetRenderBoundsCached().GetMaxPoint();
    const CVector3f& min = GetRenderBoundsCached().GetMinPoint();
    const CVector3f extents = max - min;
    const CVector3f padding = 0.5f * (xccc_ * extents - extents);
    const CAABox bounds(min - padding, max + padding);
    if ((visor == CPlayerState::kPV_Combat || visor == CPlayerState::kPV_Scan) &&
        xac4_.DoBoundsOverlap(bounds) && height > -10.f) {
      xd10_->RenderShadowBuffer(mgr, *GetModelData(), GetTransform(), lastMaterial ? 1 : 0,
                                CVector3f::Zero(), xccc_, 10.f + height);
      xd10_->SetOpacity(0.8f);
    } else {
      xd10_->Disable();
    }
  } else {
    xd10_->Disable();
  }
}

void CRidley::Render(const CStateManager& mgr) const {
  float damageDuration;
#if VERSION < VERSION_GM8P_00
  CColor damageColor = CColor::Black();
  const float damageTime0 = xb20_;
  if (damageTime0 > 0.f) {
    damageDuration = skDamageHitTime;
    damageColor = CColor::Lerp(CColor::Black(), mDamageColor, damageTime0 / damageDuration);
  }
  const CModelFlags damageFlags(CModelFlags::kT_Two, damageColor);
#endif
  CColor multiplyColor = CColor::White();
  const float damageTime1 = xb24_;
  if (damageTime1 > 0.f) {
    damageDuration = skDamageHitTime;
    multiplyColor = CColor::Lerp(CColor::White(), mDamageColor, damageTime1 / damageDuration);
  }
  gpRender->SetGXRegister1Color(multiplyColor);
  if (xa31_25_) {
    const CTransform4f xf = GetLctrTransform(mBreastPlateSegId);
    const float damageTime1 = xb24_;
    if (damageTime1 > 0.f) {
      damageDuration = skDamageHitTime;
      x9e4_.Render(mgr, xf, GetActorLights(),
                   CModelFlags(CModelFlags::kT_Two, CColor::Lerp(CColor::Black(), mDamageColor,
                                                                 damageTime1 / damageDuration)));
    } else {
      x9e4_.Render(mgr, xf, GetActorLights(), CModelFlags::Normal());
    }
  } else if (xa31_24_) {
    const CTransform4f xf = GetLctrTransform(mBreastPlateSegId);
    const float damageTime0 = xb20_;
    if (damageTime0 > 0.f) {
#if VERSION >= VERSION_GM8P_00
      CColor damageColor = CColor::Black();
      const float damageTime0 = xb20_;
      if (damageTime0 > 0.f) {
        damageDuration = skDamageHitTime;
        damageColor = CColor::Lerp(CColor::Black(), mDamageColor, damageTime0 / damageDuration);
      }
      const CModelFlags damageFlags(CModelFlags::kT_Two, damageColor);
#endif
      x998_.Render(mgr, xf, GetActorLights(), damageFlags);
    } else {
      x998_.Render(mgr, xf, GetActorLights(), CModelFlags::Normal());
    }
  }
  CPatterned::Render(mgr);
}

void CRidley::SetStage2Vulnerability(CStateManager& mgr) {
  uint i;
  for (i = 0; i < mTailCollision->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mTailCollision->GetCollisionDescFromIndex(i);
    const TUniqueId id = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerability());
      actor->HealthInfo(mgr)->SetHP(1000.f);
      actor->SetMaterialFilter(
          CMaterialFilter::MakeInclude(CMaterialList(kMT_Player, kMT_Platform)));
      actor->SetWeaponCollisionResponseType(kWCR_EnemyNormal);
    }
  }
  const CMaterialList& tailMaterials = CMaterialList(kMT_AIJoint);
  mTailCollision->AddMaterial(mgr, tailMaterials);
  for (i = 0; i < mBodyCollision->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mBodyCollision->GetCollisionDescFromIndex(i);
    const TUniqueId id = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->SetDamageVulnerability(skDirectNormal);
      actor->HealthInfo(mgr)->SetHP(1000.f);
      actor->SetMaterialFilter(
          CMaterialFilter::MakeInclude(CMaterialList(kMT_Player, kMT_Platform)));
      actor->SetWeaponCollisionResponseType(kWCR_EnemyNormal);
    }
  }
  const CMaterialList& bodyMaterials = CMaterialList(kMT_AIJoint);
  mBodyCollision->AddMaterial(mgr, bodyMaterials);
}

void CRidley::SetStage3ThroatVulnerability(CStateManager& mgr) {
  for (uint i = 0; i < mBodyCollision->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mBodyCollision->GetCollisionDescFromIndex(i);
    const TUniqueId id = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->SetDamageVulnerability(i == 3 ? skIceWeakness
                                           : CDamageVulnerability::ImmuneVulnerability());
      actor->HealthInfo(mgr)->SetHP(1000.f);
#if VERSION < VERSION_GM8P_00
      actor->SetDrawShadow(true);
#endif
    }
  }
  xa32_29_ = true;
  xa31_27_ = false;
}

void CRidley::SetStage3BreastVulnerability(CStateManager& mgr) {
  for (uint i = 0; i < mBodyCollision->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mBodyCollision->GetCollisionDescFromIndex(i);
    const TUniqueId id = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->SetDamageVulnerability(i == 2 ? skDirectNormal
                                           : CDamageVulnerability::ImmuneVulnerability());
      actor->HealthInfo(mgr)->SetHP(1000.f);
#if VERSION < VERSION_GM8P_00
      actor->SetDrawShadow(true);
#endif
    }
  }
  xa32_29_ = false;
  xa31_27_ = true;
}

void CRidley::SetStage3Immunity(CStateManager& mgr) {
#if VERSION >= VERSION_GM8P_00
  if (mInFlinch) {
    return;
  }
#endif
  for (uint i = 0; i < mBodyCollision->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mBodyCollision->GetCollisionDescFromIndex(i);
    const TUniqueId id = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerability());
      actor->HealthInfo(mgr)->SetHP(1000.f);
#if VERSION < VERSION_GM8P_00
      actor->SetDrawShadow(true);
#endif
    }
  }
  xa32_29_ = false;
  xa31_27_ = false;
}

void CRidley::ScaleCollisionSpheres(CStateManager& mgr, float scale) const {
  for (uint i = 0; i < mBodyCollision->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = mBodyCollision->GetCollisionDescFromIndex(i);
    const TUniqueId id = desc.GetCollisionActorId();
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      actor->SetSphereRadius(scale * actor->GetSphereRadius());
    }
  }
}

void CRidley::SetupCollisionManagers(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > joints;
  const CAnimData& animData = *GetModelData()->GetAnimationData();
  joints.reserve(rstl::max_val(4u, 10u));
  joints.clear();
  uint i;
  for (i = 0; i < 4; ++i) {
    const STailJoint& joint = skTailJoints[i];
#if VERSION >= VERSION_GM8P_00
    const float radius = joint.mRadius;
#else
    const float radius = joint.mRadius * GetModelData()->ScaleCopy().GetZ();
#endif
    joints.push_back(CJointCollisionDescription::OBBAutoSizeCollision(
        animData.GetLocatorSegId(rstl::string_l(joint.mFrom)),
        animData.GetLocatorSegId(rstl::string_l(joint.mTo)), CVector3f(radius, radius, radius),
        CJointCollisionDescription::kOT_One,
        GetDebugName() + rstl::string_l(" - Collision Actor ") + rstl::string_l(joint.mFrom),
        10.f));
  }
  mTailCollision =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, false);
  joints.clear();
  for (i = 0; i < 10; ++i) {
    const SBodyJoint& joint = skBodyJoints[i];
#if VERSION < VERSION_GM8P_00
    const float radius = joint.mRadius * GetModelData()->ScaleCopy().GetZ();
#endif
    joints.push_back(CJointCollisionDescription::SphereCollision(
        animData.GetLocatorSegId(rstl::string_l(joint.mName)),
#if VERSION >= VERSION_GM8P_00
        joint.mRadius,
#else
        radius,
#endif
        GetDebugName() + rstl::string_l(" - Collision Actor ") + rstl::string_l(joint.mName),
        10.f));
  }
  mBodyCollision =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), joints, false);
  mHeadId = mBodyCollision->GetCollisionDescFromIndex(3).GetCollisionActorId();
  mBreastPlateId = mBodyCollision->GetCollisionDescFromIndex(2).GetCollisionActorId();
  SetStage2Vulnerability(mgr);
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  const CMaterialList removed(kMT_Solid, kMT_CollisionActor, kMT_AIPassthrough, kMT_Player,
                              kMT_Platform);
  include.Remove(removed);
  exclude.Add(removed);
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  AddMaterial(kMT_ProjectilePassthrough, mgr);
}

CVector3f CRidley::GetAimPosition(const CStateManager& mgr, float dt) const {
  if (mAiStage == 3 && !mShotAt) {
    return GetLctrTransform(mHeadSegId).GetTranslation();
  }
  return GetLctrTransform(mBreastPlateSegId).GetTranslation();
}

void CRidley::RandomSpeedUp(CStateManager& mgr) {
  if (!xa31_24_) {
    mSpeed = 1.2f;
  }
}

void CRidley::RandomSlowDown() const {}

void CRidley::ConstrainToHenge(float dt) {
  if (mAiStage == 3 && !mVerticalMovement) {
    SetTranslation(
        CVector3f(GetTranslation().GetX(), GetTranslation().GetY(), xa84_.GetTranslation().GetZ()));
    CVector3f delta = GetTranslation() - xa84_.GetTranslation();
    const float mag = delta.Magnitude();
    delta *= 1.f / mag;
    float dot = CVector3f::Dot(xa84_.GetForward(), delta);
    dot = CMath::Clamp(-1.f, dot, 0.f);
    const float extension = -6.f * dot;
    if (mag > xab4_ + extension) {
      if (CVector3f::Dot(GetVelocityWR(), delta) > 0.f) {
        Stop();
      }
      MoveToInOneFrameWR(GetTranslation() - delta, dt);
    }
  }
}

void CRidley::ActivateBeam(CStateManager& mgr) {
  if (mPlasmaProjectile == kInvalidUniqueId) {
    CPlasmaProjectile* projectile = rs_new CPlasmaProjectile(
        xb68_.Token(), rstl::string_l(""), kWT_AI, mData.x64_, CTransform4f::Identity(),
#if VERSION >= VERSION_GM8P_00
        kMT_Player,
#else
        kMT_Character,
#endif
        xb68_.GetDamage(), mgr.AllocateUniqueId(), GetCurrentAreaId(), GetUniqueId(),
        CWeaponAssetInfo(), false, CWeapon::kPA_KeepInCinematic);
    if (projectile) {
      mgr.AddObject(*projectile);
      mPlasmaProjectile = projectile->GetUniqueId();
    }
  }
  if (CPlasmaProjectile* projectile =
          static_cast< CPlasmaProjectile* >(mgr.ObjectById(mPlasmaProjectile))) {
    projectile->Fire(GetLctrTransform(mMouthSegId), mgr, false);
    xa31_29_ = true;
    if (!xca8_) {
      CAudioSys::C3DEmitterParmData parms(1000.f, 0.1f, 1, 127, 63);
      parms.mPos = GetTranslation();
      parms.mDir = CVector3f::Zero();
      parms.mSfxId = mData.xa8_;
      xca8_ = CSfxManager::AddEmitter(parms, true, CSfxManager::kMaxPriority, true);
    }
  }
}

#if VERSION >= VERSION_GM8P_00
void CRidley::UpdateBeam(CStateManager& mgr, float dt) {
  if (CPlasmaProjectile* projectile =
          static_cast< CPlasmaProjectile* >(mgr.ObjectById(mPlasmaProjectile))) {
    if (projectile->GetActive()) {
      CTransform4f beamXf = GetLctrTransform(mMouthSegId);
      if (mAiStage == 3) {
        beamXf.SetColumn(kDY, CVector3f(beamXf.GetForward().ToVec2f().AsNormalized(), 0.f));
        beamXf.SetColumn(kDZ,
                         CVector3f::Cross(beamXf.GetRight(), beamXf.GetForward()).AsNormalized());
        beamXf.SetColumn(kDX, CVector3f::Cross(beamXf.GetForward(), beamXf.GetUp()));
      } else {
        beamXf = CTransform4f::LookAt(beamXf.GetTranslation(), xbe4_, CVector3f::Up());
        const CVector3f side = CVector3f::Cross(xbf0_, CVector3f::Up());
        const CVector3f target = mgr.GetPlayer()->GetTranslation() + CVector3f::Up() - xbe4_;
        const float speed = CVector3f::Dot(side, target) > 0.f ? -xc10_ : xc10_;
        const CQuaternion rotation = CQuaternion::ZRotation(CRelAngle::FromDegrees(dt * speed));
        const CVector3f direction = rotation.Transform(xbf0_);
        const float dot = CVector3f::Dot(xbfc_, direction);
        if (dot > 0.5f || dot > CVector3f::Dot(xbfc_, xbf0_)) {
          xbf0_ = direction;
        }
      }
      projectile->UpdateFx(beamXf, dt, mgr);

      const CVector3f currentPos = projectile->GetCurrentPos();
      const CAABox playerBounds = mgr.GetPlayer()->GetBoundingBox();
      if (mHasPreviousBeamPos && CollisionUtil::TriBoxOverlap(
                          playerBounds.GetCenterPoint(),
                          0.5f * (playerBounds.GetMaxPoint() - playerBounds.GetMinPoint()),
                          beamXf.GetTranslation(), mPreviousBeamPos, currentPos)) {
        mgr.ApplyDamage(GetUniqueId(), mgr.GetPlayer()->GetUniqueId(), GetUniqueId(),
                        xb68_.GetDamage().MakeScaledForTime(dt), CMaterialFilter::skPassEverything,
                        beamXf.GetForward());
      }

      mPreviousBeamPos = currentPos;
      mHasPreviousBeamPos = true;
    }
  }
  if (xca8_) {
    CSfxManager::UpdateEmitter(xca8_, GetTranslation(), CVector3f::Zero(), 127);
  }
}
#else
void CRidley::UpdateBeam(CStateManager& mgr, float dt) {
  if (CPlasmaProjectile* projectile =
          static_cast< CPlasmaProjectile* >(mgr.ObjectById(mPlasmaProjectile))) {
    if (projectile->GetActive()) {
      const CTransform4f mouthXf = GetLctrTransform(mMouthSegId);
      if (mAiStage == 3) {
        projectile->UpdateFx(mouthXf, dt, mgr);
      } else {
        const CTransform4f beamXf =
            CTransform4f::LookAt(mouthXf.GetTranslation(), xbe4_, CVector3f::Up());
        projectile->UpdateFx(beamXf, dt, mgr);
        const CVector3f side = CVector3f::Cross(xbf0_, CVector3f::Up());
        const CVector3f target = mgr.GetPlayer()->GetTranslation() + CVector3f::Up() - xbe4_;
        const float speed = CVector3f::Dot(side, target) > 0.f ? -xc10_ : xc10_;
        const CQuaternion rotation = CQuaternion::ZRotation(CRelAngle::FromDegrees(dt * speed));
        const CVector3f direction = rotation.Transform(xbf0_);
        const float dot = CVector3f::Dot(xbfc_, direction);
        if (dot > 0.5f || dot > CVector3f::Dot(xbfc_, xbf0_)) {
          xbf0_ = direction;
        }
      }
    }
  }
  if (xca8_) {
    CSfxManager::UpdateEmitter(xca8_, GetTranslation(), CVector3f::Zero(), 127);
  }
}
#endif

void CRidley::DeactivateBeam(CStateManager& mgr, bool reset) {
  if (CPlasmaProjectile* projectile =
          static_cast< CPlasmaProjectile* >(mgr.ObjectById(mPlasmaProjectile))) {
    projectile->ResetBeam(mgr, reset);
    xa31_29_ = false;
  }
  if (xca8_) {
    CSfxManager::RemoveEmitter(xca8_);
    xca8_.Clear();
  }
}

void CRidley::ActivateWingElectricity(float duration, bool spread) {
  if (mElectric.get()) {
    mElectric->SetParticleEmission(true);
  }
  xd08_ = spread ? 2.f * duration : duration;
  xa32_30_ = spread;
  if (!xd0c_) {
    CAudioSys::C3DEmitterParmData parms(500.f, 0.1f, 1, 127, 63);
    parms.mPos = GetTranslation();
    parms.mDir = CVector3f::Zero();
    parms.mSfxId = mData.x3f8_;
    xd0c_ = CSfxManager::AddEmitter(parms, true, CSfxManager::kMedPriority, true);
  }
}

void CRidley::UpdateWingElectricity(CStateManager& mgr, float dt) {
  if (mElectric.get()) {
    mElectric->SetGlobalOrientation(GetTransform().GetRotation());
    mElectric->SetGlobalTranslation(GetTranslation());
    mElectric->SetGlobalScale(GetModelData()->ScaleCopy());
    if (mElectric->GetParticleEmission()) {
      xd08_ -= dt;
      if (xd08_ <= 0.f) {
        mElectric->SetParticleEmission(false);
        if (xd0c_) {
          CSfxManager::RemoveEmitter(xd0c_);
          xd0c_.Clear();
        }
      } else {
        CAnimData& animData = *ModelData()->AnimationData();
        const CVector3f initial =
            animData.GetLocatorTransform(mBreastPlateSegId, nullptr).GetTranslation();
        mElectric->SetOverrideIPos(initial);
        int min = 0;
        int max = mWingBoneIds.size() - 1;
        if (!xa32_30_) {
          max -= 6;
        } else {
          min = max - 12;
        }
        const CVector3f finalPos =
            animData.GetLocatorTransform(mWingBoneIds[mgr.Random()->Range(min, max)], nullptr)
                .GetTranslation();
        mElectric->SetOverrideFPos(finalPos);
        mElectric->ForceParticleCreation(1);
      }
    }
    mElectric->Update(dt);
  } else if (xd0c_) {
    CSfxManager::RemoveEmitter(xd0c_);
    xd0c_.Clear();
  }
  if (xd0c_) {
    CSfxManager::UpdateEmitter(xd0c_, GetTranslation(), CVector3f::Zero(), 127);
  }
}

void CRidley::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  UpdateFlight(dt);
  CPatterned::Think(dt, mgr);
  ConstrainToHenge(dt);
  mBodyCollision->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  mTailCollision->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
  xb20_ = CMath::Max(0.f, xb20_ - dt);
  xb24_ = CMath::Max(0.f, xb24_ - dt);
  xcc8_ = CMath::Max(0.f, xcc8_ - dt);
  UpdateBeam(mgr, dt);
  UpdateWingElectricity(mgr, dt);
  xb2c_.Update(dt);
}

void CRidley::UpdateFlight(float dt) {
  if (IsAlive()) {
    if (xaec_.IsMagnitudeSafe()) {
      const float mag = xaec_.Magnitude();
      const CVector3f direction = (1.f / mag) * xaec_;
      float damping = 0.2f;
      if (xaf8_.MagSquared() == 0.f) {
        damping *= 3.f;
      }
      const float decay = CMath::Clamp(0.f, dt * (damping * mag), 0.5f);
      xaec_ = (mag - decay * mag) * direction;
      ApplyImpulseWR(GetMass() * xaec_, CAxisAngle::Identity());
    }
  } else {
    xaec_ = CVector3f::Zero();
  }
  xaf8_ = CVector3f::Zero();
}

void CRidley::Fly(const CVector3f& direction, float speed, float dt) {
  xaf8_ = (speed * (0.2f * speed)) * direction;
  xaec_ += dt * xaf8_;
#if VERSION >= VERSION_GM8P_00
  if (xaec_.IsMagnitudeSafe() && xaec_.Magnitude() > speed) {
#else
  if (xaec_.Magnitude() > speed) {
#endif
    xaec_ = speed * xaec_.AsNormalized();
  }
}

void CRidley::FacePlayer(CStateManager& mgr, float dt) {
#if VERSION >= VERSION_GM8P_00
  const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  if (delta.IsMagnitudeSafe()) {
    mBodyController->FaceDirection(delta.AsNormalized(), dt);
  }
#else
  mBodyController->FaceDirection(
      (mgr.GetPlayer()->GetTranslation() - GetTranslation()).AsNormalized(), dt);
#endif
}

#if VERSION >= VERSION_GM8P_00
void CRidley::PushPlayer(CStateManager& mgr) const {
  const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  const CVector3f planarDelta(delta.ToVec2f(), 0.f);
  if (planarDelta.IsMagnitudeSafe() && delta.Magnitude() < 8.f) {
    const float impulse = mgr.GetPlayer()->GetMass() * (8.f - delta.Magnitude());
    mgr.Player()->ApplyImpulseWR(impulse * planarDelta.AsNormalized(), CAxisAngle::Identity());
  }
}
#else
void CRidley::PushPlayer(CStateManager& mgr) const {
  const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  if (delta.Magnitude() < 8.f) {
    const float impulse = mgr.GetPlayer()->GetMass() * (8.f - delta.Magnitude());
    mgr.Player()->ApplyImpulseWR(impulse * CVector3f(delta.ToVec2f().AsNormalized(), 0.f),
                                 CAxisAngle::Identity());
  }
}
#endif

void CRidley::ChooseStage2Attack(CStateManager& mgr) {
  const SStage2Attack& attack = skStage2Attacks[xcb0_][xcb4_];
  xb04_ = mgr.Random()->Range(0.f, 100.f) <= attack.mProbability ? attack.mPrimary
                                                                   : attack.mAlternate;
  if (xb04_ == -1) {
    xcb4_ = 0;
    const SStage2Attack& attack = skStage2Attacks[xcb0_][xcb4_];
    xb04_ = mgr.Random()->Range(0.f, 100.f) <= attack.mProbability ? attack.mPrimary
                                                                     : attack.mAlternate;
  }
  ++xcb4_;
  xcc4_ = 1;
}

void CRidley::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    ChooseStage2Attack(mgr);
    xa32_27_ = false;
    xa33_26_ = true;
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CRidley::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Taunt, static_cast< FTryCommandCallback >(&CRidley::TryTaunt), 3);
    FacePlayer(mgr, arg);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

void CRidley::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
#if VERSION >= VERSION_GM8P_00
    xa34_25_ = false;
#else
    xa34_26_ = false;
#endif
    SetTranslation(xa84_ * CVector3f(0.f, xabc_, xac0_ - xadc_));
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_NextState));
#if VERSION >= VERSION_GM8P_00
    xa32_31_ = false;
#endif
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CRidley::SpecialAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Taunt, static_cast< FTryCommandCallback >(&CRidley::TryTaunt), 0);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

void CRidley::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    xbe4_ = mgr.GetPlayer()->GetTranslation() + CVector3f::Up() - 8.f * GetTransform().GetRight();
    xbf0_ = GetTransform().GetRight();
    xbfc_ = xbf0_;
    xc08_ = 0.f;
    xc0c_ = 0.f;
    xc10_ = 240.f;
    mAnimState = kAS_Ready;
    xa32_25_ = true;
    --xcc4_;
    break;
  case kStateMsg_Update:
    if (xa31_29_) {
      const float maxSpeed = 0.5f;
      xc0c_ = CMath::Min(maxSpeed, 0.5f * arg + xc0c_);
      xc08_ = arg * xc0c_ + xc08_;
      xbe4_ += xc08_ * xbf0_;
    } else {
      FacePlayer(mgr, arg);
    }
    TryCommand(mgr, pas::kAS_ProjectileAttack,
               static_cast< FTryCommandCallback >(&CRidley::TryProjectileAttack), 0);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    DeactivateBeam(mgr, true);
    xa32_25_ = false;
    break;
  }
}

void CRidley::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    SetStage3ThroatVulnerability(mgr);
    mAnimState = kAS_Ready;
    xa32_25_ = true;
    RandomSpeedUp(mgr);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_ProjectileAttack,
               static_cast< FTryCommandCallback >(&CRidley::TryProjectileAttack),
#if VERSION >= VERSION_GM8P_00
               xa34_29_ ? 1 : 0);
#else
               0);
#endif
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    DeactivateBeam(mgr, true);
    SetStage3Immunity(mgr);
    xa32_25_ = false;
    RandomSlowDown();
#if VERSION >= VERSION_GM8P_00
    xa34_29_ = true;
#endif
    break;
  }
}

void CRidley::Flee(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CVector3f delta = GetTranslation() - xa84_.GetTranslation();
    const CVector3f direction = delta.AsNormalized();
    CVector3f dest = xa84_.GetTranslation() + (40.f + xabc_) * direction;
    dest.SetZ(xac0_ + xa84_.GetTranslation().GetZ());
    SetDestPos(dest);
    break;
  }
  case kStateMsg_Update: {
#if VERSION >= VERSION_GM8P_00
    CVector3f direction = mDestPos - GetTranslation();
    if (direction.IsMagnitudeSafe()) {
      direction.Normalize();
      Fly(direction, 50.f, arg);
      mBodyController->FaceDirection(direction, arg);
    }
#else
    const CVector3f direction = (mDestPos - GetTranslation()).AsNormalized();
    Fly(direction, 50.f, arg);
    mBodyController->FaceDirection(direction, arg);
#endif
    break;
  }
  case kStateMsg_Deactivate:
    break;
  }
}

void CRidley::Retreat(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Step, static_cast< FTryCommandCallback >(&CRidley::TryDodge), 5);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

void CRidley::Lurk(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    if (!xa33_25_) {
      CVector3f delta = GetTranslation() - xa84_.GetTranslation();
      delta.SetZ(0.f);
      float angle = CVector3f::GetAngleDiff(xa84_.GetForward(), delta);
      if (CVector3f::Dot(delta, xa84_.GetRight()) < 0.f) {
        angle = -angle;
      }
      for (int i = 0; i < 1; ++i) {
        if (CMath::AbsF(mgr.Random()->Range(-1.2566371f, 1.2566371f) - angle) < 0.39269909f) {
          continue;
        }
        break;
      }
    } else {
      mBodyCollision->SetActive(mgr, true);
      mTailCollision->SetActive(mgr, true);
    }
    xa33_25_ = false;
    const float height = xac0_ - xadc_;
    const float y = xabc_ * CMath::FastCosR(0.f);
    const float x = xabc_ * CMath::FastSinR(0.f);
    const CVector3f offset(x, y, height);
    SetDestPos(xa84_ * offset);
    const CVector3f forward = CVector3f(GetTransform().GetForward().ToVec2f(), 0.f).AsNormalized();
    const CVector3f right = CVector3f::Cross(forward, CVector3f::Up());
    SetTransform(CTransform4f::FromColumns(right, forward, CVector3f::Up(), GetTranslation()));
    xa33_27_ = false;
#if VERSION >= VERSION_GM8P_00
    xa34_25_ = false;
#else
    xa34_26_ = false;
#endif
    break;
  }
  case kStateMsg_Update:
#if VERSION >= VERSION_GM8P_00
    const CVector3f delta = mDestPos - GetTranslation();
    if (delta.IsMagnitudeSafe()) {
      Fly(delta.AsNormalized(), 50.f, arg);
    }
#else
    Fly((mDestPos - GetTranslation()).AsNormalized(), 50.f, arg);
#endif
    FacePlayer(mgr, 10.f * arg);
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CRidley::Enraged(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimState = kAS_Ready;
    AddMaterial(kMT_Orbit, kMT_Target, mgr);
    const CVector3f direction(
        (mgr.GetPlayer()->GetTranslation() - GetTranslation()).ToVec2f().AsNormalized(), 0.f);
    CVector3f position = mgr.GetPlayer()->GetTranslation() - 20.f * direction;
    position.SetZ(xa84_.GetTranslation().GetZ());
    const CVector3f right = CVector3f::Cross(direction, CVector3f::Up());
    CEntity* explosion = rs_new CExplosion(
        x98c_, mgr.AllocateUniqueId(), true,
        CEntityInfo(GetCurrentAreaId(), CEntity::NullConnectionList), rstl::string_l(""),
        CTransform4f::FromColumns(right, direction, CVector3f::Up(), position), 0,
        CVector3f(1.f, 1.f, 1.f), CColor::White());
    if (explosion) {
      mgr.AddObject(*explosion);
    }
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Step, static_cast< FTryCommandCallback >(&CRidley::TryDodge), 4);
    FacePlayer(mgr, arg);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    xa33_27_ = true;
    break;
  }
}

void CRidley::LoopedAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    if (xa32_31_) {
      xa33_24_ = false;
      xbc4_ = GetTranslation();
      xbd0_ = GetTransform().GetForward();
#if VERSION >= VERSION_GM8P_00
      mReflectedDestPos = GetTranslation();
#endif
    } else {
      xa33_24_ = CVector3f::Dot(xa84_.GetRight(), GetTranslation() - xa84_.GetTranslation()) < 0.f;
      const rstl::vector< SConnection >& connections = GetConnectionList();
      for (AUTO(it, connections.begin()); it != connections.end(); ++it) {
        if (it->mState != kSS_Attack || it->mMsg != kSM_Follow) {
          continue;
        }
        const TUniqueId uid = mgr.GetIdForScript(it->mObjId);
        if (uid == kInvalidUniqueId) {
          continue;
        }
        if (const CScriptWaypoint* waypoint =
                TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(uid))) {
          const float side =
              CVector3f::Dot(xa84_.GetRight(), waypoint->GetTranslation() - xa84_.GetTranslation());
          if ((xa33_24_ && side > 0.f) || (!xa33_24_ && side <= 0.f)) {
            xbc4_ = waypoint->GetTranslation();
            xbd0_ = waypoint->GetTransform().GetForward();
            mReflectedDestPos = GetTranslation();
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
    xcc0_ = skStage2Settings[xcb0_].x1c_;
    xc68_ = GetModelData()->ScaleCopy();
    ScaleCollisionSpheres(mgr, 2.f);
    break;
  case kStateMsg_Update: {
    const CVector3f delta = GetTranslation() - xa84_.GetTranslation();
    const float distance = delta.Magnitude();
    const float t = CMath::Clamp(0.f, distance - 100.f, skFlightScaleDistance) / 250.f;
    float scale = 1.f - 0.7f * ((3.f - 2.f * t) * (t * t));
    scale = CMath::Clamp(skMinFlightScale, scale, 1.f);
    const float inverseScale = 1.f / scale;
    const CVector3f modelScale = scale * xc68_;
    ModelData()->SetScale(modelScale);
    mMoveScale = CVector3f(inverseScale, inverseScale, inverseScale);
    x9e4_.SetScale(modelScale);
    if (mStateMachineState.GetTime() > 1.f && skStage2Settings[xcb0_].x20_ != 0) {
      xa32_25_ = true;
    }
    if (xa32_25_ && xa31_29_) {
      xbe4_ += (90.f * arg) * xbf0_;
    }
    if (xa32_24_ && xcac_) {
      CSfxManager::UpdateEmitter(xca8_, GetTranslation(), CVector3f::Zero(), 127);
    }
    if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
      return;
    }
    if (!xa31_30_) {
      CVector3f direction = xbc4_ - GetTranslation();
      const float distance = direction.Magnitude();
#if VERSION >= VERSION_GM8P_00
      const float dot = CVector3f::Dot(direction, xbc4_ - mReflectedDestPos);
      if (distance > 2.f && dot > 0.f) {
#else
      if (distance > 2.f && CVector3f::Dot(direction, xbc4_ - mReflectedDestPos) > 0.f) {
#endif
        direction *= 1.f / distance;
        xbdc_ += CMath::Clamp(-100.f * arg, xbe0_ - xbdc_, 100.f * arg);
        Fly(direction, xbdc_, arg);
        const float turnRate = CMath::Clamp(1.f, 10.f / distance, 10.f);
        mBodyController->FaceDirection(xbd0_, arg * turnRate);
      } else {
        const pas::EStepDirection baseStep = xa32_31_ ? pas::kSD_Left : pas::kSD_Right;
        pas::EStepDirection step = baseStep;
        switch (xcc0_) {
        case 2:
          step = pas::kSD_Forward;
          break;
        case 3:
          step = pas::kSD_Up;
          break;
        }
        const pas::EStepType type = xa33_24_ ? pas::kStep_BreakDodge : pas::kStep_Normal;
        mBodyController->CommandMgr().DeliverCmd(CBCStepCmd(step, type));
        xa31_30_ = true;
      }
    } else {
      mStateMachineState.SetCodeTrigger();
    }
    break;
  }
  case kStateMsg_Deactivate:
    mBodyController->SetLocomotionType(pas::kLT_Relaxed);
    if (xa32_25_) {
      DeactivateBeam(mgr, true);
    }
    if (xa32_24_) {
      xcac_.Clear();
    }
    xa32_24_ = false;
    xa32_25_ = false;
    ModelData()->SetScale(xc68_);
    x9e4_.SetScale(xc68_);
    xa31_31_ = false;
    mMoveScale = CVector3f(1.f, 1.f, 1.f);
    SetStage2Vulnerability(mgr);
    ScaleCollisionSpheres(mgr, 0.5f);
    xa32_31_ = false;
    break;
  }
}

void CRidley::Explode(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    xa32_27_ = false;
    mAnimState = kAS_Ready;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_KnockBack, static_cast< FTryCommandCallback >(&CRidley::TryKnockBack),
               2);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

void CRidley::Approach(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CVector3f delta = GetTranslation() - xa84_.GetTranslation();
    const CVector3f direction = delta.AsNormalized();
    CVector3f dest = xa84_.GetTranslation() + xab4_ * direction;
    dest.SetZ(xae8_ + xa84_.GetTranslation().GetZ() - 1.f);
    SetDestPos(dest);
    xa33_26_ = false;
    if (mAiStage == 3 && !xa34_24_) {
      xa34_24_ = true;
      SendScriptMsgs(kSS_CameraPath, mgr, kSM_None);
    }
    break;
  }
  case kStateMsg_Update:
#if VERSION >= VERSION_GM8P_00
    const CVector3f delta = mDestPos - GetTranslation();
    if (delta.IsMagnitudeSafe()) {
      Fly(delta.AsNormalized(), 50.f, arg);
    }
#else
    Fly((mDestPos - GetTranslation()).AsNormalized(), 50.f, arg);
#endif
    FacePlayer(mgr, arg);
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CRidley::Land(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    CVector3f delta = mgr.GetPlayer()->GetTranslation() - xa84_.GetTranslation();
    delta.SetZ(0.f);
    const float distance = CMath::Min(5.f + xab4_, delta.Magnitude());
    SetDestPos(xa84_.GetTranslation() + distance * delta.Normalize());
    mAnimState = kAS_Ready;
    AddMaterial(kMT_Solid, mgr);
    CMaterialList exclude = GetMaterialFilter().GetExcludeList();
    exclude.Remove(kMT_Solid);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        GetMaterialFilter().GetIncludeList().Union(CMaterialList(kMT_Solid)), exclude));
    if (mAiStage == 3) {
      AddMaterial(kMT_GroundCollider, kMT_Solid, mgr);
    }
    const float duration = mData.x38c_;
    mIsMakingBigStrike = duration > 0.f;
    mDamageDur = duration;
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack,
               static_cast< FTryCommandCallback >(&CRidley::TryMeleeAttack_TargetPos), 8);
    if (mAnimState == kAS_Repeat) {
      mBodyController->SetLocomotionType(pas::kLT_Combat);
    }
    PushPlayer(mgr);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    mIsMakingBigStrike = false;
    mDamageDur = 0.f;
    xa33_30_ = false;
    mDoStrafe = false;
    if (mgr.Random()->Range(0.f, 100.f) < 50.f) {
      if (CVector3f::Dot(mgr.GetPlayer()->GetTranslation() - GetTranslation(),
                         GetTransform().GetForward()) > 0.f) {
        xa33_30_ = true;
      } else {
        mDoStrafe = true;
      }
    }
    break;
  }
}

void CRidley::JumpBack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimState = kAS_Ready;
    SetMomentumWR(CVector3f::Zero());
    mVerticalMovement = true;
    SetDestPos(xa84_.GetTranslation() + xabc_ * xa84_.GetForward() + CVector3f(0.f, 0.f, xac0_));
    RemoveMaterial(kMT_Solid, mgr);
    CMaterialList include = GetMaterialFilter().GetIncludeList();
    include.Remove(kMT_Solid);
    SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
        include, GetMaterialFilter().GetExcludeList().Union(CMaterialList(kMT_Solid))));
    xa32_26_ = false;
    const CVector3f delta = mDestPos - GetTranslation();
    xc7c_ = CVector2f::GetAngleDiff(GetTransform().GetForward().ToVec2f(), delta.ToVec2f());
    if (CVector3f::Dot(GetTransform().GetRight(), delta) > 0.f) {
      xc7c_ = -xc7c_;
    }
    xc78_ = 0.f;
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Generate,
               static_cast< FTryCommandCallback >(&CRidley::TryGenerateDeactivate), 4);
    switch (mAnimState) {
    case kAS_Repeat:
      mBodyController->SetLocomotionType(pas::kLT_Relaxed);
      if (xc78_ == 0.f) {
        const CCharAnimTime lock =
            ModelData()->AnimationData()->GetTimeOfUserEvent(kUE_IkLock, CCharAnimTime::Infinity());
        const CCharAnimTime release = ModelData()->AnimationData()->GetTimeOfUserEvent(
            kUE_IkRelease, CCharAnimTime::Infinity());
        if (lock != CCharAnimTime::Infinity() && release != CCharAnimTime::Infinity()) {
          xc78_ = release.GetSeconds() - lock.GetSeconds();
        }
      }
      if (xa32_26_) {
        const CQuaternion rotation = CQuaternion::ZRotation(CRelAngle(xc7c_ * arg / xc78_));
        RotateInOneFrameOR(rotation, arg);
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

#if VERSION >= VERSION_GM8P_00
void CRidley::TurnAround(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    break;
  case kStateMsg_Update:
    if (mBodyController->GetCurrentStateId() != pas::kAS_Turn) {
      const CVector3f direction =
          (mgr.GetPlayer()->GetTranslation() - GetTranslation()).AsNormalized();
      if (CVector3f::Dot(direction, GetTransform().GetForward()) < 0.9f) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), direction, 1.f));
      }
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}
#endif

void CRidley::Crouch(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    SetMomentumWR(GetGravityConstant() * CVector3f::Down());
    if (mAiStage == 3) {
      ChooseStage3Attack(mgr);
    }
    break;
  case kStateMsg_Update:
    if (mBodyController->GetCurrentStateId() != pas::kAS_Turn) {
      const CVector3f direction =
          (mgr.GetPlayer()->GetTranslation() - GetTranslation()).AsNormalized();
      if (CVector3f::Dot(direction, GetTransform().GetForward()) < 0.9f) {
        mBodyController->CommandMgr().DeliverCmd(
            CBCLocomotionCmd(CVector3f::Zero(), direction, 1.f));
      }
    }
    break;
  case kStateMsg_Deactivate:
#if VERSION >= VERSION_GM8P_00
    xa34_29_ = false;
#endif
    break;
  }
}

void CRidley::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    --xcc4_;
    xa33_31_ = true;
#if VERSION >= VERSION_GM8P_00
    xa34_27_ = true;
#endif
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_ProjectileAttack,
               static_cast< FTryCommandCallback >(&CRidley::TryProjectileAttack), 2);
    FacePlayer(mgr, arg);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    xa33_28_ = false;
    xa33_31_ = false;
    break;
  }
}

bool CRidley::CanJumpAttack() const {
  const CVector3f forward = (0.5f * xae4_) * GetTransform().GetForward();
  const CVector3f target = GetTranslation() + forward;
  const CVector3f delta = target - xa84_.GetTranslation();
  const bool ret = delta.Magnitude() < 0.5f * (xab4_ + xabc_);
  return ret;
}

#if VERSION >= VERSION_GM8P_00
void CRidley::ChooseStage3Attack(CStateManager& mgr) {
  const SStage2Attack& attack = skStage3Attacks[xcb0_][xcb4_];
  xb0c_ = mgr.Random()->Range(0.f, 100.f) <= attack.mProbability ? attack.mPrimary
                                                                   : attack.mAlternate;
  if (xb0c_ == -1) {
    xcb4_ = 0;
    const SStage2Attack& attack = skStage3Attacks[xcb0_][xcb4_];
    xb0c_ = mgr.Random()->Range(0.f, 100.f) <= attack.mProbability ? attack.mPrimary
                                                                     : attack.mAlternate;
  }
  CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  const float distance = delta.Magnitude();
  delta *= 1.f / distance;
  const float dot = CVector3f::Dot(delta, GetTransform().GetForward());
  bool adjusted = false;
  if (dot < 0.f && distance < mMaxAttackRange) {
    adjusted = true;
    xb0c_ = 3;
  }
  if (dot > 0.f && distance < mMinAttackRange && xb08_ != 4) {
    adjusted = true;
    xb0c_ = 4;
  }
  switch (xb0c_) {
  case 2:
  case 7:
    if (dot < 0.5f) {
      adjusted = true;
      xb0c_ = 0;
    }
    break;
  case 5:
    if (dot < 0.5f) {
      adjusted = true;
      xb0c_ = 0;
    } else if (!CanJumpAttack()) {
      const SStage2Attack& fallback = skStage3Attacks[xcb0_][xcb4_];
      xb0c_ = fallback.mAlternate;
    }
    break;
  case 6:
    if (dot < 0.f) {
      adjusted = true;
      xb0c_ = 0;
    }
    break;
  }
  if (!adjusted) {
    ++xcb4_;
  }
  xb08_ = xb0c_;
}
#else
void CRidley::ChooseStage3Attack(CStateManager& mgr) {
  xb08_ = xb0c_;
  const float roll = 100.f * mgr.Random()->Float();
  float threshold = 0.f;
  for (int i = 0; i < 6; ++i) {
    threshold += skStage3AttackWeights[xb08_][i];
    if (roll < threshold) {
      xb0c_ = i;
      break;
    }
  }
  if (xb0c_ == 5 && !CanJumpAttack()) {
    xb0c_ = 2;
  }
  CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  const float distance = delta.Magnitude();
  delta *= 1.f / distance;
  const float dot = CVector3f::Dot(delta, GetTransform().GetForward());
  if (xb0c_ == 2 && dot < 0.5f) {
    xb0c_ = 0;
  }
  if (xb0c_ == 5 && dot < 0.8f) {
    xb0c_ = 0;
  }
  if (dot < 0.f && distance < mMaxAttackRange && xb08_ != 3) {
    xb0c_ = 3;
  }
  if (dot > 0.f && distance < mMinAttackRange && xb08_ != 4) {
    xb0c_ = 4;
  }
}
#endif

void CRidley::FadeOut(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mAnimState = kAS_Ready;
    for (uint i = 0; i < 24; ++i) {
      ModelData()->AnimationData()->SetParticleEffectState(skWingEffects[i], false, mgr);
    }
    if (!xa34_24_) {
      xa34_24_ = true;
    }
    xb68_.SetDamage(mData.x3fc_);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_KnockBack, static_cast< FTryCommandCallback >(&CRidley::TryKnockBack),
               5);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    xcbc_ = 0.6667f * mData.x3c_;
#if VERSION >= VERSION_GM8P_00
    xcb0_ = 0;
    xcb4_ = 0;
#endif
    break;
  }
}

bool CRidley::ShouldStrafe(CStateManager& mgr, float arg) { return mDoStrafe; }

bool CRidley::ShouldDoubleSnap(CStateManager& mgr, float arg) { return xa33_30_; }

void CRidley::DoubleSnap(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimState = kAS_Ready;
#if VERSION >= VERSION_GM8P_00
    const float distance = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).Magnitude();
    mMeleeAttack = distance < 0.66f * mMinAttackRange ? 2 : 7;
#else
    const float duration = mData.x3cc_;
    mIsMakingBigStrike = duration > 0.f;
    mDamageDur = duration;
    xc8c_ = mData.x3b0_;
#endif
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack,
               static_cast< FTryCommandCallback >(&CRidley::TryMeleeAttack),
#if VERSION >= VERSION_GM8P_00
               mMeleeAttack);
#else
               2);
#endif
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    mIsMakingBigStrike = false;
    mDamageDur = 0.f;
    xc8c_ = GetContactDamage();
#if VERSION >= VERSION_GM8P_00
    for (int i = 8; i <= 9; ++i) {
      const TUniqueId id = mBodyCollision->GetCollisionDescFromIndex(i).GetCollisionActorId();
      if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
        actor->SetExtendedTouchBounds(CVector3f::Zero());
      }
    }
#endif
    break;
  }
}

void CRidley::Jump(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimState = kAS_Ready;
    const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    if (delta.Magnitude() > xae4_) {
      xc88_ = 5;
    } else {
      xc88_ = 4;
    }
    const float duration = mData.x3ac_;
    mIsMakingBigStrike = duration > 0.f;
    mDamageDur = duration;
    xc8c_ = mData.x390_;
    RandomSpeedUp(mgr);
#if VERSION >= VERSION_GM8P_00
    mJumpCanBeInterrupted = true;
    mCanBreakLockOn = true;
#endif
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack,
               static_cast< FTryCommandCallback >(&CRidley::TryMeleeAttack), xc88_);
    if (mStateMachineState.GetTime() < 1.75f) {
      FacePlayer(mgr, arg);
    }
    break;
  case kStateMsg_Deactivate:
    AddMaterial(kMT_Orbit, kMT_Target, mgr);
#if VERSION >= VERSION_GM8P_00
    mCanBreakLockOn = false;
#endif
    mIsMakingBigStrike = false;
    mDamageDur = 0.f;
    xc8c_ = GetContactDamage();
    mAnimState = kAS_NotReady;
#if VERSION < VERSION_GM8P_00
    xa34_25_ = false;
#endif
    RandomSlowDown();
#if VERSION >= VERSION_GM8P_00
    mJumpCanBeInterrupted = false;
#endif
    break;
  }
}

void CRidley::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    SetStage3BreastVulnerability(mgr);
    mAnimState = kAS_Ready;
#if VERSION >= VERSION_GM8P_00
    mInFlinch = true;
#endif
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_KnockBack, static_cast< FTryCommandCallback >(&CRidley::TryKnockBack),
               3);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
#if VERSION >= VERSION_GM8P_00
    mInFlinch = false;
#endif
    SetStage3Immunity(mgr);
    mShotAt = false;
#if VERSION >= VERSION_GM8P_00
    xcb4_ = 0;
    if (xcb0_ < 3) {
      ++xcb0_;
      if (xcb0_ == 3) {
        xcb0_ = 1;
      }
    }
#endif
    break;
  }
}

void CRidley::Hurled(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    SetStage3Immunity(mgr);
    mBodyController->CommandMgr().DeliverCmd(
        CBCKnockBackCmd(GetTransform().GetForward(), pas::kS_Four));
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CRidley::Dodge(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CVector3f delta = GetTranslation() - xa84_.GetTranslation();
    const CVector3f direction = CVector3f(delta.ToVec2f(), 0.f).AsNormalized();
    float chance = 2.f * CVector3f::Dot(direction, xa84_.GetRight()) + 1.f;
    chance = CMath::Clamp(0.f, chance, 1.f);
    xc84_ = 2;
    if (mgr.Random()->Float() < chance) {
      xc84_ = 3;
    }
    mAnimState = kAS_Ready;
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Step, static_cast< FTryCommandCallback >(&CRidley::TryDodge), xc84_);
    if (mAnimState == kAS_Over) {
      const CVector3f delta = GetTranslation() - xa84_.GetTranslation();
      const CVector3f direction = CVector3f(delta.ToVec2f(), 0.f).AsNormalized();
      CVector3f dest = xabc_ * direction;
      dest.SetZ(xac0_);
      dest = xa84_.GetTranslation() + dest;
      CVector3f movement = dest - GetTranslation();
      if (movement.Magnitude() > 1.f) {
        movement.Normalize();
      }
      Fly(movement, 10.f, arg);
    }
    mBodyController->FaceDirection((xa84_.GetTranslation() - GetTranslation()).AsNormalized(),
                                       arg);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    break;
  }
}

void CRidley::CoverAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mAnimState = kAS_Ready;
    const float duration = mData.x3ec_;
    mIsMakingBigStrike = duration > 0.f;
    mDamageDur = duration;
    xc8c_ = mData.x3d0_;
    RandomSpeedUp(mgr);
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack,
               static_cast< FTryCommandCallback >(&CRidley::TryMeleeAttack), 1);
    break;
  case kStateMsg_Deactivate:
    mAnimState = kAS_NotReady;
    mIsMakingBigStrike = false;
    mDamageDur = 0.f;
    xc8c_ = GetContactDamage();
    RandomSlowDown();
    break;
  }
}

void CRidley::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mgr.SetBossParams(kInvalidUniqueId, 0.f, 0);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

bool CRidley::ShouldTaunt(CStateManager& mgr, float arg) {
  bool ret = false;
  switch (mAiStage) {
  case 2:
    ret = xb04_ == 4;
    break;
  case 3:
    ret = xb0c_ == 1;
    break;
  }
  return ret;
}

bool CRidley::ShouldFire(CStateManager& mgr, float arg) {
  bool ret = false;
  switch (mAiStage) {
  case 2:
    ret = xb04_ == 2;
    break;
  }
  return ret;
}

bool CRidley::ShouldAttack(CStateManager& mgr, float arg) {
  bool ret = false;
  switch (mAiStage) {
  case 2:
    ret = xb04_ == 0;
    break;
  case 3:
#if VERSION >= VERSION_GM8P_00
    ret = xb0c_ == 2 || xb0c_ == 7;
#else
    ret = xb0c_ == 2;
#endif
    break;
  }
  return ret;
}

bool CRidley::AIStage(CStateManager& mgr, float arg) {
  return mAiStage >= static_cast< int >(arg);
}

bool CRidley::InRange(CStateManager& mgr, float arg) {
  const CVector3f& pos = GetTranslation();
  const CVector3f delta = pos - mDestPos;
  return delta.Magnitude() < 2.f;
}

bool CRidley::ShouldCrouch(CStateManager& mgr, float arg) { return xb04_ == 1; }

bool CRidley::ShouldRetreat(CStateManager& mgr, float arg) {
#if VERSION >= VERSION_GM8P_00
  return xa34_25_;
#else
  return xa34_26_;
#endif
}

bool CRidley::ShouldDodge(CStateManager& mgr, float arg) { return xb04_ == 3; }

bool CRidley::ShouldTurn(CStateManager& mgr, float arg) { return xb04_ == 5; }

bool CRidley::AttackOver(CStateManager& mgr, float arg) { return xcc4_ == 0; }

bool CRidley::TooClose(CStateManager& mgr, float arg) {
  if (xb0c_ == 4) {
    return true;
  }
  if (xb08_ != 4) {
    const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    const float distance = delta.Magnitude();
    if (distance < mMinAttackRange &&
        CVector3f::Dot(delta, GetTransform().GetForward()) > 0.7f * distance) {
      xb0c_ = 4;
      return true;
    }
  }
  return false;
}

bool CRidley::ShouldMove(CStateManager& mgr, float arg) {
#if VERSION >= VERSION_GM8P_00
  return xb0c_ == 5;
#else
  if (xb0c_ == 5) {
    xa34_25_ = true;
    return true;
  }
  if (xb08_ != 5) {
    const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    const float distance = delta.Magnitude();
    if (distance > mMaxAttackRange &&
        CVector3f::Dot(delta, GetTransform().GetForward()) > 0.8f * distance && CanJumpAttack()) {
      xa34_25_ = true;
      xb0c_ = 5;
      return true;
    }
  }
  return false;
#endif
}

bool CRidley::IsDizzy(CStateManager& mgr, float arg) {
#if VERSION >= VERSION_GM8P_00
  return xb0c_ == 3;
#else
  if (xb0c_ == 3) {
    return true;
  }
  if (xb08_ != 3) {
    const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
    if (delta.Magnitude() < mMaxAttackRange &&
        CVector3f::Dot(delta, GetTransform().GetForward()) < 0.f) {
      xb0c_ = 3;
      return true;
    }
  }
  return false;
#endif
}

bool CRidley::ShotAt(CStateManager& mgr, float arg) { return mShotAt; }

bool CRidley::Attacked(CStateManager& mgr, float arg) { return xa31_24_ && xa31_26_; }

bool CRidley::HitSomething(CStateManager& mgr, float arg) { return xa32_27_ || mAiStage == 3; }

#if VERSION >= VERSION_GM8P_00
bool CRidley::BounceFind(CStateManager& mgr, float arg) { return xb0c_ == 6; }

void CRidley::Bounce(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    CVector3f delta = mgr.GetPlayer()->GetTranslation() - xa84_.GetTranslation();
    delta.SetZ(0.f);
    const float distance = CMath::Min(15.f + xab4_, delta.Magnitude());
    SetDestPos(xa84_.GetTranslation() + distance * delta.Normalize());
    mAnimState = kAS_Ready;
    const float duration = mData.x38c_;
    mIsMakingBigStrike = duration > 0.f;
    mDamageDur = duration;
    break;
  }
  case kStateMsg_Update:
    if (mStateMachineState.GetTime() < 1.f) {
      FacePlayer(mgr, arg);
    }
    TryCommand(mgr, pas::kAS_MeleeAttack,
               static_cast< FTryCommandCallback >(&CRidley::TryMeleeAttack_TargetPos), 8);
    break;
  case kStateMsg_Deactivate:
    mIsMakingBigStrike = false;
    mDamageDur = 0.f;
    mAnimState = kAS_NotReady;
    break;
  }
}

bool CRidley::SpotPlayer(CStateManager& mgr, float arg) {
  const float dot = CMath::FastCosR(CMath::Deg2Rad(0.5f * arg));
  const CVector3f delta = mgr.GetPlayer()->GetTranslation() - GetTranslation();
  return CVector3f::Dot(delta.AsNormalized(), GetTransform().GetForward()) > dot;
}

bool CRidley::AggressionCheck(CStateManager& mgr, float arg) { return xb0c_ == 7; }
#endif

bool CRidley::SetAIStage(CStateManager& mgr, float arg) {
  mAiStage = arg;
  return true;
}

EWeaponCollisionResponseTypes CRidley::GetCollisionResponseType(const CVector3f&, const CVector3f&,
                                                                const CWeaponMode&, int) const {
  return kWCR_EnemyNormal;
}

float CRidley::GetGravityConstant() const { return 50.f; }

CRidley::~CRidley() {}
