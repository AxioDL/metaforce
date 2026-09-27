#include "MetroidPrime/Enemies/CThardus.hpp"

#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/Math/CRelAngle.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Text/CStringTable.hpp"
#include "MetroidPrime/CActorParameters.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CCollisionActor.hpp"
#include "MetroidPrime/CCollisionActorManager.hpp"
#include "MetroidPrime/CGameArea.hpp"
#include "MetroidPrime/CGameLight.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Cameras/CCameraShakeData.hpp"
#include "MetroidPrime/Cameras/CFirstPersonCamera.hpp"
#include "MetroidPrime/Enemies/CPatternedInfo.hpp"
#include "MetroidPrime/Enemies/CThardusRockProjectile.hpp"
#include "MetroidPrime/HUD/CSamusHud.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/ScriptObjects/CScriptDistanceFog.hpp"
#include "MetroidPrime/ScriptObjects/CScriptTimer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Weapons/CBomb.hpp"
#include "MetroidPrime/Weapons/CGameProjectile.hpp"
#include "MetroidPrime/Weapons/CIceAttackProjectile.hpp"

#include "Kyoto/Particles/CParticleData.hpp"
#include "MetroidPrime/ScriptObjects/CRepulsor.hpp"
#include "rstl/StringExtras.hpp"
#include <float.h>

const char* const CThardus::skHeadRockNameStr = "Neck_1";
static const float skMinAttackTime = 0.75f;
static const float skThermalFlashFadeInTime = 0.25f;
static const float skThermalFlashFadeOutTime = 2.f;

const char* const CThardus::skDamageableRockJointNameList[7] = {
    "R_knee",           "R_forearm", "L_elbow", "L_hip", "R_collar_BigRock_SDK",
    "collar_rock4_SDK", "Neck_1"};
const char* const CThardus::skDamageableRockCollisionJointNameList[7] = {
    "R_knee",
    "R_Elbow_Collision_LCTR",
    "L_Elbow_Collision_LCTR",
    "L_Knee_Collision_LCTR",
    "R_Back_Rock_Collision_LCTR",
    "L_Back_Rock_Collision_LCTR",
    "Head_Collision_LCTR"};
const CThardus::SRockProjectileOffset CThardus::skRockProjectileForwardOffsets[6] = {
    {"", 0.f, 11.f, 0.f}, {"", 0.f, 9.f, 0.f}, {"", 0.f, 7.f, 0.f},
    {"", -3.f, 9.f, 0.f}, {"", 4.f, 7.f, 0.f}, {"", -4.f, 8.f, 0.f}};
const CPatternedCollisionUtils::SSphereJointInfo CThardus::skDamageableSphereJointInfoList[7] = {
    {"R_knee", 1.f},
    {"R_Elbow_Collision_LCTR", 1.5f},
    {"L_Elbow_Collision_LCTR", 1.5f},
    {"L_Knee_Collision_LCTR", 1.f},
    {"R_Back_Rock_Collision_LCTR", 2.5f},
    {"L_Back_Rock_Collision_LCTR", 1.5f},
    {"Head_Collision_LCTR", 1.5f}};
const CPatternedCollisionUtils::SSphereJointInfo CThardus::skNonDamageableSphereJointInfoList[5] = {
    {"R_Shoulder_Collision_LCTR", 0.75f},
    {"L_Shoulder_Collision_LCTR", 0.75f},
    {"Spine_Collision_LCTR", 0.75f},
    {"R_Hand_Collision_LCTR", 2.25f},
    {"L_Hand_Collision_LCTR", 2.f}};
const CPatternedCollisionUtils::SAABoxJointInfo CThardus::skAABoxJointInfoList[2] = {
    {"R_Foot_Collision_LCTR", 3.f, 3.f, 1.f}, {"L_Foot_Collision_LCTR", 3.f, 2.f, 3.f}};
const CVector3f CThardus::skThardusRayCastOffset(0.f, 0.f, 10.f);
const CHealthInfo CThardus::skCollisionActorHealthInfo(1000000.f, 10.f);

CDestroyableRock::CDestroyableRock(
    TUniqueId id, bool active, const rstl::string& name, const CEntityInfo& info,
    const CTransform4f& xf, const CModelData& modelData, float mass, const CHealthInfo& health,
    const CDamageVulnerability& vulnerability, const CMaterialList& matList, CAssetId fsm,
    const CActorParameters& actParams, const CModelData& phazonModel, int w1)
: CAi(id, active, name, info, xf, modelData, modelData.GetBounds(), mass, health, vulnerability,
      matList, fsm, actParams, 0.3f, 0.8f)
, mPhazonModel(phazonModel)
, x324_(0.f)
, x328_(0.f)
, mThermalMag(actParams.GetThermalMag())
, x330_(1.f, 1.f, 1.f)
, mIsCold(false)
, mUsePhazonModel(false)
, mHealthInfo(health)
, x341_(w1 == 0) {
  SetDoTargetDistanceTest(false);
  mPhazonModel.SetSortThermal(true);
  SetDrawShadow(false);
}

void CDestroyableRock::PreThink(float dt, CStateManager& mgr) { CEntity::PreThink(dt, mgr); }

void CDestroyableRock::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }

  float damageMag = mThermalMag;
  if (x324_ > 0.f) {
    x324_ = CMath::Max(0.f, x324_ - dt);
    x330_ = CColor::Lerp(CColor(0.f, 0.f, 0.f), CColor(0.5f, 0.0f, 0.0f), x324_);
    if (mUsePhazonModel) {
      damageMag = x324_ * 0.5f + damageMag;
    } else {
      damageMag = x324_ * 0.25f + damageMag;
    }
  }
  SetDamageMag(damageMag);
  CEntity::Think(dt, mgr);
}

void CDestroyableRock::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  if (GetActive()) {
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal) {
      SetModelFlags(CModelFlags::Normal().DepthCompareUpdate(true, false));
    } else if ((x330_.GetRedu8() & x330_.GetGreenu8() & x330_.GetBlueu8()) < 255) {
      x330_.SetAlpha(static_cast< uchar >(255));
      SetModelFlags(CModelFlags(CModelFlags::kT_Two, x330_));
    } else {
      SetModelFlags(CModelFlags::Normal());
    }
  }
  CActor::PreRender(mgr, frustum);
}

void CDestroyableRock::Render(const CStateManager& mgr) const { CPhysicsActor::Render(mgr); }

ENTITY_ACCEPT_IMPL(CDestroyableRock)

void CDestroyableRock::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                       CStateManager& mgr) {
  CAi::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Registered:
    AddMaterial(kMT_ProjectilePassthrough, mgr);
    AddMaterial(kMT_CameraPassthrough, mgr);
    break;
  }
}

rstl::optional_object< CAABox > CDestroyableRock::GetTouchBounds() const {
  return GetModelData()->GetBounds(GetTransform());
}

void CDestroyableRock::UsePhazonModel() {
  SetModelData(mPhazonModel);
  mUsePhazonModel = true;
}

void CThardus::RenderThermalSpot(float t) const {
  if (mFlareTexture.GetObject() != nullptr) {
    mFlareTexture.GetObject()->Load(GX_TEXMAP0, CTexture::kCM_Repeat);
    const float scale = 30.f * t;
    const CVector3f right = scale * CGraphics::GetViewMatrix().GetRight();
    const CVector3f up = scale * CGraphics::GetViewMatrix().GetUp();
    const CVector3f pos = mCurrentRockPos;
    CGraphics::SetModelMatrix(CTransform4f::Identity());
    CGraphics::SetBlendMode(kBM_Blend, kBF_One, kBF_One, kLO_Clear);
    CGraphics::SetTevOp(kTS_Stage0, CGraphics::kEnvModulate);
    CGraphics::SetTevOp(kTS_Stage1, CGraphics::kEnvPassthru);
    CGraphics::SetDepthWriteMode(false, kE_Always, false);
    const CColor color(t, t, t, t);
    CGraphics::StreamColor(color);
    CGraphics::StreamBegin(kP_TriangleFan);
    CGraphics::StreamTexcoord(0.f, 0.f);
    CGraphics::StreamVertex(pos - right + up);
    CGraphics::StreamTexcoord(1.f, 0.f);
    CGraphics::StreamVertex(pos - right - up);
    CGraphics::StreamTexcoord(1.f, 1.f);
    CGraphics::StreamVertex(pos + right - up);
    CGraphics::StreamTexcoord(0.f, 1.f);
    CGraphics::StreamVertex(pos + right + up);
    CGraphics::StreamEnd();
  }
}

CThardus::CThardus(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                   const CTransform4f& xf, const CModelData& mData,
                   const CActorParameters& actParms, const CPatternedInfo& pInfo,
                   const rstl::vector< CModelData >& models1,
                   const rstl::vector< CModelData >& models2, uint particle1, uint particle2,
                   uint particle3, float f1, float f2, float f3, float f4, float f5, float f6,
                   uint stateMachine, uint particle4, uint particle5, uint particle6,
                   uint particle7, uint particle8, uint particle9, uint texture, uint sfxID1,
                   uint particle10, uint sfxID2, uint sfxID3, uint sfxID4)
: CPatterned(kC_Thardus, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Ground, kCT_One,
             kBT_BiPedal, actParms, kCS_Large)
, x56c_(kInvalidUniqueId)
, x570_(0)
, x574_(0)
, x5c4_(1)
, mHeardPlayer(false)
, x5cc_(models1)
, x5dc_(models2)
, mStateProg(-1)
, mProjectileId(kInvalidUniqueId)
, x600_(particle1)
, x604_(particle2)
, x608_(particle3)
, mProjectileEditorId(kInvalidEditorId)
, x630_(stateMachine)
, x644_(kTS_Invalid)
, mCurrentRock(0)
, mFog(kInvalidUniqueId)
, x650_(0.f, 1.f)
, x658_(-1)
, x65c_(-1)
, x660_(0)
, x688_(false)
, x689_(false)
, x68c_(0)
, x690_(1.f)
, x694_(f1)
, x698_(f2)
, x6a0_(f3)
, x6a4_(f4)
, x6a8_(f5)
, x6ac_(f6)
, x6d0_(particle4)
, x6d4_(particle5)
, x6d8_(particle6)
, x6dc_(particle7)
, x6e0_(particle8)
, x6e4_(particle9)
, x6e8_(texture)
, x6ec_(CSfxManager::TranslateSFXID(sfxID1))
, x6f0_(particle10)
, x6f4_(0)
, x6f8_(0.3f)
, x74c_(CVector3f::Forward())
, x758_(sfxID2)
, x75c_(sfxID3)
, x760_(sfxID4)
, mStartTransform(CTransform4f::Identity())
, x794_(0)
, x7b8_(0.f)
, x7bc_(10.f)
, x7c4_(0)
, x7c8_(false)
, x7cc_(CVector3f::Zero())
, x7d8_(CVector3f::Zero())
, x7e4_(CVector3f::Zero())
, mPathFindSearch(nullptr, 1, pInfo.GetPathfindingIndex(), 1.f, 1.f)
, x8d4_(false)
, x8d8_(CVector3f::Zero())
, x8e4_(CVector3f::Zero())
, x8f0_(false)
, x908_(false)
, x909_(false)
, mFlareTexture(gpSimplePool->GetObj("Thermal_Spot_2"))
, mCurrentRockId(kInvalidUniqueId)
, mCurrentRockPos(CVector3f::Zero())
, x938_(false)
, x939_(false)
, x93a_(false)
, x93b_(false)
, x93c_(false)
, x93d_(true)
, x940_(0)
, x944_(0.3f)
, mInitialized(false)
, x94d_(false)
, x950_(CVector3f::Zero())
, mDoCodeTrigger(false)
, x95d_(0)
, x95e_(false) {
  SetDoTargetDistanceTest(false);
  SetAngularEnabled(true);
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  exclude.Add(CMaterialList(kMT_Player, kMT_Character, kMT_CollisionActor));
  SetMaterialFilter(
      CMaterialFilter::MakeIncludeExclude(GetMaterialFilter().GetIncludeList(), exclude));
  rstl::vector< CAssetId > gens;
  gens.reserve(6);
  gens.push_back(particle4);
  gens.push_back(particle5);
  gens.push_back(particle6);
  gens.push_back(particle7);
  gens.push_back(particle8);
  gens.push_back(particle9);
  AnimationData()->GetParticleDB().CacheParticleDesc(CCharacterInfo::CParticleResData(
      gens, rstl::vector< CAssetId >(), rstl::vector< CAssetId >(), rstl::vector< CAssetId >()));
  x798_.reserve(6);
  mTimers.reserve(16);
  UpdateThermalFrozenState(true);
  SetDamageMag(0.f);
  mBaseDamageMag = 0.f;
  mWaypoints.reserve(16);
  mFlareTexture.Lock();
  mStateControlledMassiveDeath = false;
  mKnockBackController.SetAutoResetImpulse(false);
  SetMass(100000.f);
}

ENTITY_ACCEPT_IMPL(CThardus)

void CThardus::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_Action:
    if (!mHeardPlayer) {
      mHeardPlayer = true;
    }
    break;
  case kSM_Registered: {
    const uint count = x5cc_.size();
    mDestroyableRocks.reserve(count);
    mDestroyedRocks.reserve(count);
    mRockLights.reserve(count);
    mRockHealths.reserve(count);
    for (uint i = 0; i < count; ++i) {
      const float health = i == count - 1 ? 2.f * x6a8_ : x6a8_;
      const rstl::string rockName("", -1);
      const TUniqueId rockId = mgr.AllocateUniqueId();
      CEntity* rock = rs_new CDestroyableRock(
          rockId, true, rockName, CEntityInfo(GetCurrentAreaId(), NullConnectionList),
          CTransform4f::Identity(), x5cc_[i], 0.f, CHealthInfo(health, 0.f),
          CDamageVulnerability(kVN_Normal, kVN_Deflect, kVN_Normal, kVN_Normal, kVN_Normal,
                               kVN_Normal, kVN_Normal, kVN_Deflect, kVN_Deflect, kVN_Deflect,
                               kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Deflect, kVN_Deflect,
                               kDT_Ricochet),
          GetMaterialList(), x630_,
          CActorParameters(
              CLightParameters(false, 0.f, CLightParameters::kST_Invalid, 0.f, 0.f,
                               CColor(1.f, 1.f, 1.f, 1.f), true, CLightParameters::kLO_NoShadowCast,
                               CLightParameters::kLR_Never, CVector3f(0.f, 0.f, 0.f), -1, -1, false,
                               0),
              CScannableParameters(kInvalidAssetId), rstl::pair< CAssetId, CAssetId >(0, 0),
              rstl::pair< CAssetId, CAssetId >(0, 0), CVisorParameters::None(), true, true, false,
              false, 0.f, 0.f, 1.f),
          x5dc_[i], 0);
      mgr.AddObject(rock);
      mDestroyableRocks.push_back(rockId);
      mDestroyedRocks.push_back(false);
      const TUniqueId lightId = mgr.AllocateUniqueId();
      const rstl::string lightName("", -1);
      CGameLight* light = rs_new CGameLight(
          lightId, GetCurrentAreaId(), false, lightName, CTransform4f::Identity(), rockId,
          CLight::BuildPoint(CVector3f::Zero(), CColor::Blue()), 0, 0, 0.f);
      light->SetActive(false);
      mgr.AddObject(*light);
      mRockLights.push_back(lightId);
      mRockHealths.push_back(health);
    }
    AddMaterial(kMT_ScanPassthrough, mgr);
    AddMaterial(kMT_CameraPassthrough, mgr);
    RemoveMaterial(kMT_Orbit, mgr);
    RemoveMaterial(kMT_Target, mgr);
    InitializeCollisionManagers(mgr);
    mBodyController->SetFallState(pas::kFS_Two);
    mBodyController->Activate(mgr);
    mBodyController->BodyStateInfo().SetLocoAnimChangeAtEndOfAnimOnly(true);
    SetThardusState(kTS_Zero, mgr);
    ComputeNumberOfRangedAttacks(mgr);
    AddMaterial(kMT_RadarObject, mgr);
    break;
  }
  case kSM_Deleted: {
    mRockColliders->Destroy(mgr);
    x5f4_->Destroy(mgr);
    x5f8_->Destroy(mgr);
    mgr.DeleteObjectRequest(mFog);
    const uint rockCount = mDestroyableRocks.size();
    for (uint i = 0; i < rockCount; ++i) {
      mgr.DeleteObjectRequest(mDestroyableRocks[i]);
    }
    uint i = 0;
    const uint lightCount = mRockLights.size();
    for (; i < lightCount; ++i) {
      mgr.DeleteObjectRequest(mRockLights[i]);
    }
    break;
  }
  case kSM_InitializedInArea:
    if (!mInitialized) {
      mInitialized = true;
      mStartTransform = GetTransform();
      const uint count = GetConnectionList().size();
      for (uint i = 0; i < count; ++i) {
        const SConnection& connection = GetConnectionList()[i];
        const TUniqueId id = mgr.GetIdForScript(connection.mObjId);
        if (id != kInvalidUniqueId) {
          if (connection.mState == kSS_Patrol) {
            if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(id))) {
              rstl::reserved_vector< TUniqueId, 16 > waypoints;
              GetWaypoints(*waypoint, mgr, waypoints);
              mWaypointGroups.push_back(waypoints);
            } else if (CThardusRockProjectile* projectile = PATTERNED_CAST_TO(CThardusRockProjectile, mgr.ObjectById(id))) {
              mProjectileId = id;
              mProjectileEditorId = connection.mObjId;
              projectile->SetActive(false);
            } else if (TCastToConstPtr< CRepulsor >(mgr.GetObjectById(id))) {
              mRepulsors.push_back(id);
            } else if (TCastToConstPtr< CScriptDistanceFog >(mgr.GetObjectById(id))) {
              mFog = id;
            }
          } else if (connection.mState == kSS_Zero) {
            if (id != kInvalidUniqueId) {
              if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(id))) {
                mWaypoints.push_back(id);
                waypoint->SetActive(false);
              }
            }
          } else if (connection.mState == kSS_Dead) {
            if (TCastToConstPtr< CScriptTimer >(mgr.GetObjectById(id))) {
              mTimers.push_back(id);
            }
          }
        }
      }
      const TAreaId area = GetCurrentAreaId();
      mPathFindSearch.SetArea(
          mgr.GetWorld()->GetAreaAlways(area).GetPostConstructed()->mPathArea);
    }
    break;
  case kSM_Touched: {
    CEntity* entity = mgr.ObjectById(uid);
    if (CCollisionActor* collider = TCastToPtr< CCollisionActor >(entity)) {
      const TUniqueId touchedId = collider->GetLastTouchedObject();
      CEntity* touched = mgr.ObjectById(touchedId);
      if (CPlayer* player = TCastToPtr< CPlayer >(touched)) {
        if (mCurDamageRemTime <= 0.f) {
          float multiplier = 1.f;
          CVector3f direction = CVector3f::Forward();
          if (x644_ == kTS_Retreat) {
            multiplier = 2.f;
            if (mgr.Random()->Next() % 2 != 0) {
              direction = CVector3f::Left();
            } else {
              direction = CVector3f::Right();
            }
          }
          if (mgr.GetPlayer()->GetFrozenState()) {
            mgr.Player()->BreakFrozenState(mgr);
          }
          direction = GetTransform().BuildMatrix3f() * direction;
          const CDamageInfo damage = GetContactDamage();
          mgr.ApplyDamage(
              GetUniqueId(), player->GetUniqueId(), GetUniqueId(),
              CDamageInfo(damage.GetWeaponMode(), multiplier * damage.GetDamage(),
                          damage.GetRadius(), damage.GetKnockBackPower()),
              CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
              x644_ == kTS_Retreat ? direction : CVector3f::Zero());
          mCurDamageRemTime = mDamageWaitTime;
        }
      } else if (TCastToConstPtr< CBomb >(mgr.GetObjectById(touchedId))) {
        if (x644_ == kTS_Retreat && x93c_) {
          ExposeDestroyableRock(mgr, mCurrentRock);
        }
      }
    }
    break;
  }
  case kSM_Damage:
    if (CCollisionActor* collider = TCastToPtr< CCollisionActor >(mgr.ObjectById(uid))) {
      const TUniqueId touchedId = collider->GetLastTouchedObject();
      const uint count = mRockColliders->GetNumCollisionActors();
      TUniqueId rockId = kInvalidUniqueId;
      for (uint i = 0; i < count; ++i) {
        if (uid == mRockColliders->GetCollisionDescFromIndex(i).GetCollisionActorId()) {
          rockId = mDestroyableRocks[i];
          break;
        }
      }
      if (rockId != kInvalidUniqueId) {
        if (CDestroyableRock* rock = static_cast< CDestroyableRock* >(mgr.ObjectById(rockId))) {
          if (const CGameProjectile* const projectile =
                  TCastToConstPtr< CGameProjectile >(mgr.GetObjectById(touchedId))) {
            if (mBodyController->GetBodyStateInfo().GetCurrentAdditiveStateId() !=
                    pas::kAS_AdditiveReaction &&
                rock->GetDamageFlashTimer() <= 0.f) {
              mBodyController->CommandMgr().DeliverCmd(
                  CBCAdditiveReactionCmd(pas::kART_Five, 1.f, false));
            }
            rock->TakeDamage(CVector3f(0.f, 0.f, 0.f), 0.f);
            const CVector3f position = projectile->GetTranslation();
            const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetCurrentVisor();
            if (visor != CPlayerState::kPV_Thermal ||
                visor == CPlayerState::kPV_Thermal && x7c4_ != 3) {
              AddParticleEffect(mgr, position, x6d0_);
            }
            if (rock->IsUsingPhazonModel()) {
              ProcessSoundEvent(0xac0, 1.f, 0, 0.1f, 1000.f, 20, 127, CVector3f::Zero(), position,
                                mgr.GetNextAreaId().Value(), mgr, false);
            } else {
              ProcessSoundEvent(x75c_, 1.f, 0, 0.1f, 1000.f, 20, 127, CVector3f::Zero(), position,
                                mgr.GetNextAreaId().Value(), mgr, true);
            }
          }
        }
      }
    }
    break;
  case kSM_Stop:
    Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    break;
  case kSM_SetToMax: {
    const uint count = mDestroyableRocks.size();
    for (uint i = mCurrentRock; i < count - 1; ++i) {
      const TUniqueId rockId = mDestroyableRocks[i];
      if (CEntity* rock = mgr.ObjectById(rockId)) {
        rock->SetActive(false);
      }
      ++mCurrentRock;
    }
    break;
  }
  case kSM_Reset:
    mDoCodeTrigger = true;
    mBodyController->SetFallState(pas::kFS_Zero);
    mBodyController->BodyStateInfo().SetState(pas::kAS_Locomotion);
    x93d_ = false;
    break;
  default:
    break;
  }
}

void CThardus::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                               float dt) {
  switch (type) {
  case kUE_FadeIn: {
    const int random = mgr.Random()->Next() % 2;
    const uint count = (IsEnraged() ? 6 : 3) - random;
    x940_ = count;
    x798_.clear();
    for (uint i = 0; i < count; ++i) {
      const TUniqueId id = mgr.GenerateObject(mProjectileEditorId).second;
      if (id == kInvalidUniqueId) {
        continue;
      }
      if (CThardusRockProjectile* rock = PATTERNED_CAST_TO(CThardusRockProjectile, mgr.ObjectById(id))) {
        rock->SetActive(true);
        rock->SetChildrenActive(mgr, true);
        const CVector3f scale = GetModelData()->GetScale();
        const CMatrix3f rotation = GetTransform().BuildMatrix3f();
        const int sign = mgr.Random()->Next() % 2;
        const float randomX = mgr.Random()->Float();
        const float xOffset = sign > 0 ? randomX : -randomX;
        const float zOffset = mgr.Random()->Float();
        const CVector3f position =
            GetTranslation() +
            rotation *
                CVector3f((2.5f * xOffset + skRockProjectileForwardOffsets[i].x) * scale.GetX(),
                          skRockProjectileForwardOffsets[i].y * scale.GetY(), 5.f);
        mgr.RayStaticIntersection(position, CVector3f(0.f, 1.f, 0.f), 100.f,
                                  CMaterialFilter::skPassEverything);
        const CVector3f adjustedPosition =
            position + CVector3f(0.f, 0.f, (-7.f + zOffset) * scale.GetZ());
        rock->SetTransform(CTransform4f(rotation, adjustedPosition));
        rock->SetState(CThardusRockProjectile::kPS_One);
        rock->SetGetUpDelayTime(0.5f + (0.5f * i + 0.5f));
        rock->SetAttackDelayTime(0.5f + (0.4f * i + 0.4f));
        rock->SetThardusId(GetUniqueId());
        rock->x5c4_ = x6dc_;
        rock->x5c8_ = x6e0_;
        rock->x5cc_ = x6e4_;
        rock->x5d4_ = x75c_;
        rock->x5d8_ = x758_;
        rock->x5de_ = mCurrentRock >= mDestroyableRocks.size() - 2;
        x798_.push_back(rock->GetUniqueId());
      }
    }
    break;
  }
  case kUE_TakeOff: {
    const uint triggerCount = (mgr.Random()->Next() & 1) + 2;
    rstl::vector< uint > inRange;
    const uint count = mWaypoints.size();
    inRange.reserve(count);
    const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
    for (uint i = 0; i < count; ++i) {
      if (CScriptWaypoint* waypoint =
              TCastToPtr< CScriptWaypoint >(mgr.ObjectById(mWaypoints[i]))) {
        const CVector3f delta = waypoint->GetTranslation() - playerPos;
        if (delta.MagSquared() > 10.f) {
          inRange.push_back(i);
        }
      }
    }
    uint i = 0;
    const uint inRangeCount = inRange.size();
    for (; i < triggerCount; ++i) {
      const TUniqueId id = mWaypoints[mgr.Random()->Next() % inRangeCount];
      if (CScriptWaypoint* waypoint = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(id))) {
        waypoint->SetActive(true);
        SendScriptMsgs(kSS_Zero, mgr, kSM_None);
        waypoint->SetActive(false);
      }
    }
    break;
  }
  case kUE_Projectile: {
    const CTransform4f wrist = GetLctrTransform(rstl::string("L_wrist", -1));
    const CRayCastResult result =
        mgr.RayStaticIntersection(wrist.GetTranslation(), CVector3f::Down(), 100.f,
                                  CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
    CTransform4f transform =
        CTransform4f::LookAt(result.GetPoint() + CVector3f(0.f, 0.f, 2.f),
                             mgr.GetPlayer()->GetTranslation(), CVector3f::Up());
    transform.RotateLocalZ(CRelAngle::FromDegrees(mgr.Random()->Range(-5.f, 5.f)));
    mgr.AddObject(rs_new CIceAttackProjectile(
        gpSimplePool->GetObj(SObjectTag('PART', x600_)),
        gpSimplePool->GetObj(SObjectTag('PART', x604_)),
        gpSimplePool->GetObj(SObjectTag('PART', x608_)), mgr.AllocateUniqueId(), GetCurrentAreaId(),
        mgr.GetPlayer()->GetUniqueId(), true, transform,
        CDamageInfo(CWeaponMode::Ice(), 6.f, 0.f, 0.f),
        CAABox(CVector3f(-1.f, -1.f, -1.f), CVector3f(1.f, 1.f, 1.f)), x6ac_, CMath::Deg2Rad(42.f),
        x6e8_, x6ec_, 0xaad, x6f0_));
    break;
  }
  case kUE_ScreenShake:
    ShakeScreen(mgr, GetTranslation(), 1.25f, 125.f, 1.f);
    break;
  case kUE_DamageOn: {
    x7c8_ = true;
    const CTransform4f transform = GetTransform();
    const CTransform4f locator = GetModelData()->GetAnimationData()->GetLocatorTransform(
        rstl::string("R_ankle", -1), nullptr);
    const CTransform4f worldLocator = transform * (CTransform4f::Scale(GetModelScale()) * locator);
    x7cc_ = worldLocator.GetTranslation();
    break;
  }
  case kUE_DamageOff:
    x7c8_ = false;
    x7cc_ = CVector3f::Zero();
    break;
  case kUE_FadeOut:
    if (x644_ == kTS_Retreat) {
      x93c_ = true;
      x688_ = true;
    }
    break;
  case kUE_Landing:
    x93c_ = false;
    break;
  case kUE_AlignTargetPos:
    SetThardusState(kTS_Patrol, mgr);
    break;
  case kUE_Delete:
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
    break;
  case kUE_LoopedSoundStop:
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
    break;
  case kUE_BeginAction:
  default:
    break;
  }
}

void CThardus::Think(float dt, CStateManager& mgr) {
  if (GetActive() && mBodyController->GetIsActive()) {
    mFlareTexture.TryCache();
    if (x7c8_) {
      const CPlayer& player = *mgr.GetPlayer();
      const CVector3f delta = player.GetTranslation() - x7cc_;
      const float radius = 10.f * GetModelData()->GetScale().GetX();
      if (delta.MagSquared() < radius * radius) {
        const CDamageInfo damage(CWeaponMode(kWT_AI), 0.f, radius, 10.f);
        mgr.ApplyDamage(GetUniqueId(), player.GetUniqueId(), GetUniqueId(), damage,
                        CMaterialFilter::skPassEverything, delta.AsNormalized());
        x688_ = true;
        x7c8_ = false;
        x7cc_ = CVector3f::Zero();
      }
    }
    UpdateThermalFlash(mgr, dt);
    const uint rockCount = mDestroyableRocks.size();
    if (IsEnraged()) {
      x690_ = 1.f;
      SendScriptMsgs(kSS_DeactivateState, mgr, kSM_None);
    } else if (mCurrentRock >= mDestroyableRocks.size() - 2) {
      x690_ = 1.f;
    } else {
      x690_ = 1.f;
    }
    if (!x93c_) {
      mSpeed = x690_;
      mIsMakingBigStrike = false;
      mDamageDur = 0.f;
    } else {
      mSpeed = x690_ * x694_;
      mIsMakingBigStrike = true;
      mDamageDur = 1.f;
    }
    CPatterned::Think(dt, mgr);
    if (mCurrentRock >= 3 && !x689_) {
      MinorKnockBackNow(mgr);
    }
    mRockColliders->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    x5f4_->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    x5f8_->Update(dt, mgr, CCollisionActorManager::kUO_ObjectSpace);
    UpdateDestroyableRockPositions(mgr);
    UpdateDestroyableRockCollisionActors(mgr);
    if (mCurrentRock >= mDestroyableRocks.size()) {
      Death(mgr, CVector3f::Zero(), kSS_DeathRattle);
    }
    if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Thermal) {
      mDrawParticles = false;
      UpdateNonDestroyableCollisionActorMaterials(kUMM_Add, kMT_ProjectilePassthrough, mgr);
      for (uint i = 0; i < rockCount; ++i) {
        if (CActor* rock = static_cast< CActor* >(mgr.ObjectById(mDestroyableRocks[i]))) {
          if (i == mCurrentRock && !x688_ && !x93c_ && !x909_ && !x93d_) {
            rock->AddMaterial(kMT_Orbit, mgr);
            rock->AddMaterial(kMT_Target, mgr);
          } else {
            rock->RemoveMaterial(kMT_Orbit, mgr);
            rock->RemoveMaterial(kMT_Target, mgr);
          }
        }
      }
      if (x688_) {
        x688_ = false;
      }
    } else {
      mDrawParticles = true;
      UpdateNonDestroyableCollisionActorMaterials(kUMM_Remove, kMT_ProjectilePassthrough, mgr);
      const uint count = mDestroyableRocks.size();
      for (uint i = 0; i < count; ++i) {
        if (CActor* rock = static_cast< CActor* >(mgr.ObjectById(mDestroyableRocks[i]))) {
          const rstl::string& name = rock->GetDebugName();
          if (x688_ || x93c_ || x909_ || x93d_) {
            x688_ = false;
            rock->RemoveMaterial(kMT_Orbit, mgr);
            rock->RemoveMaterial(kMT_Target, mgr);
          } else {
            if (name == rstl::string_l(skHeadRockNameStr) &&
                mDestroyedRocks[mCurrentRock] && mCurrentRock != count - 1) {
              rock->RemoveMaterial(kMT_Orbit, mgr);
              rock->RemoveMaterial(kMT_Target, mgr);
            } else if (mDestroyedRocks[i] || name == rstl::string_l(skHeadRockNameStr) &&
                                                     !mDestroyedRocks[mCurrentRock]) {
              rock->AddMaterial(kMT_Orbit, mgr);
              rock->AddMaterial(kMT_Target, mgr);
            } else {
              rock->RemoveMaterial(kMT_Orbit, mgr);
              rock->RemoveMaterial(kMT_Target, mgr);
            }
          }
        }
      }
    }
    if (x644_ == kTS_Retreat) {
      UpdateExcludeList(*mRockColliders, kUMM_Add, kMT_Player, mgr);
      UpdateExcludeList(*x5f4_, kUMM_Add, kMT_Player, mgr);
      UpdateExcludeList(*x5f8_, kUMM_Add, kMT_Player, mgr);
      if (x93c_) {
        if (x6f8_ >= 0.3f) {
          if (mgr.GetCameraManager()->GetCurrentCameraId() ==
              mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId()) {
            const CCameraShakeData& shake =
                CCameraShakeData::HardVertShakeDistance(0.25f, 0.75f, 125.f, GetTranslation());
            mgr.CameraManager()->AddCameraShaker(shake, true);
          }
          x6f8_ = 0.f;
        } else {
          x6f8_ += dt;
        }
        if (mgr.GetCameraManager()->GetCurrentCameraId() !=
                mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId() &&
            x95d_ < 1) {
          const wchar_t* memo = gpStringTable->GetString(104);
          CSamusHud::DisplayHudMemo(rstl::wstring_l(memo), CHUDMemoParms(5.f, true, false, false));
          ++x95d_;
        }
      }
    } else {
      UpdateExcludeList(*mRockColliders, kUMM_Remove, kMT_Player, mgr);
      UpdateExcludeList(*x5f4_, kUMM_Remove, kMT_Player, mgr);
      UpdateExcludeList(*x5f8_, kUMM_Remove, kMT_Player, mgr);
    }
    UpdateTotalHealth(mgr);
  }
}

void CThardus::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x658_ = -1;
    x950_ = mgr.GetPlayer()->GetTranslation();
    break;
  case kStateMsg_Update:
    if (ShouldTurn(mgr, 0.f)) {
      const CVector3f playerPos = mgr.GetPlayer()->GetTranslation();
      const CQuaternion rotation =
          CQuaternion::LookAt(x950_, playerPos.AsNormalized(), CRelAngle(CMath::Deg2Rad(360.f)));
      const CVector3f direction = CVector3f(playerPos - GetTranslation()).AsNormalized();
      rotation.BuildTransform() * direction;
      mBodyController->CommandMgr().DeliverCmd(
          CBCLocomotionCmd(CVector3f::Zero(), direction, 1.f));
      x950_ = playerPos;
    }
    break;
  case kStateMsg_Deactivate:
    ComputeNextRangedAttack(mgr);
    x94d_ = false;
    break;
  }
}

void CThardus::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mStateProg = 0;
    if (!x95e_) {
      mgr.SetBossParams(GetUniqueId(), HealthInfo(mgr)->GetHP(), 88);
      x95e_ = true;
    }
    break;
  }
  case kStateMsg_Update: {
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Taunt) {
        mStateProg = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_One));
      }
      break;
    case 1:
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Taunt) {
        mStateProg = 3;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate: {
    break;
  }
  }
}

void CThardus::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mStateProg = 0;
    break;
  }
  case kStateMsg_Update: {
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Taunt) {
        mStateProg = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_One));
      }
      break;
    case 1:
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Taunt) {
        mStateProg = 3;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate: {
    break;
  }
  }
}

void CThardus::ProjectileAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mStateProg = 0;
    break;
  }
  case kStateMsg_Update: {
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_ProjectileAttack) {
        mStateProg = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_Zero, CVector3f::Zero(), false));
        mStateProg = 0;
      }
      break;
    case 1:
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_ProjectileAttack) {
        mStateProg = 3;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate: {
    break;
  }
  }
}

void CThardus::TelegraphAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mStateProg = 0;
    break;
  }
  case kStateMsg_Update: {
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_ProjectileAttack) {
        mStateProg = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCProjectileAttackCmd(pas::kS_One, CVector3f::Zero(), false));
        mStateProg = 0;
      }
      break;
    case 1:
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_ProjectileAttack) {
        mStateProg = 3;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate: {
    break;
  }
  }
}

void CThardus::LoopedAttack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x658_ = 0;
    x660_ = 0;
    x570_ = 0;
    x574_ = 0;
    ComputeNumberOfRangedAttacks(mgr);
    x93b_ = false;
    x5c4_ = -1;
    break;
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Update: {
    const CVector3f& position = GetTranslation();
    switch (x658_) {
    case 2:
      break;
    case 0: {
      const CVector3f& playerPos = mgr.GetPlayer()->GetTranslation();
      const CVector3f direction = CVector3f(playerPos - position).AsNormalized();
      const CVector2f steering = GetRepulsorSteering(mgr);
      if (!(steering == CVector2f::Zero())) {
        x650_ = steering;
      } else {
        x650_ = direction.ToVec2f().AsNormalized();
      }
      if (CVector3f(playerPos - position).Magnitude() < x698_) {
        x658_ = 1;
      }
      break;
    }
    case 1: {
      const CVector3f origin = position + skThardusRayCastOffset;
      const CVector3f direction =
          CQuaternion::FromMatrix(GetTransform()).BuildTransform() * CVector3f(0.f, 1.f, 0.f);
      const CRayCastResult result = mgr.RayStaticIntersection(
          origin, direction, 100.f,
          CMaterialFilter::MakeInclude(CMaterialList(kMT_Wall, kMT_Floor, kMT_Ceiling)));
      if (result.IsValid()) {
        x8d8_ = result.GetPoint();
        x8e4_ = origin;
        if (CVector3f(result.GetPoint() - origin).Magnitude() < 20.f) {
          x658_ = 2;
          x8d4_ = true;
        }
      } else {
        const CVector2f steering = GetRepulsorSteering(mgr);
        if (!(steering == CVector2f::Zero())) {
          x650_ = steering;
        }
      }
      break;
    }
    }
    mBodyController->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_MaintainVelocity));
    mBodyController->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(CVector3f(x650_, 0.f), CVector3f::Zero(), 1.f));
    break;
  }
  }
}

void CThardus::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mStateProg = 0;
    ++x570_;
    mStateProg = 0;
    break;
  case kStateMsg_Update:
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_MeleeAttack) {
        mStateProg = 2;
      } else {
        if (mgr.Random()->Float() > 0.5f) {
          mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_Zero));
        } else {
          mBodyController->CommandMgr().DeliverCmd(CBCMeleeAttackCmd(pas::kS_One));
        }
        ++x570_;
      }
      break;
    case 1:
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_MeleeAttack) {
        mStateProg = 3;
      }
      break;
    }
    break;
  case kStateMsg_Deactivate:
    break;
  }
}

void CThardus::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    SetTransform(mStartTransform);
    EndBigThermalFlash(mgr);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
  CPatterned::Dead(mgr, msg, arg);
}

bool CThardus::InPosition(CStateManager& mgr, float arg) { return x660_ > 3; }

bool CThardus::ShouldTaunt(CStateManager& mgr, float arg) { return false; }

bool CThardus::InRange(CStateManager& mgr, float arg) {
  const float scale = GetModelData()->GetScale().GetX();
  const CVector2f delta = mgr.GetPlayer()->GetTranslation().ToVec2f() - GetTranslation().ToVec2f();
  return delta.Magnitude() < 10.f * scale;
}

bool CThardus::HasAttackPattern(CStateManager& mgr, float arg) {
  return x5c4_ == 1 && !ShouldMove(mgr, 0.f);
}

bool CThardus::CoveringFire(CStateManager& mgr, float arg) {
  return x5c4_ == 0 && !ShouldMove(mgr, 0.f);
}

bool CThardus::ShouldMove(CStateManager& mgr, float arg) { return x574_ > x68c_ || x93b_; }

void CThardus::ComputeNumberOfRangedAttacks(CStateManager& mgr) { x68c_ = 20000; }

bool CThardus::PatternOver(CStateManager& mgr, float arg) { return x570_ >= 1 || x93b_; }

bool CThardus::HitSomething(CStateManager& mgr, float arg) {
  return mgr.GetPlayer()->GetFrozenState();
}

bool CThardus::ShouldTurn(CStateManager& mgr, float arg) {
  const CVector2f delta = (mgr.GetPlayer()->GetTranslation() - GetTranslation()).ToVec2f();
  const CVector2f forward = GetTransform().GetForward().ToVec2f();
  return CMath::AbsF(CVector2f::GetAngleDiff(forward, delta)) > CMath::Deg2Rad(30.f);
}

void CThardus::ComputeNextRangedAttack(CStateManager& mgr) {
  if (x574_ != 0) {
    if (IsEnraged() && x5c4_ == 0 && x944_ > 0.f) {
      x944_ = 0.f;
    } else {
      ComputeNewRangedAttack(mgr);
    }
  } else {
    ComputeNewRangedAttack(mgr);
  }
}

void CThardus::ComputeNewRangedAttack(CStateManager& mgr) {
  const float random = mgr.Random()->Float();
  if (IsEnraged() && random < 0.3f) {
    x93b_ = true;
  } else {
    const float selection = rstl::max_val(0.f, random - 0.19999999f);
    if (selection < 0.4f) {
      x5c4_ = 0;
    } else if (selection <= 0.8f) {
      x5c4_ = 1;
    } else {
      x5c4_ = 2;
    }
    ++x574_;
    x944_ = 0.3f;
  }
}

CVector3f CThardus::ComputePatrolDest(CStateManager& mgr) {
  char pathIndex = -1;
  char waypointIndex = -1;
  const uint pathCount = mWaypointGroups.size();
  if (pathCount != 0) {
    const CVector3f& position = GetTranslation();
    rstl::vector< uint > nearest;
    nearest.reserve(pathCount);
    GetTransform().GetForward().ToVec2f().AsNormalized();
    uint i;
    for (i = 0; i < pathCount; ++i) {
      const rstl::reserved_vector< TUniqueId, 16 >& path = mWaypointGroups[i];
      uint nearestIndex = 0;
      const uint count = path.size();
      float minimum = 1000000.f;
      for (uint j = 0; j < count; ++j) {
        const CScriptWaypoint* const waypoint =
            TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(path[j]));
        const CVector2f point = waypoint->GetTranslation().ToVec2f();
        const CVector2f delta = point - position.ToVec2f();
        const float distance = delta.Magnitude();
        if (distance < minimum) {
          nearestIndex = j;
          minimum = distance;
        }
      }
      nearest.push_back(nearestIndex);
    }
    const CVector2f playerPos = mgr.GetPlayer()->GetTranslation().ToVec2f();
    float maxForwardDistance = 0.f;
    float maxDistance = 0.f;
    char farthestPath = 0;
    for (i = 0; i < pathCount; ++i) {
      const CScriptWaypoint* const waypoint =
          TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(mWaypointGroups[i][nearest[i]]));
      const CScriptWaypoint* const next =
          TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(waypoint->NextWaypoint(mgr)));
      const CVector2f point = waypoint->GetTranslation().ToVec2f();
      const CVector2f nextPoint = next->GetTranslation().ToVec2f();
      const CVector2f delta = point - playerPos;
      const CVector2f nextDelta = nextPoint - playerPos;
      const float distance = delta.Magnitude();
      const float nextDistance = nextDelta.Magnitude();
      if (distance > maxForwardDistance && nextDistance >= distance) {
        pathIndex = i;
        maxForwardDistance = distance;
      }
      if (distance > maxDistance) {
        farthestPath = i;
        maxDistance = distance;
      }
    }
    if (pathIndex == -1) {
      pathIndex = farthestPath;
    }
    waypointIndex = nearest[pathIndex];
  }
  if (pathIndex != -1 && waypointIndex != -1) {
    mCurPatrolPath = pathIndex;
    mCurPatrolPathWaypoint = waypointIndex;
    return TCastToConstPtr< CScriptWaypoint >(
               mgr.GetObjectById(mWaypointGroups[pathIndex][waypointIndex]))
        ->GetTranslation();
  }
  return CVector3f::Zero();
}

CVector3f CThardus::GetNextPatrolDest(const CStateManager& mgr) {
  if (mCurPatrolPath != -1 && mCurPatrolPathWaypoint != -1) {
    const signed char oldIndex = mCurPatrolPathWaypoint;
    const rstl::reserved_vector< TUniqueId, 16 >& path = mWaypointGroups[mCurPatrolPath];
    mCurPatrolPathWaypoint = (mCurPatrolPathWaypoint + 1) % path.size();
    const CScriptWaypoint* next =
        TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(path[mCurPatrolPathWaypoint]));
    const CScriptWaypoint* previous =
        TCastToConstPtr< CScriptWaypoint >(mgr.GetObjectById(path[oldIndex]));
    return next->GetTranslation();
  }
  return CVector3f::Zero();
}

void CThardus::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  CPatterned::PreRender(mgr, frustum);
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal) {
    SetModelFlags(CModelFlags::Normal().DepthCompareUpdate(true, false));
  } else {
    SetModelFlags(CModelFlags::Normal());
  }
}

void CThardus::Render(const CStateManager& mgr) const {
  CPatterned::Render(mgr);
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal && x7c4_ != 0) {
    RenderThermalSpot(x7c0_);
  }
}

void CThardus::GetWaypoints(CScriptWaypoint& waypoint, CStateManager& mgr,
                            rstl::reserved_vector< TUniqueId, 16 >& ids) {
  if (ids.size() < 16u && waypoint.GetActive()) {
    waypoint.SetActive(false);
    ids.push_back(waypoint.GetUniqueId());
    const uint count = waypoint.GetConnectionList().size();
    for (uint i = 0; i < count; ++i) {
      const TUniqueId id = mgr.GetIdForScript(waypoint.GetConnectionList()[i].mObjId);
      if (id != kInvalidUniqueId) {
        if (CScriptWaypoint* next = TCastToPtr< CScriptWaypoint >(mgr.ObjectById(id))) {
          GetWaypoints(*next, mgr, ids);
        }
      }
    }
    waypoint.SetActive(true);
  }
}

void CThardus::GetUp(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    RemoveMaterial(kMT_RadarObject, mgr);
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
    break;
  }
}

void CThardus::Cover(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    SetThardusState(kTS_Retreat, mgr);
    x93d_ = false;
    x909_ = false;
    if (mCurrentRock >= mDestroyableRocks.size() - 2) {
      x690_ = 1.1f;
    }
    AddMaterial(kMT_RadarObject, mgr);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    x690_ = 1.f;
    break;
  }
}

bool CThardus::AnimOver(CStateManager& mgr, float arg) { return mStateProg == 3; }

bool CThardus::HearPlayer(CStateManager& mgr, float arg) { return mHeardPlayer; }

void CThardus::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mStateProg = 0;
    break;
  }
  case kStateMsg_Update: {
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Getup) {
        mStateProg = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCGetupCmd(pas::kGetup_Zero));
      }
      break;
    case 1:
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Getup) {
        mStateProg = 3;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate: {
    x93d_ = false;
    break;
  }
  }
}

void CThardus::InitializeCollisionManagers(CStateManager& mgr) {
  rstl::vector< CJointCollisionDescription > damageable;
  rstl::vector< CJointCollisionDescription > nonDamageable;
  damageable.reserve(7);
  nonDamageable.reserve(5);
  AddSphereCollisionList(skDamageableSphereJointInfoList, 7, damageable);
  AddSphereCollisionList(skNonDamageableSphereJointInfoList, 5, nonDamageable);
  mRockColliders =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), damageable, true);
  SetMaterialProperties(mRockColliders, mgr);
  x5f4_ =
      rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), nonDamageable, true);
  SetMaterialProperties(x5f4_, mgr);
  rstl::vector< CJointCollisionDescription > boxes;
  boxes.reserve(2);
  AddAABoxCollisionList(skAABoxJointInfoList, 2, boxes);
  x5f8_ = rs_new CCollisionActorManager(mgr, GetUniqueId(), GetCurrentAreaId(), boxes, true);
  SetMaterialProperties(x5f8_, mgr);
  const uint sphereCount =
      mRockColliders->GetNumCollisionActors() + x5f4_->GetNumCollisionActors();
  const uint boxCount = x5f8_->GetNumCollisionActors();
  mNonDestroyableActors.reserve(sphereCount + boxCount);
  CacheNonDestroyableCollisionActorIds(*x5f4_);
  CacheNonDestroyableCollisionActorIds(*x5f8_);
  const uint count = mRockColliders->GetNumCollisionActors();
  for (uint i = 0; i < count; ++i) {
    const CJointCollisionDescription& desc = mRockColliders->GetCollisionDescFromIndex(i);
    const TUniqueId colliderId = desc.GetCollisionActorId();
    const TUniqueId rockId = mDestroyableRocks[i];
    CCollisionActor* collider = TCastToPtr< CCollisionActor >(mgr.ObjectById(colliderId));
    CDestroyableRock* rock = static_cast< CDestroyableRock* >(mgr.ObjectById(rockId));
    if (collider != nullptr && rock != nullptr) {
      if (i != 0) {
        collider->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerability());
        rock->SetThermalMag(0.f);
      } else {
        collider->SetDamageVulnerability(
            *static_cast< const CDestroyableRock* >(rock)->GetDamageVulnerability());
        rock->SetThermalMag(0.8f);
      }
      *collider->HealthInfo(mgr) = *rock->HealthInfo(mgr);
    }
  }
}

void CThardus::SetMaterialProperties(rstl::single_ptr< CCollisionActorManager >& colMgr,
                                     CStateManager& mgr) {
  for (uint i = 0; i < colMgr->GetNumCollisionActors(); ++i) {
    const CJointCollisionDescription& desc = colMgr->GetCollisionDescFromIndex(i);
    const TUniqueId id = desc.GetCollisionActorId();
    CActor* actor = static_cast< CActor* >(mgr.ObjectById(id));
    if (actor != nullptr) {
      actor->AddMaterial(kMT_ScanPassthrough, mgr);
      actor->AddMaterial(kMT_CameraPassthrough, mgr);
      actor->AddMaterial(kMT_Immovable, mgr);
      actor->AddMaterial(kMT_NoPlayerCollision, mgr);
      CMaterialList include = GetMaterialFilter().GetIncludeList();
      CMaterialList exclude = GetMaterialFilter().GetExcludeList();
      const CMaterialList actorInclude = actor->GetMaterialFilter().GetIncludeList();
      const CMaterialList actorExclude = actor->GetMaterialFilter().GetExcludeList();
      actor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include.Union(actorInclude),
                                                                   exclude.Union(actorExclude)));
    }
  }
}

void CThardus::AddSphereCollisionList(const CPatternedCollisionUtils::SSphereJointInfo* joints,
                                      int count, rstl::vector< CJointCollisionDescription >& list) {
  const CAnimData* animData = GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId seg = animData->GetLocatorSegId(rstl::string_l(joints[i].name));
    if (seg != CSegId(0xff)) {
      const CJointCollisionDescription desc = CJointCollisionDescription::SphereCollision(
          seg, joints[i].radius, rstl::string_l(joints[i].name), 0.001f);
      list.push_back(desc);
    }
  }
}

void CThardus::AddAABoxCollisionList(const CPatternedCollisionUtils::SAABoxJointInfo* joints,
                                     int count, rstl::vector< CJointCollisionDescription >& list) {
  const CAnimData* animData = GetAnimationData();
  for (int i = 0; i < count; ++i) {
    const CSegId seg = animData->GetLocatorSegId(rstl::string_l(joints[i].name));
    if (seg != CSegId(0xff)) {
      const CJointCollisionDescription desc = CJointCollisionDescription::AABoxCollision(
          seg, CVector3f(joints[i].xExtent, joints[i].yExtent, joints[i].zExtent),
          rstl::string_l(joints[i].name), 0.001f);
      list.push_back(desc);
    }
  }
}

void CThardus::UpdateDestroyableRockPositions(CStateManager& mgr) {
  const uint count = mDestroyableRocks.size();
  const CTransform4f& transform = GetTransform();
  const CVector3f& scale = GetModelScale();
  for (uint i = 0; i < count; ++i) {
    const CTransform4f localXf = GetAnimationData()->GetLocatorTransform(
        rstl::string(skDamageableRockJointNameList[i]), nullptr);
    const CTransform4f xf = transform * (CTransform4f::Scale(scale) * localXf);
    const TUniqueId rockId = mDestroyableRocks[i];
    if (CActor* rock = TCastToPtr< CActor >(mgr.ObjectById(rockId))) {
      rock->SetTransform(xf);
    }
    const TUniqueId lightId = mRockLights[i];
    if (CGameLight* light = TCastToPtr< CGameLight >(mgr.ObjectById(lightId))) {
      light->SetTransform(xf);
    }
  }
}

void CThardus::CacheNonDestroyableCollisionActorIds(const CCollisionActorManager& colMgr) {
  const uint count = colMgr.GetNumCollisionActors();
  for (uint i = 0; i < count; ++i) {
    const CJointCollisionDescription& desc = colMgr.GetCollisionDescFromIndex(i);
    const TUniqueId id = desc.GetCollisionActorId();
    bool found = false;
    const rstl::string& name = desc.GetName();
    for (uint j = 0; j < 7; ++j) {
      if (name == rstl::string_l(skDamageableRockCollisionJointNameList[j])) {
        found = true;
        break;
      }
    }
    if (!found) {
      mNonDestroyableActors.push_back(id);
    }
  }
}

void CThardus::UpdateExcludeList(const CCollisionActorManager& colMgr, EUpdateMaterialMode mode,
                                 EMaterialTypes material, CStateManager& mgr) {
  const uint count = colMgr.GetNumCollisionActors();
  uint i = 0;
  const CMaterialList materials(1ull << material);
  for (; i < count; ++i) {
    const CJointCollisionDescription& desc = colMgr.GetCollisionDescFromIndex(i);
    const TUniqueId id = desc.GetCollisionActorId();
    if (CActor* actor = TCastToPtr< CActor >(mgr.ObjectById(id))) {
      CMaterialList exclude = actor->GetMaterialFilter().GetExcludeList();
      switch (mode) {
      case kUMM_Add:
        exclude.Add(materials);
        break;
      case kUMM_Remove:
        exclude.Remove(materials);
        break;
      }
      actor->SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(
          actor->GetMaterialFilter().GetIncludeList(), exclude));
    }
  }
}

void CThardus::UpdateNonDestroyableCollisionActorMaterials(EUpdateMaterialMode mode,
                                                           EMaterialTypes material,
                                                           CStateManager& mgr) {
  const uint count = mNonDestroyableActors.size();
  for (uint i = 0; i < count; ++i) {
    const TUniqueId id = mNonDestroyableActors[i];
    if (CCollisionActor* actor = TCastToPtr< CCollisionActor >(mgr.ObjectById(id))) {
      switch (mode) {
      case kUMM_Add:
        actor->AddMaterial(material, mgr);
        break;
      case kUMM_Remove:
        actor->RemoveMaterial(material, mgr);
        break;
      }
      *actor->HealthInfo(mgr) = skCollisionActorHealthInfo;
    }
  }
}

void CThardus::UpdateDestroyableRockCollisionActors(CStateManager& mgr) {
  const uint count = mRockColliders->GetNumCollisionActors();
  for (uint i = 0; i < count; ++i) {
    const TUniqueId colliderId =
        mRockColliders->GetCollisionDescFromIndex(i).GetCollisionActorId();
    if (CCollisionActor* collider = TCastToPtr< CCollisionActor >(mgr.ObjectById(colliderId))) {
      if (!collider->GetActive()) {
        continue;
      }
      const TUniqueId rockId = mDestroyableRocks[i];
      if (CDestroyableRock* rock = static_cast< CDestroyableRock* >(mgr.ObjectById(rockId))) {
        if (x909_) {
          *collider->HealthInfo(mgr) = CHealthInfo(mRockHealths[i], 0.f);
        }
        if (i != mCurrentRock || x93d_) {
          collider->SetDamageVulnerability(CDamageVulnerability::ImmuneVulnerability());
          rock->SetThermalMag(0.f);
        } else {
          collider->SetDamageVulnerability(
              *static_cast< const CDestroyableRock* >(rock)->GetDamageVulnerability());
          rock->SetThermalMag(0.8f);
        }
        if (CHealthInfo* health = collider->HealthInfo(mgr)) {
          if (health->GetHP() <= 0.f) {
            if (rock->IsUsingPhazonModel()) {
              rock->SetActive(false);
              collider->SetActive(false);
              mgr.ObjectById(mRockLights[i])->SetActive(false);
              ++mCurrentRock;
              KnockBackNow(mgr);
              const CVector3f position = rock->GetTranslation();
              const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetCurrentVisor();
              if (visor != CPlayerState::kPV_Thermal ||
                  visor == CPlayerState::kPV_Thermal && x7c4_ != 3) {
                AddParticleEffect(mgr, position, x6d8_);
              }
              ProcessSoundEvent(x758_, 1.f, 0, 0.1f, 1000.f, 20, 127, CVector3f::Zero(), position,
                                mgr.GetNextAreaId().Value(), mgr, true);
              if (IsEnraged() && !x8f0_) {
                BecomeEnraged(mgr);
              }
              EndBigThermalFlash(mgr);
            } else {
              ExposeDestroyableRock(mgr, i);
              SummonIceStorm(mgr);
            }
          } else {
            *rock->HealthInfo(mgr) = *health;
            if (!x909_) {
              mRockHealths[i] = health->GetHP();
            }
          }
        }
      }
    }
  }
}

void CThardus::Flinch(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mStateProg = 0;
    const uint count = x798_.size();
    for (uint i = 0; i < count; ++i) {
      if (CThardusRockProjectile* rock = PATTERNED_CAST_TO(CThardusRockProjectile, mgr.ObjectById(x798_[i]))) {
        rock->AddGravity(mgr);
      }
    }
    x93b_ = true;
    x93d_ = true;
    x909_ = true;
    x93c_ = false;
    SetThardusState(kTS_Invalid, mgr);
    x94d_ = true;
    break;
  }
  case kStateMsg_Update: {
    pas::ESeverity severity = pas::kS_Invalid;
    switch (mCurrentRock) {
    case 1:
      severity = pas::kS_Zero;
      break;
    case 2:
      severity = pas::kS_One;
      break;
    case 3:
      severity = pas::kS_Two;
      break;
    case 4:
      severity = pas::kS_Three;
      break;
    case 5:
      severity = pas::kS_Four;
      break;
    case 6:
      severity = pas::kS_Five;
      break;
    }
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_KnockBack) {
        mStateProg = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCKnockBackCmd(CVector3f::Zero(), severity));
      }
      break;
    case 1:
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_KnockBack) {
        mStateProg = 3;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate: {
    break;
  }
  }
}

void CThardus::Growth(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mStateProg = 0;
    x904_ = CSfxManager::SfxStart(0x7ad, 127, 64, false, CSfxManager::kMedPriority, true);
    break;
  }
  case kStateMsg_Update: {
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
        mStateProg = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Forward, pas::kStep_BreakDodge));
      }
      break;
    case 1:
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Step) {
        mStateProg = 3;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate: {
    CScriptDistanceFog* fog = TCastToPtr< CScriptDistanceFog >(mgr.ObjectById(mFog));
    const TEditorId editorId = mgr.GetEditorIdForUniqueId(fog->GetUniqueId());
    mgr.SendScriptMsg(GetUniqueId(), editorId, kSM_Activate, kSS_Any);
    SendScriptMsgs(kSS_Play, mgr, kSM_None);
    break;
  }
  }
}

void CThardus::Enraged(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mStateProg = 0;
    x688_ = true;
    x908_ = true;
    break;
  }
  case kStateMsg_Update: {
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Taunt) {
        mStateProg = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(CBCTauntCmd(pas::kTT_Zero));
      }
      break;
    case 1:
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Taunt) {
        mStateProg = 3;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate: {
    x908_ = false;
    break;
  }
  }
}

void CThardus::Shuffle(CStateManager& mgr, EStateMsg msg, float arg) {}

bool CThardus::AggressionCheck(CStateManager& mgr, float arg) {
  return mStateMachineState.GetTime() > 0.1f;
}

void CThardus::SummonIceStorm(CStateManager& mgr) {
  if (x644_ != kTS_Retreat) {
    mStateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("Faint"));
  }
}

void CThardus::KnockBackNow(CStateManager& mgr) {
  mStateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("Flinch"));
}

void CThardus::BecomeEnraged(CStateManager& mgr) {
  mStateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("DoubleSnap"));
}

void CThardus::MinorKnockBackNow(CStateManager& mgr) {
  mStateMachineState.SetState(mgr, *this, GetStateMachine(), rstl::string_l("Suck"));
}

CVector2f CThardus::GetSteeringVector(CStateManager& mgr, float dt) {
  CVector3f move = CVector3f::Zero();
  if (GetSearchPath() != nullptr) {
    if (GetSearchPath()->GetResult() != CPathFindSearch::kR_Success) {
      CVector3f point = CVector3f::Zero();
      if (GetSearchPath()->FindClosestReachablePoint(GetTranslation(), point) ==
          CPathFindSearch::kR_Success) {
        move = mSteeringBehaviors.Arrival(*this, x7d8_, 0.f);
      }
    } else {
      CPatterned::PathFind(mgr, kStateMsg_Update, dt);
      move = mBodyController->GetCommandMgr().GetMoveVector();
    }
  }
  CVector2f ret = move.ToVec2f();
  const CVector3f pos = GetTranslation();
  if (!x8d4_ || (x8d4_ && mPathFindSearch.OnPath(pos) != CPathFindSearch::kR_Success)) {
    const CVector2f repulsor = GetRepulsorSteering(mgr);
    if (!(repulsor == CVector2f::Zero())) {
      return repulsor;
    }
  }
  if (ret == CVector2f::Zero()) {
    ret = mSteeringBehaviors.Arrival(*this, x7d8_, 0.f).ToVec2f();
  }
  return ret;
}

void CThardus::AddParticleEffect(CStateManager& mgr, const CVector3f& pos, CAssetId particle) {
  const rstl::string name = rstl::string("ROCK_EFFECT", -1) +
                            CStringExtras::CreateFromInteger(particle) + rstl::string("-", -1) +
                            CStringExtras::CreateFromInteger(x6f4_++);
  AnimationData()->GetParticleDB().AddAuxiliaryParticleEffect(
      name, 0x40, CAuxiliaryParticleData(0, SObjectTag('PART', particle), pos, 1.f),
      GetModelScale() * 2.f, mgr, GetCurrentAreaId(), 0);
}

bool CThardus::AttackOver(CStateManager& mgr, float arg) {
  if (x5c4_ == 1 && !x94d_) {
    const float attackTime = 0.9f * x940_;
    const float duration = IsEnraged() ? attackTime : rstl::max_val(attackTime, skMinAttackTime);
    if (GetStateMachineTime() > duration) {
      return true;
    }
  } else {
    if (IsEnraged() || x93b_ || x94d_ || x5c4_ == -1) {
      return true;
    }
    if (GetStateMachineTime() > 0.75f) {
      return true;
    }
  }
  return false;
}

bool CThardus::CoverBlown(CStateManager& mgr, float arg) {
  return x5c4_ == 2 && !ShouldMove(mgr, 0.f);
}

bool CThardus::IsEnraged() const { return mDestroyableRocks.size() - 1 == mCurrentRock; }

void CThardus::UpdateThermalFlash(CStateManager& mgr, float dt) {
  if (x7c4_ == 0) {
    x93a_ = false;
    return;
  }
  bool displayMemo = false;
  float intensity = 0.f;
  float viewFactor = 1.f;
  if (mCurrentRockId != kInvalidUniqueId) {
    if (const CActor* rock = TCastToConstPtr< CActor >(mgr.GetObjectById(mCurrentRockId))) {
      mCurrentRockPos = rock->GetTranslation();
    } else {
      mCurrentRockId = kInvalidUniqueId;
    }
  }
  if (!x939_) {
    const CGameCamera& camera = mgr.GetCameraManager()->GetCurrentCamera(mgr);
    const CVector3f delta = mCurrentRockPos - camera.GetTranslation();
    const CVector3f forward = camera.GetTransform().GetForward();
    const CVector3f& direction = delta.AsNormalized();
    const float dot = CVector3f::Dot(direction, forward);
    viewFactor = dot < 0.f ? 0.f : dot * dot;
  }
  switch (x7c4_) {
  case 1:
    intensity = rstl::min_val(x7b8_, skThermalFlashFadeInTime) / 0.25f;
    if (x7b8_ > 0.25f) {
      x7c4_ = 3;
      x7b8_ = 0.f;
    }
    x7b8_ += dt;
    if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Thermal) {
      x688_ = true;
    }
    break;
  case 2:
    intensity = -(rstl::min_val(x7b8_, skThermalFlashFadeOutTime) / 2.f - 1.f);
    if (x7b8_ > 2.f) {
      x7c4_ = 0;
      x7b8_ = 0.f;
      x7c0_ = 0.f;
    }
    x7b8_ += dt;
    break;
  case 3:
    intensity = 1.f;
    if (x7b8_ > x7bc_ && !x938_) {
      x7c4_ = 2;
      x7b8_ = 0.f;
    }
    x7b8_ += dt;
    if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Thermal) {
      x688_ = true;
      displayMemo = x938_ && viewFactor > 0.75f;
    }
    break;
  }
  intensity *= viewFactor;
  x7c0_ = intensity;
  const float magnitude = 0.8f * intensity;
  mgr.AddThermalColdScale2(magnitude);
  mBaseDamageMag = magnitude;
  if (displayMemo != x93a_) {
    if (displayMemo) {
      CSamusHud::DisplayHudMemo(rstl::wstring_l(gpStringTable->GetString(18)),
                                CHUDMemoParms(5.f, true, false, false));
    }
    x93a_ = displayMemo;
  }
}

bool CThardus::PathFound(CStateManager& mgr, float arg) { return x8d4_; }

void CThardus::PathFind(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mDestPos = ComputePatrolDest(mgr);
    x7d8_ = mDestPos;
    x7e4_ = x7d8_;
    CPatterned::PathFind(mgr, kStateMsg_Activate, arg);
    break;
  case kStateMsg_Update: {
    const CVector2f pos = GetTranslation().ToVec2f();
    const CVector2f target = x7e4_.ToVec2f();
    const CVector3f& delta = CVector3f(pos - target, 0.f);
    if (delta.Magnitude() < 10.f) {
      mDestPos = GetNextPatrolDest(mgr);
      x7d8_ = mDestPos;
      x7e4_ = x7d8_;
      CPatterned::PathFind(mgr, kStateMsg_Activate, arg);
      ++x660_;
    }
    x650_ = GetSteeringVector(mgr, arg);
    break;
  }
  case kStateMsg_Deactivate:
    x8d4_ = false;
    break;
  }
  mBodyController->CommandMgr().DeliverCmd(
      CBCLocomotionCmd(CVector3f(x650_, 0.f), CVector3f::Zero(), 1.f));
}

void CThardus::StartTimedThermalFlash(CStateManager& mgr, float duration, const CActor& actor) {
  if (!x938_) {
    x7bc_ = duration;
    x939_ = true;
    BeginFlash(mgr, actor);
  }
}

void CThardus::StartBigThermalFlash(CStateManager& mgr, const CActor& actor) {
  if (!x938_) {
    x938_ = true;
    x939_ = false;
    BeginFlash(mgr, actor);
  }
}

void CThardus::EndBigThermalFlash(CStateManager& mgr) {
  x7b8_ = FLT_EPSILON + x7bc_;
  x938_ = false;
}

void CThardus::BeginFlash(CStateManager& mgr, const CActor& actor) {
  if (mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::kPV_Thermal) {
    x688_ = true;
  }
  if (x7c4_ == 0 || x7c4_ == 2) {
    x7c4_ = 1;
    x7b8_ = 0.f;
  }
  mCurrentRockId = actor.GetUniqueId();
  mCurrentRockPos = actor.GetTranslation();
}

void CThardus::ShakeScreen(CStateManager& mgr, const CVector3f& pos, float magnitude,
                           float distance, float duration) {
  const CVector3f delta = pos - mgr.GetPlayer()->GetTranslation();
  const float distanceToPlayer = delta.Magnitude();
  const float shake = rstl::max_val(0.f, -(distanceToPlayer * (magnitude / distance) - magnitude));
  if (mgr.GetCameraManager()->GetCurrentCameraId() ==
      mgr.GetCameraManager()->GetFirstPersonCamera()->GetUniqueId()) {
    const CCameraShakeData& data =
        CCameraShakeData::HardVertShakeDistance(duration, magnitude, distance, GetTranslation());
    mgr.CameraManager()->AddCameraShaker(data, true);
  }
  if (x908_) {
    ShakePlayer(mgr, shake);
  }
}

void CThardus::Explode(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mStateProg = 0;
    CSfxManager::SfxStop(x904_);
    x909_ = true;
    x93d_ = true;
    x909_ = true;
    SendScriptMsgs(kSS_Arrived, mgr, kSM_None);
    break;
  }
  case kStateMsg_Update: {
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_Step) {
        mStateProg = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCStepCmd(pas::kSD_Forward, pas::kStep_Dodge));
      }
      break;
    case 1:
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_Step) {
        mStateProg = 3;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate: {
    x8f0_ = true;
    x909_ = false;
    x93d_ = false;
    break;
  }
  }
}

void CThardus::Suck(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    x909_ = true;
    x93d_ = true;
    SendScriptMsgs(kSS_MaxReached, mgr, kSM_None);
    break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    x689_ = true;
    break;
  }
}

bool CThardus::StartAttack(CStateManager& mgr, float arg) { return true; }

void CThardus::ShakePlayer(CStateManager& mgr, float intensity) {
  if (intensity > 0.f) {
    CPlayer& player = *mgr.Player();
    const CVector3f delta = GetTranslation() - player.GetTranslation();
    const float distance = delta.Magnitude();
    if (player.GetSurfaceRestraint() != CPlayer::kSR_Air && !player.IsInsideFluid()) {
      const CVector3f baseImpulse = intensity * (40.f * CVector3f::Up());
      CVector3f additionalImpulse = CVector3f::Zero();
      if (distance > 10.f) {
        const CVector3f flatDelta(delta.ToVec2f(), 0.f);
        if (flatDelta.CanBeNormalized()) {
          additionalImpulse = intensity * (12.5f * flatDelta.AsNormalized());
        }
      }
      const CVector3f impulse = baseImpulse + additionalImpulse;
      player.ApplyImpulseWR(player.GetMass() * impulse, CAxisAngle::Identity());
      player.SetMoveState(NPlayer::kMS_ApplyJump, mgr);
    }
  }
}

bool CThardus::CanLockOnToRockProjectiles(const CStateManager& mgr) const {
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal) {
    return !x93a_ || x7c4_ == 0;
  }
  return true;
}

void CThardus::SetThardusState(EThardusState state, CStateManager& mgr) {
  x644_ = state;
  switch (state) {
  case kTS_Patrol:
    SendScriptMsgs(kSS_Patrol, mgr, kSM_None);
    break;
  case kTS_Retreat:
    SendScriptMsgs(kSS_Retreat, mgr, kSM_None);
    break;
  }
}

bool CThardus::ShouldCallForBackup(CStateManager& mgr, float arg) {
  return mStateMachineState.GetTime() > 0.5f;
}

bool CThardus::IsDizzy(CStateManager& mgr, float arg) {
  return mStateMachineState.GetTime() > 4.f;
}

CVector3f CThardus::GetOrbitPosition(const CStateManager& mgr) const {
  return GetAimPosition(mgr, 0.f);
}

CVector3f CThardus::GetAimPosition(const CStateManager& mgr, float dt) const {
  const CTransform4f xf =
      GetLctrTransform(x93c_ ? rstl::string("center_LCTR", -1) : rstl::string("Neck_1", -1));
  return xf.GetTranslation();
}

void CThardus::DoubleSnap(CStateManager& mgr, EStateMsg msg, float arg) {}

void CThardus::Faint(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    mStateProg = 0;
    x93c_ = false;
    SetThardusState(kTS_Invalid, mgr);
    const uint count = x798_.size();
    for (uint i = 0; i < count; ++i) {
      if (CThardusRockProjectile* rock = PATTERNED_CAST_TO(CThardusRockProjectile, mgr.ObjectById(x798_[i]))) {
        rock->AddGravity(mgr);
      }
    }
    x94d_ = true;
    break;
  }
  case kStateMsg_Update: {
    switch (mStateProg) {
    case 0:
      if (mBodyController->GetCurrentStateId() == pas::kAS_KnockBack) {
        mStateProg = 2;
      } else {
        mBodyController->CommandMgr().DeliverCmd(
            CBCKnockBackCmd(CVector3f::Zero(), pas::kS_Six));
      }
      break;
    case 1:
      break;
    case 2:
      if (mBodyController->GetCurrentStateId() != pas::kAS_KnockBack) {
        mStateProg = 3;
      }
      break;
    }
    break;
  }
  case kStateMsg_Deactivate: {
    break;
  }
  }
}

bool CThardus::CodeTrigger(CStateManager& mgr, float arg) { return mDoCodeTrigger; }

void CThardus::UpdateTotalHealth(CStateManager& mgr) {
  float totalHealth = 0.f;
  const uint count = mDestroyableRocks.size();
  for (uint i = mCurrentRock; i < count; ++i) {
    const float health = i == count - 1 ? 2.f * x6a4_ : x6a4_;
    CDestroyableRock* rock =
        static_cast< CDestroyableRock* >(mgr.ObjectById(mDestroyableRocks[i]));
    if (rock != nullptr) {
      if (!rock->IsUsingPhazonModel()) {
        totalHealth += health;
      }
      totalHealth += rock->HealthInfo(mgr)->GetHP();
    }
  }
  HealthInfo(mgr)->SetHP(totalHealth);
}

CAABox CThardus::GetSortingBounds(const CStateManager& mgr) const {
  const CAABox& bounds = GetRenderBoundsCached();
  const CVector3f extent = 0.15f * (bounds.GetMaxPoint() - bounds.GetMinPoint());
  const CVector3f center = bounds.GetCenterPoint();
  return CAABox(center - extent, center + extent);
}

void CThardus::ExposeDestroyableRock(CStateManager& mgr, uint index) {
  const uint count = mRockColliders->GetNumCollisionActors();
  const TUniqueId rockId = mDestroyableRocks[index];
  CDestroyableRock* rock = static_cast< CDestroyableRock* >(mgr.ObjectById(rockId));
  const TUniqueId colliderId =
      mRockColliders->GetCollisionDescFromIndex(index).GetCollisionActorId();
  CCollisionActor* collider = TCastToPtr< CCollisionActor >(mgr.ObjectById(colliderId));
  if (rock && collider && !rock->IsUsingPhazonModel()) {
    rock->UsePhazonModel();
    const float hp = index == count - 1 ? 2.f * x6a4_ : x6a4_;
    *rock->HealthInfo(mgr) = CHealthInfo(hp, 0.f);
    *collider->HealthInfo(mgr) = CHealthInfo(hp, 0.f);
    mDestroyedRocks[index] = true;
    mgr.ObjectById(mRockLights[index])->SetActive(true);
    rock->SetThermalMag(1.5f);
    const CPlayerState::EPlayerVisor visor = mgr.GetPlayerState()->GetCurrentVisor();
    if (visor != CPlayerState::kPV_Thermal || visor == CPlayerState::kPV_Thermal && x7c4_ != 3) {
      AddParticleEffect(mgr, rock->GetTranslation(), x6d4_);
    }
    mRockHealths[index] = hp;
    StartBigThermalFlash(mgr, *rock);
    const CVector3f position = rock->GetTranslation();
    ProcessSoundEvent(x760_, 1.f, 0, 0.1f, 1000.f, 20, 127, CVector3f::Zero(), position,
                      mgr.GetNextAreaId().Value(), mgr, true);
  }
}

CVector2f CThardus::GetRepulsorSteering(CStateManager& mgr) {
  const uint count = mRepulsors.size();
  CVector2f ret = CVector2f::Zero();
  const CVector2f pos = GetTranslation().ToVec2f();
  for (uint i = 0; i < count; ++i) {
    const TUniqueId id = mRepulsors[i];
    const CRepulsor* const repulsor = TCastToConstPtr< CRepulsor >(mgr.GetObjectById(id));
    const CVector2f repPos = repulsor->GetTranslation().ToVec2f();
    const float radius = repulsor->GetAffectRadius();
    const CVector2f delta = pos - repPos;
    const float distSq = delta.MagSquared();
    if (distSq < radius * radius) {
      CMath::SqrtF(distSq);
      ret = mSteeringBehaviors.Flee2D(*this, repPos);
      break;
    }
  }
  if (!(ret == CVector2f::Zero())) {
    const CVector2f arrival =
        mSteeringBehaviors.Arrival2D(*this, mStartTransform.GetTranslation().ToVec2f());
    ret = CVector2f::Lerp(arrival, ret, 0.f);
  }
  return ret;
}
