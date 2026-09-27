#include "MetroidPrime/Enemies/CFireFlea.hpp"

#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Math/CloseEnough.hpp"
#include "MetroidPrime/BodyState/CBodyController.hpp"
#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CEntityInfo.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/CWorld.hpp"
#include "MetroidPrime/Cameras/CCameraFilterPass.hpp"
#include "MetroidPrime/Enemies/CKnockBackMgr.hpp"
#include "MetroidPrime/Enemies/CPatterned.hpp"
#include "MetroidPrime/Player/CPlayerState.hpp"
#include "MetroidPrime/TCastTo.hpp"

const CColor CFireFlea::CDeathCameraEffect::skEndFadeColor(1.f, 1.f, 0.5f, 1.f);
const CColor CFireFlea::CDeathCameraEffect::skStartFadeColor(1.f, 0.f, 0.f, 0.f);
CColor CFireFlea::CDeathCameraEffect::sCurrentFadeColor = CColor(0.f, 0.f, 0.f, 0.f);
int CFireFlea::sLightIdx = 0;

CFireFlea::CDeathCameraEffect::CDeathCameraEffect(TUniqueId uid, TAreaId aid,
                                                  const rstl::string& name)
: CEntity(uid, CEntityInfo(aid, CEntity::NullConnectionList), true, name)
, mStartFadeTime(13)
, mFadeDuration(5)
, mReverseFadeDuration(60)
, mTotalFadeDuration(190)
, mCurrentTime(0) {}

void CFireFlea::CDeathCameraEffect::PreThink(float dt, CStateManager& mgr) {
  CCameraFilterPass& pass = mgr.CameraFilterPass(CStateManager::kCFS_Five);
  const uint endFadeTime = mStartFadeTime + mFadeDuration;
  const uint reverseFadeStartTime = endFadeTime + mReverseFadeDuration;
  const uint endTransitionTime = reverseFadeStartTime + mTotalFadeDuration;

  if (mCurrentTime >= mStartFadeTime && mCurrentTime <= endFadeTime) {
    sCurrentFadeColor =
        CColor::Add(sCurrentFadeColor,
                    CColor::Lerp(skStartFadeColor, skEndFadeColor,
                                 (float)(mCurrentTime - mStartFadeTime) / mFadeDuration));
    pass.SetFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen, 0.f,
                   sCurrentFadeColor, -1);
  } else if (mCurrentTime >= reverseFadeStartTime && mCurrentTime <= endTransitionTime) {
    sCurrentFadeColor = CColor::Add(
        sCurrentFadeColor,
        CColor::Lerp(skEndFadeColor, skStartFadeColor,
                     (float)(mCurrentTime - reverseFadeStartTime) / mTotalFadeDuration));
    pass.SetFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen, 0.f,
                   sCurrentFadeColor, -1);
  } else if (mCurrentTime >= endFadeTime) {
    sCurrentFadeColor = skEndFadeColor;
    pass.SetFilter(CCameraFilterPass::kFT_Blend, CCameraFilterPass::kFS_Fullscreen, 0.f,
                   sCurrentFadeColor, -1);
  }

  if (mCurrentTime == endTransitionTime) {
    pass.DisableFilter(0.f);
    mgr.DeleteObjectRequest(GetUniqueId());
    mCurrentTime = 0;
  } else {
    ++mCurrentTime;
  }

  if (mgr.GetPlayerState()->GetActiveVisor(mgr) != CPlayerState::kPV_Thermal) {
    pass.DisableFilter(0.f);
  }
}

void CFireFlea::CDeathCameraEffect::Think(float dt, CStateManager& mgr) {
  sCurrentFadeColor = CColor(0.f, 0.f, 0.f, 0.f);
}

ENTITY_ACCEPT_IMPL(CFireFlea::CDeathCameraEffect)

CFireFlea::CFireFlea(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CModelData& mData,
                     const CActorParameters& actParams, const CPatternedInfo& pInfo, float f1)
: CPatterned(kC_FireFlea, uid, name, kFT_Zero, info, xf, mData, pInfo, kMT_Flyer, kCT_One,
             kBT_Flyer, actParams, kCS_Small)
, x568_(1.f)
, x56c_(f1)
, xd74_(CVector3f::Zero())
, mTargetPos(CVector3f::Zero())
, mPathFind(nullptr, 1 | 2, pInfo.GetPathfindingIndex(), 1.f, 1.f) {
  CMaterialList exclude = GetMaterialFilter().GetExcludeList();
  exclude.Add(CMaterialList(kMT_Character));
  CMaterialList include = GetMaterialFilter().GetIncludeList();
  SetMaterialFilter(CMaterialFilter::MakeIncludeExclude(include, exclude));
  ModelData()->AnimationData()->SetParticleLightIdx(sLightIdx);
  ++sLightIdx;
}

ENTITY_ACCEPT_IMPL(CFireFlea)
void CFireFlea::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr) {
  CPatterned::AcceptScriptMsg(msg, sender, mgr);

  switch (msg) {
  case kSM_Registered:
    BodyCtrl()->Activate(mgr);
    break;
  case kSM_InitializedInArea: {
    TAreaId aid = GetCurrentAreaId();
    const CGameArea& area = mgr.GetWorld()->GetAreaAlways(aid);
    mPathFind.SetArea(area.GetPostConstructed()->mPathArea);
    mPathFind.SetPadding(50.f);
    break;
  }
  default:
    break;
  }
}

void CFireFlea::Think(float dt, CStateManager& mgr) {
  if (!GetActive()) {
    return;
  }
  CPatterned::Think(dt, mgr);
}

bool CFireFlea::HearShot(CStateManager& mgr, float arg) {
  mNearList.clear();
  CVector3f translation = GetTranslation();
  CAABox box(translation - CVector3f(10.f, 10.f, 10.f), translation + CVector3f(10.f, 10.f, 10.f));
  CMaterialFilter filter = CMaterialFilter::MakeInclude(CMaterialList(kMT_Projectile));
  mgr.BuildNearList(mNearList, box, filter, nullptr);
  return HeardShot();
}

void CFireFlea::Patrol(CStateManager& mgr, EStateMsg msg, float arg) {
  if (!close_enough(GetMoveVector(), CVector3f::Zero())) {
    MoveVector().Normalize();
  }

  SetMoveVector(AdjustMovementVec(mgr, GetMoveVector()));
  CPatterned::Patrol(mgr, msg, arg);
  if (GetPatrolState() == kPS_Done) {
    mgr.DeleteObjectRequest(GetUniqueId());
  }
}

bool CFireFlea::MoveTooCloseToWater(CStateManager& mgr, const CVector3f& forward) const {
  TEntityList nearList;
  mgr.BuildNearList(nearList, GetTranslation(), forward, 2.f, CMaterialFilter::GetPassEverything());

  for (TEntityList::const_iterator iter = nearList.begin(); iter != nearList.end(); ++iter) {
    TUniqueId uid = *iter;
    if (TCastToConstPtr< CScriptWater >(mgr.GetObjectById(uid))) {
      return true;
    }
  }

  return false;
}

CVector3f CFireFlea::AdjustMovementVec(CStateManager& mgr, const CVector3f& forward) const {
  CVector3f movement = forward;
  const float magnitude = movement.Magnitude();
  if (magnitude > 0.f) {
    const CVector3f direction = movement.AsNormalized();
    const CRayCastResult result = mgr.RayStaticIntersection(
        GetTranslation(), direction, 1.f, CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
    const bool nearWater = MoveTooCloseToWater(mgr, direction);
    if (result.IsValid() || nearWater) {
      const CVector3f& worldUp = CVector3f::Up();
      const float upX = worldUp.GetX();
      const float upY = worldUp.GetY();
      const float upZ = worldUp.GetZ();
      const float dirX = direction.GetX();
      const float dirY = direction.GetY();
      const float dirZ = direction.GetZ();
      const float rightX = upY * dirZ - dirY * upZ;
      const float rightY = upZ * dirX - dirZ * upX;
      const float rightZ = upX * dirY - dirX * upY;
      const CVector3f right = CVector3f(rightX, rightY, rightZ).AsNormalized();
      const CRayCastResult rightResult = mgr.RayStaticIntersection(
          GetTranslation(), right, 1.f, CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
      if (rightResult.IsValid()) {
        const CVector3f left = -right;
        const CRayCastResult leftResult = mgr.RayStaticIntersection(
            GetTranslation(), left, 1.f, CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
        if (leftResult.IsValid()) {
          const CVector3f up = CVector3f::Cross(direction, right);
          const CRayCastResult upResult = mgr.RayStaticIntersection(
              GetTranslation(), up, 1.f, CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
          if (upResult.IsValid()) {
            const CVector3f down = -up;
            const CRayCastResult downResult =
                mgr.RayStaticIntersection(GetTranslation(), down, 1.f,
                                          CMaterialFilter::MakeInclude(CMaterialList(kMT_Solid)));
            if (downResult.IsInvalid()) {
              movement = magnitude * down;
            } else {
              movement = -movement;
            }
          } else {
            movement = magnitude * up;
          }
        } else {
          movement = magnitude * left;
        }
      } else {
        movement = magnitude * right;
      }
    }
    return movement;
  }
  return CVector3f::Zero();
}

void CFireFlea::Flee(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Lurk);
    break;
  case kStateMsg_Update:
    if (mNearList.size() == 0) {
      xd74_ = AdjustMovementVec(mgr, xd74_);
      BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(xd74_, CVector3f::Zero(), 1.f));
    } else {
      for (TEntityList::const_iterator it = mNearList.begin(); it != mNearList.end();
           ++it) {
        if (const CActor* actor = static_cast< const CActor* >(mgr.GetObjectById(*it))) {
          CVector3f direction = mSteeringBehaviors.Flee(*this, actor->GetTranslation());
          direction = AdjustMovementVec(mgr, direction);
          xd74_ = direction;
          BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(direction, CVector3f::Zero(), 1.f));
        }
      }
    }
    break;
  case kStateMsg_Deactivate:
    BodyCtrl()->SetLocomotionType(pas::kLT_Relaxed);
    break;
  default:
    break;
  }
}

void CFireFlea::Dead(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Activate:
    if (mgr.GetPlayerState()->GetActiveVisor(mgr) == CPlayerState::kPV_Thermal) {
      const rstl::string name("");
      mgr.AddObject(rs_new CDeathCameraEffect(mgr.AllocateUniqueId(), GetCurrentAreaId(), name));
    }
    break;
  case kStateMsg_Update:
  case kStateMsg_Deactivate:
  default:
    break;
  }
}

void CFireFlea::TargetPatrol(CStateManager& mgr, EStateMsg msg, float arg) {
  switch (msg) {
  case kStateMsg_Deactivate:
    break;
  case kStateMsg_Activate:
    CPatterned::Patrol(mgr, msg, arg);
    UpdateDest(mgr);
    mTargetPos = mDestPos;
    break;
  case kStateMsg_Update:
    if (GetSearchPath()) {
      if (GetSearchPath()->GetResult() != CPathFindSearch::kR_Success) {
        CVector3f closestPoint = CVector3f::Zero();
        if (GetSearchPath()->FindClosestReachablePoint(GetTranslation(), closestPoint) ==
            CPathFindSearch::kR_Success) {
          CVector3f direction = mSteeringBehaviors.Arrival(*this, mTargetPos, 5.f);
          direction = AdjustMovementVec(mgr, direction);
          BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(direction, CVector3f::Zero(), 1.f));
        }
      } else {
        CPatterned::PathFind(mgr, msg, arg);
      }
    } else {
      BodyCtrl()->CommandMgr().DeliverCmd(CBCLocomotionCmd(
          mSteeringBehaviors.Arrival(*this, mTargetPos, 5.f), CVector3f::Zero(), 1.f));
    }
    break;
  default:
    break;
  }
}

bool CFireFlea::InPosition(CStateManager& mgr, float arg) {
  if (GetDestObj() != kInvalidUniqueId) {
    const CVector3f& delta = mTargetPos - GetTranslation();
    return delta.MagSquared() < 25.f;
  }
  return false;
}

CPathFindSearch* CFireFlea::GetSearchPath() { return &mPathFind; }

bool CFireFlea::Delay(CStateManager& mgr, float arg) {
  return GetStateMachineState().GetTime() > 0.5f;
}
