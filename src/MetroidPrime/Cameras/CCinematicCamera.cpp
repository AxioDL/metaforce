#include "MetroidPrime/Cameras/CCinematicCamera.hpp"

#include "MetroidPrime/CAnimData.hpp"
#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Player/CMorphBall.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/ScriptObjects/CScriptCameraWaypoint.hpp"
#include "MetroidPrime/TCastTo.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerRes.hpp"

#include "Kyoto/Animation/CAnimTreeNode.hpp"
#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/CRandom16.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"

#include "rstl/algorithm.hpp"

CCinematicCamera::CCinematicCamera(const TUniqueId uid, const rstl::string& name,
                                   const CEntityInfo& info, const CTransform4f& xf,
                                   const bool active, const float shotDuration, const float fovy,
                                   const float znear, const float zfar, const float aspect,
                                   const uint flags)
: CGameCamera(uid, active, name, info, xf, fovy, znear, zfar, aspect, kInvalidUniqueId,
              (flags & 0x20) != 0, 0)
, mDuration(shotDuration)
, mT(0.f)
, mOrigFovy(fovy)
, mPassedViewPoint(0)
, mPassedTarget(0)
, mOrigOrientation(CQuaternion::FromMatrix(xf))
, mLookAtId(kInvalidUniqueId)
, mMoveIntoEyePos(CVector3f::Zero())
, mFlags(flags)
, x220_24_(false) {}

CCinematicCamera::~CCinematicCamera() {}

void CCinematicCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {}

void CCinematicCamera::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CVector3f viewPoint = GetTranslation();
    if (mViewPoints.size() > 0) {
      int idx = 0;
      viewPoint = GetInterpolatedSplinePoint(mViewPoints, idx, mT);
      if (idx > mPassedViewPoint) {
        mPassedViewPoint = idx;
        SendArrivedMsg(mViewPointArrivals[mPassedViewPoint], mgr);
      }
    }
    const CQuaternion orientation = GetInterpolatedOrientation(mViewOrientations, mT);
    if ((mFlags & 1) == 0) {
      if (mTargets.size() > 0) {
        int idx = 0;
        CVector3f target = GetInterpolatedSplinePoint(mTargets, idx, mT);
        if (mTargets.size() == 1) {
          if (const CActor* actor =
                  TCastToConstPtr< CActor >(mgr.GetObjectById(mTargetArrivals[0]))) {
            target = actor->GetTranslation();
          } else {
            mT = mDuration;
          }
        }
        if (idx > mPassedTarget) {
          mPassedTarget = idx;
          SendArrivedMsg(mTargetArrivals[mPassedTarget], mgr);
        }
        const CVector3f up = orientation.Transform(CVector3f::Up());
        if (CVector3f(target - viewPoint).DropZ().Magnitude() < 0.0011920929f) {
          SetTranslation(target);
        } else {
          SetTransform(CTransform4f::LookAt(viewPoint, target, up));
        }
      } else {
        SetTransform(orientation.BuildTransform4f(viewPoint));
      }
    } else {
      const CPlayer* player = mgr.GetPlayer();
      CVector3f target = player->GetTranslation();
      if (player->GetMorphballTransitionState() == CPlayer::kMS_Morphed) {
        target += CVector3f(0.f, 0.f, player->GetMorphBall()->GetBallRadius());
      } else {
        target += CVector3f(0.f, 0.f, player->GetEyeHeight());
      }
      const CVector3f up = orientation.Transform(CVector3f::Up());
      if (CVector3f(target - viewPoint).DropZ().Magnitude() < 0.0011920929f) {
        SetTranslation(target);
      } else {
        SetTransform(CTransform4f::LookAt(viewPoint, target, up));
      }
    }
#if defined(TARGET_PC)
    SetFov(GetInterpolatedHFov(mViewHFovs, mT) / mAspect);
#else
    SetFov(GetInterpolatedHFov(mViewHFovs, mT) / GetAspectRatio());
#endif
    if (mLookAtId != kInvalidUniqueId) {
      if (CScriptActor* actor = TCastToPtr< CScriptActor >(mgr.ObjectById(mLookAtId))) {
        if (actor->IsPlayerActor()) {
          const float alpha = GetMoveOutofIntoAlpha();
          actor->SetModelFlags(CModelFlags::AlphaBlended(alpha));
        }
      }
    }
    mT += dt;
    if (mT > mDuration) {
      for (int i = mPassedViewPoint + 1; i < mViewPointArrivals.size(); ++i) {
        SendArrivedMsg(mViewPointArrivals[i], mgr);
      }
      for (int i = mPassedTarget + 1; i < mTargetArrivals.size(); ++i) {
        SendArrivedMsg(mTargetArrivals[i], mgr);
      }
      DeactivateSelf(mgr);
    }
  }
}

void CCinematicCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr) {}

ENTITY_ACCEPT_IMPL(CCinematicCamera)

void CCinematicCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid,
                                       CStateManager& mgr) {
  CGameCamera::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case kSM_InitializedInArea:
    if ((mFlags & 4) != 0 || (mFlags & 2) != 0) {
      for (AUTO(it, GetConnectionList().begin()); it != GetConnectionList().end(); ++it) {
        const TUniqueId id = mgr.GetIdForScript(it->mObjId);
        CEntity* entity = mgr.ObjectById(id);
        if (const CScriptActor* actor = TCastToConstPtr< CScriptActor >(entity)) {
          if (actor->IsPlayerActor()) {
            mLookAtId = id;
            if (it->mMsg != kSM_Deactivate && it->mMsg != kSM_Reset) {
              break;
            }
          }
        }
      }
    }
    break;
  case kSM_Activate:
    CalculateWaypoints(mgr);
    if ((mFlags & 1) == 0 && x220_24_ && mTargets.size() == 0) {
      break;
    }
    mT = 0.f;
    Think(0.f, mgr);
    mgr.CameraManager()->AddCinemaCamera(GetUniqueId(), mgr);
    mPassedViewPoint = 0;
    if (mViewPointArrivals.size() > 0) {
      SendArrivedMsg(mViewPointArrivals[mPassedViewPoint], mgr);
    }
    mPassedTarget = 0;
    if (mTargetArrivals.size() > 0) {
      SendArrivedMsg(mTargetArrivals[mPassedTarget], mgr);
    }
    if ((mFlags & 0x100) != 0) {
      mgr.SetCinematicPause(true);
    }
    break;
  case kSM_Deactivate:
    WasDeactivated(mgr);
    break;
  }
}

void CCinematicCamera::SendArrivedMsg(TUniqueId reciever, CStateManager& mgr) const {
  mgr.SendScriptMsgAlways(reciever, GetUniqueId(), kSM_Arrived);
}

void CCinematicCamera::DeactivateSelf(CStateManager& mgr) {
  SetActive(false);
  SendScriptMsgs(kSS_Inactive, mgr, kSM_None);
  WasDeactivated(mgr);
}

void CCinematicCamera::WasDeactivated(CStateManager& mgr) {
  mgr.CameraManager()->RemoveCinemaCamera(GetUniqueId(), mgr);
  mgr.Player()->MorphBall()->LoadMorphBallModel(mgr);

  if ((mFlags & 0x100) != 0) {
    mgr.SetCinematicPause(false);
  }

  mViewPoints = rstl::vector< CVector3f >();
  mViewOrientations = rstl::vector< CQuaternion >();
  mViewPointArrivals = rstl::vector< TUniqueId >();
  mTargets = rstl::vector< CVector3f >();
  mTargetArrivals = rstl::vector< TUniqueId >();
  mViewHFovs = rstl::vector< float >();
}

void CCinematicCamera::CalculateWaypoints(CStateManager& mgr) {
  const SConnection* firstVP = nullptr;
  const SConnection* firstTarget = nullptr;

  rstl::vector< SConnection >::const_iterator iter = GetConnectionList().begin();

  for (; iter != GetConnectionList().end(); ++iter) {
    if (iter->mState == kSS_CameraPath && iter->mMsg == kSM_Activate) {
      firstVP = &*iter;
    } else if (iter->mState == kSS_CameraTarget && iter->mMsg == kSM_Activate) {
      firstTarget = &*iter;
    }
  }

  mViewPoints.clear();
  mViewPoints.reserve(3);
  mViewOrientations.clear();
  mViewOrientations.reserve(3);
  mViewPointArrivals.clear();
  mViewPointArrivals.reserve(3);
  mTargets.clear();
  mTargets.reserve(3);
  mTargetArrivals.clear();
  mTargetArrivals.reserve(3);
  mViewHFovs.clear();
  mViewHFovs.reserve(3);

  x220_24_ = false;

  if ((mFlags & 0x2) != 0 && (mFlags & 0x200) == 0) {
    GenerateMoveOutofIntoPoints(true, mgr);
  }

  if (firstVP) {
    const CActor* wp =
        TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetIdForScript(firstVP->mObjId)));

    while (wp) {
      mViewPoints.reserve(mViewPoints.size() + 1);
      mViewPoints.push_back(wp->GetTranslation());
      mViewOrientations.reserve(mViewOrientations.size() + 1);
      mViewOrientations.push_back(wp->GetRotation());

      if (const CScriptCameraWaypoint* cwp = TCastToConstPtr< CScriptCameraWaypoint >(wp)) {
        mViewHFovs.reserve(mViewHFovs.size() + 1);
        mViewHFovs.push_back(cwp->GetHFov());
      }

      if (rstl::find(mViewPointArrivals.begin(), mViewPointArrivals.end(),
                     wp->GetUniqueId()) == mViewPointArrivals.end()) {
        mViewPointArrivals.reserve(mViewPointArrivals.size() + 1);
        mViewPointArrivals.push_back(wp->GetUniqueId());
        SConnection randConn(kSS_Active, kSM_UNKM0, kInvalidEditorId);
        if (PickRandomActiveConnection(wp->GetConnectionList(), randConn, mgr)) {
          wp = TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetIdForScript(randConn.mObjId)));
        } else {
          break;
        }
      } else {
        break;
      }
    }
  }

  if (firstTarget) {
    const CActor* tgt =
        TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetIdForScript(firstTarget->mObjId)));
    while (tgt) {
      mTargets.reserve(mTargets.size() + 1);
      mTargets.push_back(tgt->GetTranslation());
      if (rstl::find(mTargetArrivals.begin(), mTargetArrivals.end(), tgt->GetUniqueId()) ==
          mTargetArrivals.end()) {
        mTargetArrivals.reserve(mTargetArrivals.size() + 1);
        mTargetArrivals.push_back(tgt->GetUniqueId());
        SConnection randConn(kSS_Active, kSM_UNKM0, kInvalidEditorId);
        if (PickRandomActiveConnection(tgt->GetConnectionList(), randConn, mgr)) {
          tgt = TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetIdForScript(randConn.mObjId)));
        } else {
          break;
        }
      } else {
        break;
      }
    }
  }

  if ((mFlags & 0x4) != 0 && (mFlags & 0x200) == 0) {
    GenerateMoveOutofIntoPoints(false, mgr);
  }
}

void CCinematicCamera::GenerateMoveOutofIntoPoints(bool outOfEye, CStateManager& mgr) {
  const float distance = gpTweakPlayerRes->GetCinematicMoveOutofIntoPlayerDistance();
  const CPlayer* player = mgr.GetPlayer();
  mViewPoints.reserve(mViewPoints.size() + 2);
  mViewOrientations.reserve(mViewOrientations.size() + 2);
  mViewPointArrivals.reserve(mViewPointArrivals.size() + 2);
  mTargets.reserve(mTargets.size() + 2);
  mTargetArrivals.reserve(mTargetArrivals.size() + 2);

  const CQuaternion q = CQuaternion::FromMatrix(player->GetTransform());
  const CVector3f eyePos = player->GetEyePosition();
  CVector3f behindPos = eyePos;
  CVector3f behindDelta = q.Transform(CVector3f(0.f, -distance, 0.f));
  if (!outOfEye) {
    behindPos += 1.f * behindDelta;
    behindDelta *= -1.f;
  }
  for (int i = 0; i < 2; ++i) {
    mViewPoints.push_back(behindPos);
    mViewOrientations.push_back(q);
    mViewPointArrivals.push_back(player->GetUniqueId());
    mTargets.push_back(eyePos);
    mTargetArrivals.push_back(kInvalidUniqueId);
    behindPos += behindDelta;
  }
  CalculateMoveOutofIntoEyePosition(outOfEye, mgr);
}

void CCinematicCamera::CalculateMoveOutofIntoEyePosition(bool outOfEye, CStateManager& mgr) {
  static const char* leftEyeName = "L_eye";
  static const char* rightEyeName = "R_eye";
  const CPlayer* player = mgr.GetPlayer();
  CQuaternion q = CQuaternion::FromMatrix(player->GetTransform());
  CVector3f eyePos = player->GetEyePosition();
  if (mLookAtId != kInvalidUniqueId) {
    CEntity* entity = mgr.ObjectById(mLookAtId);
    if (const CScriptActor* actor = TCastToConstPtr< CScriptActor >(entity)) {
      if (actor->IsPlayerActor() && actor->HasAnimation()) {
        const CAnimData* animData = actor->GetAnimationData();
        const rstl::ncrc_ptr< CAnimTreeNode >& root = animData->GetRootAnimationTree();
        if (root.GetPtr()) {
          const CModelData* modelData = actor->GetModelData();
          const CSegId leftEye = animData->GetLocatorSegId(rstl::string_l(leftEyeName));
          const CSegId rightEye = animData->GetLocatorSegId(rstl::string_l(rightEyeName));
          if (leftEye != CSegId::Invalid() && rightEye != CSegId::Invalid()) {
            const CCharAnimTime time =
                outOfEye ? CCharAnimTime::ZeroFlat() : root->GetSteadyStateAnimInfo().GetDuration();
            const CCharAnimTime* timePtr = outOfEye ? nullptr : &time;
            const CTransform4f leftLocal =
                modelData->GetScaledLocatorTransformDynamic(rstl::string_l(leftEyeName), timePtr);
            const CTransform4f leftWorld = actor->GetTransform() * leftLocal;
            const CTransform4f rightLocal =
                modelData->GetScaledLocatorTransformDynamic(rstl::string_l(rightEyeName), timePtr);
            const CTransform4f rightWorld = actor->GetTransform() * rightLocal;
            eyePos = (leftWorld.GetTranslation() + rightWorld.GetTranslation()) * 0.5f;
            q = CQuaternion::FromMatrix(actor->GetTransform());
          }
        }
      }
    }
  }

  CVector3f behindPos = eyePos;
  CVector3f behindDelta = q.Transform(
      CVector3f(0.f, -gpTweakPlayerRes->GetCinematicMoveOutofIntoPlayerDistance(), 0.f));
  if (!outOfEye) {
    behindPos += 1.f * behindDelta;
    behindDelta *= -1.f;
  }
  for (int i = 0; i < 2; ++i) {
    const int fromEnd = 2 - i;
    mViewPoints[outOfEye ? i : mViewPoints.size() - fromEnd] = behindPos;
    mViewOrientations[outOfEye ? i : mViewOrientations.size() - fromEnd] = q;
    mTargets[outOfEye ? i : mTargets.size() - fromEnd] = eyePos;
    behindPos += behindDelta;
  }
  mMoveIntoEyePos = eyePos;
}

bool CCinematicCamera::PickRandomActiveConnection(const rstl::vector< SConnection >& conns,
                                                  SConnection& randConn, CStateManager& mgr) {
  int count = 0;
  AUTO(it, conns.begin());
  for (; it != conns.end(); ++it) {
    if (it->mState == kSS_Arrived && it->mMsg == kSM_Next) {
      if (const CActor* const actor =
              TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetIdForScript(it->mObjId)))) {
        if (actor->GetActive()) {
          ++count;
        }
      }
    }
  }
  if (count == 0) {
    return false;
  }

  const int randIdx = mgr.Random()->Next() % count;
  int idx = 0;
  for (it = conns.begin(); it != conns.end(); ++it) {
    if (it->mState == kSS_Arrived && it->mMsg == kSM_Next) {
      if (const CActor* const actor =
              TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetIdForScript(it->mObjId)))) {
        if (actor->GetActive()) {
          if (randIdx == idx) {
            randConn = *it;
            break;
          }
          ++idx;
        }
      }
    }
  }
  return true;
}

CVector3f CCinematicCamera::GetInterpolatedSplinePoint(const rstl::vector< CVector3f >& points,
                                                       int& idxOut, float tin) const {
  const int count = points.size();
  if (count > 0) {
    const float cycleT = CCast::ToReal32(fmod(tin, mDuration));
    const float durPerPoint = mDuration / float(count - 1);
    const int idx = CCast::ToInt32(cycleT / durPerPoint);
    idxOut = idx;
    const float t = (cycleT - float(idx) * durPerPoint) / durPerPoint;
    if (count == 1) {
      return points[0];
    }
    if (count == 2) {
      return points[0] + (points[1] - points[0]) * t;
    }

    CVector3f a = CVector3f::Zero();
    if (idx > 0) {
      a = points[idx - 1];
    } else {
      a = points[0] - (points[1] - points[0]);
    }
    const CVector3f b = points[idx];
    CVector3f c = CVector3f::Zero();
    if (idx + 1 >= count) {
      c = points[count - 1] - (points[count - 2] - points[count - 1]);
    } else {
      c = points[idx + 1];
    }
    CVector3f d = CVector3f::Zero();
    if (idx + 2 >= count) {
      d = points[count - 1] - (points[count - 2] - points[count - 1]);
    } else {
      d = points[idx + 2];
    }
    return CMath::GetCatmullRomSplinePoint(a, b, c, d, t);
  }
  idxOut = 0;
  return CVector3f::Zero();
}

CQuaternion
CCinematicCamera::GetInterpolatedOrientation(const rstl::vector< CQuaternion >& rotations,
                                             float tin) const {
  const int count = rotations.size();
  if (count == 0) {
    return mOrigOrientation;
  }
  if (count == 1) {
    return rotations[0];
  }
  const float cycleT = CCast::ToReal32(fmod(tin, mDuration));
  const float durPerPoint = mDuration / static_cast< float >(count - 1);
  const int idx = CCast::ToInt32(cycleT / durPerPoint);
  const CQuaternion& second = rotations[idx + 1];
  const CQuaternion& first = rotations[idx];
  const float t = (cycleT - static_cast< float >(idx) * durPerPoint) / durPerPoint;
  return CQuaternion::Slerp(first, second, t);
}

float CCinematicCamera::GetInterpolatedHFov(const rstl::vector< float >& fovs, float tin) const {
  float result;
  const int count = fovs.size();
  if (count == 0) {
    result = mOrigFovy;
  } else if (count == 1) {
    result = fovs[0];
  } else {
    const float cycleT = CCast::ToReal32(fmod(tin, mDuration));
    const float durPerPoint = mDuration / float(count - 1);
    const int idx = CCast::ToInt32(cycleT / durPerPoint);
    result = fovs[idx];
    const float t = (cycleT - float(idx) * durPerPoint) / durPerPoint;
    result += (fovs[idx + 1] - result) * t;
  }
  return result;
}

float CCinematicCamera::GetMoveOutofIntoAlpha() const {
  const float startDist = 0.25f + GetNearClipDistance();
  const float endDist = 1.f + startDist;
  const float dist = (GetTranslation() - mMoveIntoEyePos).Magnitude();
  float alpha = 0.f;
  if (dist >= startDist && dist <= endDist) {
    alpha = (dist - startDist) / (endDist - startDist);
  } else if (dist > endDist) {
    alpha = 1.f;
  }
  return alpha;
}
