#include "MetroidPrime/Enemies/CChozoGhost.hpp"

#include "Kyoto/Animation/CInt32POINode.hpp"
#include "Kyoto/Animation/CPASAnimParmData.hpp"
#include "Kyoto/Audio/CSfxManager.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Particles/CElementGen.hpp"
#include "MetroidPrime/CActorLights.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCoverPoint.hpp"
#include "MetroidPrime/Weapons/CEnergyProjectile.hpp"

#include "Kyoto/Math/CRelAngle.hpp"
#include "MetroidPrime/Enemies/CTeamAiMgr.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include <float.h>

const rstl::string CChozoGhost::skSpeedSwooshName = rstl::string_l("SpeedSwoosh");

CChozoGhost::CBehaveChance::CBehaveChance(CInputStream& in)
: mPropertyCount(in.Get< uint >())
, mLurk(in.Get< float >())
, mTaunt(in.Get< float >())
, mAttack(in.Get< float >())
, mMove(in.Get< float >())
, mLurkTime(in.Get< float >())
, mChargeAttack(mPropertyCount > 5 ? in.Get< float >() * 0.01f : 0.5f)
, mNumBolts(mPropertyCount > 6 ? in.Get< uint >() : 2) {
  const float average = 1.f / (mLurk + mTaunt + mAttack + mMove);
  mLurk *= average;
  mTaunt *= average;
  mAttack *= average;
  mMove *= average;
}

CChozoGhost::EBehaveType CChozoGhost::CBehaveChance::GetBehave(const EBehaveType type,
                                                               CStateManager& mgr) const {
  float lurkChance = mLurk;
  float tauntChance = mTaunt;
  float attackChance = mAttack;
  switch (type) {
  case kBT_Lurk: {
    const float delta = lurkChance / 3.f;
    lurkChance = 0.f;
    tauntChance += delta;
    attackChance += delta;
  } break;
  case kBT_Taunt: {
    const float delta = tauntChance / 3.f;
    tauntChance = 0.f;
    lurkChance += delta;
    attackChance += delta;
  } break;
  case kBT_Attack: {
    const float delta = attackChance / 3.f;
    attackChance = 0.f;
    lurkChance += delta;
    tauntChance += delta;
  } break;
  case kBT_Move: {
    const float delta = mMove / 3.f;
    lurkChance += delta;
    tauntChance += delta;
    attackChance += delta;
  } break;
  default:
    break;
  }

  const float rnd = mgr.Random()->Float();
  EBehaveType ret = kBT_Move;
  if (rnd < lurkChance) {
    ret = kBT_Lurk;
  } else if (rnd - lurkChance < tauntChance) {
    ret = kBT_Taunt;
  } else if (rnd - lurkChance - tauntChance < attackChance) {
    ret = kBT_Attack;
  }
  return ret;
}

CChozoGhost::CChozoGhost(
    const TUniqueId uid, const rstl::string& name, const CEntityInfo& info, const CTransform4f& xf,
    const CModelData& mData, const CActorParameters& actParms, const CPatternedInfo& pInfo,
    const float hearingRadius, const float fadeOutDelay, const float attackDelay,
    const float freezeTime, const CAssetId wpsc1, const CDamageInfo& dInfo1, const CAssetId wpsc2,
    const CDamageInfo& dInfo2, const CBehaveChance& chance1, const CBehaveChance& chance2,
    const CBehaveChance& chance3, const ushort soundImpact, const float f1, const ushort sfxFadeIn,
    const ushort sfxFadeOut, const uint w1, const float f2, const uint w2,
    const float hurlRecoverTime, const CAssetId projectileVisor, const ushort soundProjectileVisor,
    const float f3, const float f4, const uint nearChance, const uint midChance)
: CPatterned(kC_ChozoGhost, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_Zero,
             kBT_BiPedal, actParms, kCS_Medium)
, mHearingRadius(hearingRadius)
, mFadeOutDelay(fadeOutDelay)
, mAttackDelay(attackDelay)
, mFreezeTime(freezeTime)
, mProjectileInfo1(wpsc1, dInfo1)
, mProjectileInfo2(wpsc2, dInfo2)
, mBehaveChance1(chance1)
, mBehaveChance2(chance2)
, mBehaveChance3(chance3)
, mSoundImpact(soundImpact)
, x62c_(f1)
, mSfxFadeIn(sfxFadeIn)
, mSfxFadeOut(sfxFadeOut)
, x634_(f2)
, mHurlRecoverTime(hurlRecoverTime)
, x63c_(w2)
, mSoundProjectileVisor(soundProjectileVisor)
, x654_(f3)
, x658_(f4)
, mNearChance(nearChance)
, mMidChance(midChance)
, mBehaviorEnabled((w1 << 6) & 0x40)
, mFlinch((w1 << 5) & 0x40)
, mAlert(false)
, mOnGround(false)
, x664_28_(false)
, mFadedIn(false)
, mFadedOut(false)
, x664_31_(false)
, x665_24_(true)
, x665_25_(false)
, mShouldSwoosh(false)
//, x665_27_playerInLeashRange(false)
, mInRange(false)
, mAggressive(false)
, x668_(0.f)
, x66c_(0.f)
, x670_(0.f)
, mCoverPoint(kInvalidUniqueId)
, mFloorLevel(0.f)
, mAttackType(-1)
, mBehaveType(mBehaviorEnabled ? kBT_Attack : kBT_None)
, mLurkDelay(1.f)
, mBoneTracking(*GetAnimationData(), rstl::string_l("Head_1"), 80.f * M_PIF / 180.f,
                    CRelAngle::FromDegrees(180.f).AsRadians(), kBTF_None)
, mTeamMgr(kInvalidUniqueId)
, mSpaceWarpTime(0.f)
, mSpaceWarpPosition(CVector3f::Zero())
, x6d8_(1) {
  mProjectileInfo1.Token().Lock();
  mProjectileInfo2.Token().Lock();

  const CPASAnimParmData jumpAnimParms(pas::kAS_Jump, CPASAnimParm::FromEnum(3),
                                       CPASAnimParm::FromEnum(0));
  x668_ = GetModelScale().GetZ() * GetAnimationDistance(jumpAnimParms);
  const CPASAnimParmData slideAnimParms(pas::kAS_Slide, CPASAnimParm::FromEnum(1),
                                        CPASAnimParm::FromReal32(90.f));
  x66c_ = GetModelScale().GetY() * GetAnimationDistance(slideAnimParms);
  const CPASAnimParmData meleeAnimParms(pas::kAS_MeleeAttack, CPASAnimParm::FromEnum(2),
                                        CPASAnimParm::FromEnum(1));
  x670_ = GetModelScale().GetZ() * GetAnimationDistance(meleeAnimParms);

  if (projectileVisor != kInvalidAssetId) {
    mProjectileVisor = gpSimplePool->GetObj(SObjectTag('PART', projectileVisor));
  }

  mKnockBackController.SetEnableBurn(false);
  mKnockBackController.SetEnableLaggedBurnDeath(false);
  mKnockBackController.SetEnableShock(false);
  mKnockBackController.SetEnableFreeze(false);
  SetDrawShadow(false);
  MakeThermalColdAndHot();
}

ENTITY_ACCEPT_IMPL(CChozoGhost)

void CChozoGhost::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_InitializedInArea:
    if (GetActive()) {
      AddToTeam(mgr);
    }
    break;
  case kSM_Activate:
    AddToTeam(mgr);
    break;
  case kSM_Alert:
    if (!mAlert) {
      mAlert = true;
      SetWasHit(true);
    }
    break;
  case kSM_Action:
    if (mFlinch) {
      mAggressive = true;
    }
    break;
  case kSM_Deactivate:
  case kSM_Deleted:
    RemoveFromTeam(mgr);
    break;
  case kSM_Falling:
  case kSM_Jumped:
    if (!GetVerticalMovement()) {
      mMomentum = CVector3f(0.f, 0.f, -GetGravityConstant() * GetMass());
    }
    break;
  default:
    break;
  }
}

void CChozoGhost::Touch(CActor& act, CStateManager& mgr) {
  if (IsVisibleEnough(mgr)) {
    if (CPlayer* player = TCastToPtr< CPlayer >(act)) {
      if (mCurDamageRemTime <= 0.f) {
        mgr.ApplyDamage(
            GetUniqueId(), player->GetUniqueId(), GetUniqueId(), GetContactDamage(),
            CMaterialFilter::MakeIncludeExclude(CMaterialList(kMT_Solid), CMaterialList()),
            CVector3f::Zero());
        mCurDamageRemTime = mDamageWaitTime;
      }
    }
  }
  CPatterned::Touch(act, mgr);
}

bool CChozoGhost::CanBeShot(const CStateManager& mgr, int w1) { return IsVisibleEnough(mgr); }

EWeaponCollisionResponseTypes CChozoGhost::GetCollisionResponseType(const CVector3f&,
                                                                    const CVector3f&,
                                                                    const CWeaponMode&, int) const {
  return kWCR_ChozoGhost;
}

const CDamageVulnerability* CChozoGhost::GetDamageVulnerability() const {
  if (x665_24_) {
    return &CDamageVulnerability::PassThroughVulnerability();
  }

  return CAi::GetDamageVulnerability();
}

uchar CChozoGhost::GetModelAlphau8(const CStateManager& mgr) const {
  uchar ret = 255;
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_XRay || !IsAlive()) {
    ret = mColor.GetAlphau8();
  }

  return ret & 0xFF;
}

// TODO: Alpha register allocation and the model-data reload before bone tracking.
void CChozoGhost::PreRender(CStateManager& mgr, const CFrustumPlanes& frustum) {
  const bool xray = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay;
  mDrawParticles = !xray;
  if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal) {
    SetCalculateLighting(false);
    ActorLights()->BuildConstantAmbientLighting(CColor::White());
  } else {
    SetCalculateLighting(true);
  }
  CColor color = mColor;
  const uchar alpha = GetModelAlphau8(mgr);
  if (alpha < 255 || color.GetRedu8() != 0) {
    if (color.GetRedu8() != 0) {
      const uchar value = CMath::Max(0, 255 - 2 * color.GetRedu8());
      color.SetRed(static_cast< uchar >(255));
      color.SetGreen(value);
      color.SetBlue(value);
    } else {
      color = CColor::White();
    }
    SetModelFlags(CModelFlags::AlphaBlended(
        CColor(color.GetRedu8(), color.GetGreenu8(), color.GetBlueu8(), alpha)));
  } else {
    SetModelFlags(CModelFlags::Normal());
  }
  CActor::PreRender(mgr, frustum);
  mBoneTracking.PreRender(mgr, *ModelData()->AnimationData(), GetTransform(), GetModelScale(),
                              *BodyCtrl());
}

void CChozoGhost::Render(const CStateManager& mgr) const {
  if (mSpaceWarpTime > 0.f) {
    mgr.DrawSpaceWarp(mSpaceWarpPosition,
                      CMath::FastSinR(M_PIF * mSpaceWarpTime / mFadeOutDelay));
  }
  const bool xray = mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_XRay;
  const TAreaId areaId = GetCurrentAreaId();
  if (xray) {
    CElementGen::SetSubtractBlend(true);
    CElementGen::SetMoveRedToAlphaBuffer(true);
    CGraphics::SetFog(kRFM_PerspLin, 0.f, 75.f, CColor::Black());
    GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnFirst();
    mgr.SetupFogForArea3XRange(areaId);
  }
  CPatterned::Render(mgr);
  if (xray) {
    CGraphics::SetFog(kRFM_PerspLin, 0.f, 75.f, CColor::Black());
    GetAnimationData()->GetParticleDB().RenderSystemsToBeDrawnLast();
    mgr.SetupFogForArea(areaId);
    CElementGen::SetSubtractBlend(false);
    CElementGen::SetMoveRedToAlphaBuffer(false);
  }
}

bool CChozoGhost::IsVisibleEnough(const CStateManager& mgr) const {
  return GetModelAlphau8(mgr) > 31;
}

CVector3f CChozoGhost::GetOrigin(const CStateManager& mgr, const CTeamAiRole& role,
                                 const CVector3f& aimPos) const {
  return GetTranslation();
}

void CChozoGhost::AddToTeam(CStateManager& mgr) {
  if (mTeamMgr == kInvalidUniqueId) {
    mTeamMgr = CTeamAiMgr::GetTeamAiMgr(*this, mgr);
  }

  if (mTeamMgr == kInvalidUniqueId) {
    return;
  }

  if (CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamMgr))) {
    teamMgr->AssignTeamAiRole(*this, CTeamAiRole::kTAR_Projectile, CTeamAiRole::kTAR_Unknown,
                              CTeamAiRole::kTAR_Invalid);
  }
}

void CChozoGhost::RemoveFromTeam(CStateManager& mgr) {
  if (mTeamMgr == kInvalidUniqueId) {
    return;
  }

  CTeamAiMgr* teamMgr = TCastToPtr< CTeamAiMgr >(mgr.ObjectById(mTeamMgr));
  if (teamMgr && teamMgr->IsPartOfTeam(GetUniqueId())) {
    teamMgr->RemoveTeamAiRole(GetUniqueId());
    mTeamMgr = kInvalidUniqueId;
  }
}

void CChozoGhost::FloatToLevel(const float f1, const float dt) {
  CVector3f translation = GetTranslation();
  const float floatAmt = ((f1 - translation[kDZ]) * 4.f);
  translation[kDZ] += floatAmt * dt;
  SetTranslation(translation);
}

bool CChozoGhost::IsOnGround() const { return mOnGround; }

void CChozoGhost::FindBestAnchor(CStateManager& mgr) {
  float bestScore = FLT_MAX;
  mPlayerInLeashRange = false;
  CScriptCoverPoint* target = nullptr;
  CObjectList& waypoints = mgr.ObjectListById(kOL_AiWaypoint);
  const int random = mgr.Random()->Next() % 100;
  const int range =
      random < mNearChance ? 0 : (random < mNearChance + mMidChance ? 1 : 2);
  float nearWeight = 10.f * x658_;
  float midWeight = nearWeight;
  float farWeight = nearWeight;
  switch (range) {
  case 0:
    farWeight *= 10.f;
    midWeight *= 5.f;
    break;
  case 1:
    nearWeight *= 10.f;
    farWeight *= 5.f;
    break;
  case 2:
    nearWeight *= 10.f;
    midWeight *= 5.f;
    break;
  }
  for (int i = waypoints.GetFirstObjectIndex(); i != -1; i = waypoints.GetNextObjectIndex(i)) {
    if (CScriptCoverPoint* cover = TCastToPtr< CScriptCoverPoint >(waypoints[i])) {
      if (cover->GetActive() && !cover->GetInUse(kInvalidUniqueId) &&
          cover->GetCurrentAreaId() == GetCurrentAreaId()) {
        const float distance = (cover->GetTranslation() - GetTranslation()).Magnitude();
        if (!(distance < 2.f * x66c_)) {
          float score = CMath::Max(0.f, x654_ - distance);
          CVector3f delta = cover->GetTranslation() - mgr.GetPlayer()->GetTranslation();
          const float playerDistance = delta.Magnitude();
          if (!(playerDistance < mMinAttackRange)) {
            if (CMath::AbsF(delta.GetZ()) / playerDistance > 0.2f) {
              score += (20.f * x658_) * (CMath::AbsF(delta.GetZ()) / playerDistance - 0.2f);
            }
            if (playerDistance < x654_) {
              score += nearWeight;
              if (score < bestScore) {
                delta *= 1.f / playerDistance;
                score +=
                    (10.f * x658_) *
                    (1.f - CVector3f::Dot(mgr.GetPlayer()->GetTransform().GetForward(), delta));
              }
            } else if (playerDistance < x658_) {
              score += midWeight;
              if (score < bestScore) {
                delta *= 1.f / playerDistance;
                score +=
                    (10.f * x658_) *
                    (1.f - CVector3f::Dot(mgr.GetPlayer()->GetTransform().GetForward(), delta));
              }
            } else {
              score += farWeight;
            }
            if (score < bestScore) {
              score += x658_ * mgr.Random()->Float();
              if (score < bestScore) {
                bestScore = score;
                target = cover;
                mPlayerInLeashRange = playerDistance > mLeashRadius;
              }
            }
          }
        }
      }
    }
  }
  if (target) {
    mDestObj = target->GetUniqueId();
    SetDestPos(target->GetTranslation());
    ReleaseCoverPoint(mgr, mCoverPoint);
    SetCoverPoint(target, mCoverPoint);
  } else if (mgr.GetPlayer()->GetCurrentAreaId() == GetCurrentAreaId()) {
    mDestObj = mgr.GetPlayer()->GetUniqueId();
    CVector3f destPos =
        mgr.GetPlayer()->GetTranslation() -
        x654_ * (mgr.GetPlayer()->GetTranslation() - GetTranslation()).AsNormalized();
    const CRayCastResult result = mgr.RayStaticIntersection(
        destPos, CVector3f::Down(), 8.f, CMaterialFilter::MakeInclude(CMaterialList(kMT_Floor)));
    if (result.IsValid()) {
      destPos = result.GetPoint();
    }
    SetDestPos(destPos);
  } else {
    mDestObj = kInvalidUniqueId;
    mDestPos = GetTranslation();
  }
}

const CChozoGhost::CBehaveChance& CChozoGhost::ChooseBehaveChanceRange(CStateManager& mgr) const {
  const float dist = (GetTranslation() - mgr.GetPlayer()->GetTranslation()).Magnitude();
  if (dist < x654_) {
    return mBehaveChance1;
  }
  if (dist < x658_) {
    return mBehaveChance2;
  }

  return mBehaveChance3;
}

void CChozoGhost::SetWarpPosition(CStateManager& mgr, const CVector3f& dir) {
  const CVector3f center = GetBoundingBox().GetCenterPoint();
  const CRayCastResult result = mgr.RayStaticIntersection(
      center + dir * 8.f, -dir, 8.f, CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
  if (result.IsValid()) {
    mSpaceWarpPosition = result.GetPoint();
  } else {
    mSpaceWarpPosition = center + dir;
  }
}

void CChozoGhost::InActive(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    if (!BodyCtrl()->GetIsActive()) {
      BodyCtrl()->Activate(mgr);
    }

    if (x63c_ == 3) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
      mColor.SetAlpha(1.f);
    } else {
      BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
      mColor.SetAlpha(0.f);
    }
    RemoveMaterial(kMT_Solid, mgr);
    mMomentum = CVector3f::Zero();
    x665_24_ = true;
  } break;
  case kStateMsg_Update:
    break;
  default:
    break;
  }
}

// TODO: The equality subtraction has its operands reversed.
bool CChozoGhost::AIStage(CStateManager& mgr, float arg) {
  return static_cast< int >(arg) == x63c_;
}
void CChozoGhost::Growth(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    StateMachineState().SetDelay(mFadeOutDelay);
    BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
    mAlphaDelta = 1.f;
    mFadedIn = true;
    if (mFadeOutDelay > 0.f) {
      mSpaceWarpTime = mFadeOutDelay;
      SetWarpPosition(mgr, CVector3f::Up());
    }
  } break;
  case kStateMsg_Update:
    break;
  case kStateMsg_Deactivate:
    x665_24_ = false;
    mBoneTracking.SetActive(true);
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    break;
  }
}

void CChozoGhost::Generate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    StateMachineState().SetDelay(mFadeOutDelay);
    SetAnimationState(kAS_Ready);
    mOnGround = false;
    const CRayCastResult result =
        mgr.RayStaticIntersection(GetTranslation(), CVector3f::Down(), 100.f,
                                  CMaterialFilter::MakeInclude(CMaterialList(kMT_Floor)));
    if (result.IsValid()) {
      mFloorLevel = result.GetPoint().GetZ();
    } else {
      mFloorLevel = mgr.GetPlayer()->GetTranslation().GetZ();
    }

    mAlphaDelta = 1.f;
    mFadedIn = true;

    if (mFadeOutDelay > 0.f) {
      mSpaceWarpTime = mFadeOutDelay;
      SetWarpPosition(mgr, CVector3f::Down());
    }
  } break;
  case kStateMsg_Update: {
    TryCommand(mgr, pas::kAS_Jump, &CPatterned::TryJump, 0);
    switch (GetAnimationState()) {
    case kAS_Repeat: {
      BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
      if (mOnGround) {
        break;
      }

      if (GetTranslation().GetZ() < mFloorLevel + x668_) {
        CVector3f newPos = GetTranslation();
        newPos.SetZ(mFloorLevel + x668_);
        SetTranslation(newPos);
        mOnGround = true;
      }
    } break;
    case kAS_Over: {
      mBoneTracking.SetActive(true);
      mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
      FloatToLevel(mFloorLevel, arg);
    } break;
    default:
      break;
    }
  } break;
  case kStateMsg_Deactivate: {
    SetAnimationState(kAS_NotReady);
    x665_24_ = false;
    mOnGround = false;
  } break;
  }
}

void CChozoGhost::WallDetach(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    StateMachineState().SetDelay(mFadeOutDelay);
    mAlphaDelta = 1.f;
    mFadedIn = false;

    if (mFadeOutDelay > 0.f) {
      mSpaceWarpTime = mFadeOutDelay;
      SetWarpPosition(mgr, GetTransform().GetForward());
    }

    const CActor* wp = nullptr;
    const TUniqueId wpId = GetConnectedObject(mgr, kSS_Attack, kSM_Follow);
    if (wpId != kInvalidUniqueId) {
      wp = TCastToConstPtr< CActor >(mgr.GetObjectById(wpId));
    }

    if (wp) {
      SetDestPos(wp->GetTranslation());
    } else {
      SetDestPos(GetTranslation() + GetTransform().GetForward() * (2.f * x66c_));
    }

    SendScriptMsgs(kSS_Attack, mgr, kSM_Follow);
  } break;
  case kStateMsg_Update: {

  } break;
  case kStateMsg_Deactivate: {
    mBoneTracking.SetActive(true);
    mBoneTracking.SetTarget(mgr.GetPlayer()->GetUniqueId());
    x665_24_ = false;
    mBehaveType = kBT_Move;
  } break;
  }
}
void CChozoGhost::Run(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    SetWasHit(false);
    KnockBackCtrl().EnableAnimReaction(kAR_KnockBack, false);
    mInRange = false;
  } break;
  case kStateMsg_Update: {
    BodyCtrl()->CommandMgr().DeliverCmd(
        CBCLocomotionCmd(mSteeringBehaviors.Seek(*this, mDestPos), CVector3f::Zero(), 1.f));
    if (!mShouldSwoosh) {
      break;
    }

    mFloorLevel = mDestPos.GetZ();
    FloatToLevel(mFloorLevel, arg);
    AnimationData()->SetParticleEffectState(skSpeedSwooshName, true, mgr);
    x665_24_ = false;
    if (mInRange) {
      break;
    }

    const float movement = arg * mVelocity.Magnitude();
    const float range = x66c_ + movement * 2.5f;
    // TODO: These two vector temporaries use reversed stack slots.
    const CVector3f& translation = GetTranslation();
    const CVector3f& delta = translation - mDestPos;
    mInRange = delta.MagSquared() < range * range;
  } break;
  case kStateMsg_Deactivate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Crouch);
    SetDestPos(mgr.GetPlayer()->GetTranslation());
    AnimationData()->SetParticleEffectState(skSpeedSwooshName, false, mgr);
    KnockBackCtrl().EnableAnimReaction(kAR_KnockBack, true);
    mInRange = false;
    break;
  }
}

bool CChozoGhost::InRange(CStateManager& mgr, float arg) { return mInRange; }

void CChozoGhost::SelectTarget(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    FindBestAnchor(mgr);
    break;
  }
  }
}

bool CChozoGhost::ShouldAttack(CStateManager& mgr, float arg) {
  return mBehaveType == kBT_Attack;
}

void CChozoGhost::Attack(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    CTeamAiMgr::AddAttacker(kAT_Projectile, mgr, mTeamMgr, GetUniqueId());
    SetAnimationState(kAS_Ready);
    switch (x6d8_) {
    case 1:
      mAttackType = 3;
      break;
    case 2:
      mAttackType = 4;
      break;
    case 3:
      mAttackType = 5;
      break;
    }
    if (x665_25_) {
      const CRayCastResult result =
          mgr.RayStaticIntersection(GetTranslation() + 0.5f * CVector3f::Up(), CVector3f::Up(),
                                    x670_, CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
      if (!result.IsValid()) {
        mAttackType = 2;
        KnockBackCtrl().EnableAnimReaction(kAR_KnockBack, false);
      }
    }
    mMomentum = CVector3f::Zero();
    mConstantForce = CVector3f::Zero();
    break;
  }
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_MeleeAttack, &CPatterned::TryMeleeAttack, mAttackType);
    BodyCtrl()->CommandMgr().DeliverTargetVector(mgr.GetPlayer()->GetTranslation() -
                                                 GetTranslation());
    if (mAttackType != 2) {
      FloatToLevel(mFloorLevel, arg);
    }
    break;
  case kStateMsg_Deactivate:
    SetAnimationState(kAS_NotReady);
    mShouldSwoosh = false;
    KnockBackCtrl().EnableAnimReaction(kAR_KnockBack, true);
    CTeamAiMgr::ResetTeamAiRole(kAT_Projectile, mgr, mTeamMgr, GetUniqueId(), true);
    break;
  }
}

void CChozoGhost::Land(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Update: {
    FloatToLevel(mFloorLevel, arg);
    if (CMath::AbsF(mFloorLevel - GetTranslation().GetZ()) < 0.05f) {
      StateMachineState().SetCodeTrigger();
    }
    break;
  }
  }
}

void CChozoGhost::Shuffle(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate: {
    const CBehaveChance& chance = ChooseBehaveChanceRange(mgr);
    const CTeamAiRole* role = CTeamAiMgr::GetTeamAiRole(mgr, mTeamMgr, GetUniqueId());
    if (role &&
        (role->GetTeamAiRole() != CTeamAiRole::kTAR_Projectile ||
         !CTeamAiMgr::CanAcceptAttacker(kAT_Projectile, mgr, mTeamMgr, GetUniqueId()))) {
      mBehaveType = kBT_Attack;
    }
    mBehaveType = ChooseBehaveChanceRange(mgr).GetBehave(mBehaveType, mgr);
    switch (mBehaveType) {
    case kBT_Lurk:
      mLurkDelay = chance.GetLurkTime();
      break;
    case kBT_Attack:
      x665_25_ = mgr.Random()->Float() < chance.GetChargeAttack();
      x6d8_ = mgr.Random()->Next() % chance.GetNumBolts() + 1;
      break;
    default:
      break;
    }
    x664_31_ = false;
    mPlayerInLeashRange = false;
    break;
  }
  }
}

bool CChozoGhost::ShouldTaunt(CStateManager& mgr, float arg) {
  return mBehaveType == kBT_Taunt;
}

void CChozoGhost::Taunt(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    SetAnimationState(kAS_Ready);
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Taunt, &CPatterned::TryTaunt, 0);
    FloatToLevel(mFloorLevel, arg);
    break;
  case kStateMsg_Deactivate:
    SetAnimationState(kAS_NotReady);
    mShouldSwoosh = false;
    break;
  }
}

void CChozoGhost::Hurled(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    SetVerticalMovement(false);
    mOnGround = false;
    x665_24_ = true;
    break;
  case kStateMsg_Update:
    mAlphaDelta = 2.f;
    if (!mOnGround) {
      if (GetVelocityWR().GetZ() < 0.f) {
        const CRayCastResult result =
            mgr.RayStaticIntersection(GetTranslation() + CVector3f::Up(), CVector3f::Down(), 2.f,
                                      CMaterialFilter::MakeInclude(CMaterialList(kMT_Floor)));
        if (result.IsValid() && result.GetTime() < 1.05f) {
          mOnGround = true;
          mMomentum = CVector3f::Zero();
          SetVelocityWR(CVector3f(GetVelocityWR().GetX(), GetVelocityWR().GetY(), 0.f));
          mFloorLevel = result.GetPoint().GetZ();
          StateMachineState().SetCodeTrigger();
        }
      }
      if (!mOnGround && GetStateMachineTime() > mHurlRecoverTime) {
        BodyCtrl()->CommandMgr().DeliverCmd(CBodyStateCmd(kBSC_ExitState));
        BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
        StateMachineState().SetCodeTrigger();
      }
    }
    break;
  case kStateMsg_Deactivate:
    SetVerticalMovement(true);
    mMomentum = CVector3f::Zero();
    break;
  }
}

void CChozoGhost::Lurk(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    StateMachineState().SetDelay(mLurkDelay);
    break;
  case kStateMsg_Update:
    FloatToLevel(mFloorLevel, arg);
    break;
  }
}

bool CChozoGhost::ShouldMove(CStateManager& mgr, float arg) { return mBehaveType == kBT_Move; }

bool CChozoGhost::Leash(CStateManager& mgr, float arg) {
  return mPlayerInLeashRange || CPatterned::Leash(mgr, arg);
}

bool CChozoGhost::ShouldFlinch(CStateManager& mgr, float arg) { return mFlinch; }

bool CChozoGhost::AggressionCheck(CStateManager& mgr, float arg) { return mAggressive; }

void CChozoGhost::Deactivate(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    mBoneTracking.SetActive(false);
    ReleaseCoverPoint(mgr, mCoverPoint);
    SetAnimationState(kAS_Ready);
    x665_24_ = true;
    break;
  case kStateMsg_Update:
    TryCommand(mgr, pas::kAS_Generate, &CPatterned::TryGenerateDeactivate, 1);
    if (GetAnimationState() == kAS_Repeat) {
      BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    }
    break;
  case kStateMsg_Deactivate:
    SetAnimationState(kAS_NotReady);
    break;
  }
}

void CChozoGhost::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    ReleaseCoverPoint(mgr, mCoverPoint);
    mAlphaDelta = 4.f;
    mFadedOut = false;
    mFadedIn = false;
    mBoneTracking.SetActive(false);
    Stop();
    break;
  case kStateMsg_Update:
    Stop();
    mMomentum = CVector3f::Zero();
    break;
  }
}

CProjectileInfo* CChozoGhost::ProjectileInfo() {
  if (mAttackType == 2) {
    return &mProjectileInfo1;
  }
  return &mProjectileInfo2;
}

void CChozoGhost::DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node,
                                  EUserEventType type, float dt) {
  bool handled = false;
  switch (type) {
  case kUE_Projectile: {
    const CTransform4f locator = GetLctrTransform(node.GetLocatorName());
    const CVector3f aimPos = mgr.GetPlayer()->GetAimPosition(mgr, 0.f);
    const CTransform4f projectileXf = CTransform4f::LookAt(locator.GetTranslation(), aimPos);
    switch (mAttackType) {
    case 2: {
      CGameProjectile* projectile = LaunchProjectile(
          projectileXf, mgr, 2,
          static_cast< CWeapon::EProjectileAttrib >(CWeapon::kPA_BigStrike |
                                                    CWeapon::kPA_StaticInterference),
          true, mProjectileVisor, mSoundProjectileVisor, false, CVector3f(1.f, 1.f, 1.f));
      if (projectile) {
        projectile->SetDamageDuration(x62c_);
        projectile->SetInterferenceDuration(x62c_);
        projectile->SetMinHomingDistance(x634_);
      }
      break;
    }
    default: {
      CGameProjectile* projectile = LaunchProjectile(
          projectileXf, mgr, 5,
          static_cast< CWeapon::EProjectileAttrib >(CWeapon::kPA_DamageFalloff |
                                                    CWeapon::kPA_StaticInterference),
          true, mProjectileVisor, mSoundProjectileVisor, false, CVector3f(1.f, 1.f, 1.f));
      if (projectile) {
        const float speed = ProjectileInfo()->GetProjectileSpeed();
        if (speed > 0.f) {
          projectile->SetDamageFalloffSpeed(80.f / speed);
        }
        projectile->SetDamageDuration(x62c_);
        projectile->SetInterferenceDuration(x62c_);
        projectile->SetMinHomingDistance(x634_);
      }
      break;
    }
    }
    handled = true;
    break;
  }
  case kUE_FadeIn:
    if (mFadedOut) {
      mAlphaDelta = 2.f;
      CSfxManager::AddEmitter(mSfxFadeIn, GetTranslation(), CVector3f::Zero(), false, false,
                              CSfxManager::kMedPriority, CSfxManager::kAllAreas);
    }
    AddMaterial(kMT_Target, mgr);
    mFadedOut = false;
    mFadedIn = true;
    handled = true;
    break;
  case kUE_FadeOut:
    if (mFadedIn) {
      mAlphaDelta = -2.f;
      CSfxManager::AddEmitter(mSfxFadeOut, GetTranslation(), CVector3f::Zero(), false, false,
                              CSfxManager::kMedPriority, CSfxManager::kAllAreas);
    }
    RemoveMaterial(kMT_Target, mgr);
    mFadedIn = false;
    mFadedOut = true;
    mShouldSwoosh = true;
    handled = true;
    break;
  }
  if (!handled) {
    CPatterned::DoUserAnimEvent(mgr, node, type, dt);
  }
  if (type == kUE_Delete) {
    mAlphaDelta = -1.f;
  }
}

void CChozoGhost::KnockBack(const CVector3f& dir, CStateManager& mgr, const CDamageInfo& info,
                            float magnitude, bool direct, const bool inDeferred) {
  if (!IsAlive()) {
    KnockBackCtrl().EnableAnimReaction(kAR_Hurled, false);
  } else if (!KnockBackCtrl().TestAvailableState(kAR_KnockBack) &&
             info.GetWeaponMode().IsCharged()) {
    KnockBackCtrl().SetAnimationStateRange(kAR_Hurled, kAR_Fall);
  }
  CPatterned::KnockBack(dir, mgr, info, magnitude, direct, inDeferred);
  KnockBackCtrl().SetAnimationStateRange(kAR_Flinch, kAR_Fall);
  if (IsAlive()) {
    if (KnockBackCtrl().GetActiveParms().mAnimState == kAR_Hurled) {
      StateMachineState().SetState(mgr, *this, GetStateMachine(), rstl::string_l("Hurled"));
    }
  } else {
    Stop();
    mMomentum = CVector3f::Zero();
  }
}

void CChozoGhost::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CPatterned::Think(dt, mgr);
  UpdateThermalFrozenState(false);
  mBoneTracking.Update(dt);
  mSpaceWarpTime = CMath::Max(0.f, mSpaceWarpTime - dt);
  SetTargetable(IsVisibleEnough(mgr));
}
