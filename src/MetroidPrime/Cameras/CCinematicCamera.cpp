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
, x1e8_duration(shotDuration)
, x1ec_t(0.f)
, x1f0_origFovy(fovy)
, x1f4_passedViewPoint(0)
, x1f8_passedTarget(0)
, x1fc_origOrientation(CQuaternion::FromMatrix(xf))
, x20c_lookAtId(kInvalidUniqueId)
, x210_moveIntoEyePos(CVector3f::Zero())
, x21c_flags(flags)
, x220_24_(false) {}

CCinematicCamera::~CCinematicCamera() {}

void CCinematicCamera::Reset(const CTransform4f& xf, CStateManager& mgr) {}

void CCinematicCamera::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    CVector3f viewPoint = GetTranslation();
    if (x188_viewPoints.size() > 0) {
      int idx = 0;
      viewPoint = GetInterpolatedSplinePoint(x188_viewPoints, idx, x1ec_t);
      if (idx > x1f4_passedViewPoint) {
        x1f4_passedViewPoint = idx;
        SendArrivedMsg(x1a8_viewPointArrivals[x1f4_passedViewPoint], mgr);
      }
    }
    const CQuaternion orientation = GetInterpolatedOrientation(x198_viewOrientations, x1ec_t);
    if ((x21c_flags & 1) == 0) {
      if (x1b8_targets.size() > 0) {
        int idx = 0;
        CVector3f target = GetInterpolatedSplinePoint(x1b8_targets, idx, x1ec_t);
        if (x1b8_targets.size() == 1) {
          if (const CActor* actor =
                  TCastToConstPtr< CActor >(mgr.GetObjectById(x1c8_targetArrivals[0]))) {
            target = actor->GetTranslation();
          } else {
            x1ec_t = x1e8_duration;
          }
        }
        if (idx > x1f8_passedTarget) {
          x1f8_passedTarget = idx;
          SendArrivedMsg(x1c8_targetArrivals[x1f8_passedTarget], mgr);
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
    SetFov(GetInterpolatedHFov(x1d8_viewHFovs, x1ec_t) / GetAspectRatio());
    if (x20c_lookAtId != kInvalidUniqueId) {
      if (CScriptActor* actor = TCastToPtr< CScriptActor >(mgr.ObjectById(x20c_lookAtId))) {
        if (actor->IsPlayerActor()) {
          const float alpha = GetMoveOutofIntoAlpha();
          actor->SetModelFlags(CModelFlags::AlphaBlended(alpha));
        }
      }
    }
    x1ec_t += dt;
    if (x1ec_t > x1e8_duration) {
      for (int i = x1f4_passedViewPoint + 1; i < x1a8_viewPointArrivals.size(); ++i) {
        SendArrivedMsg(x1a8_viewPointArrivals[i], mgr);
      }
      for (int i = x1f8_passedTarget + 1; i < x1c8_targetArrivals.size(); ++i) {
        SendArrivedMsg(x1c8_targetArrivals[i], mgr);
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
    if ((x21c_flags & 4) != 0 || (x21c_flags & 2) != 0) {
      for (AUTO(it, GetConnectionList().begin()); it != GetConnectionList().end(); ++it) {
        const TUniqueId id = mgr.GetIdForScript(it->x8_objId);
        CEntity* entity = mgr.ObjectById(id);
        if (const CScriptActor* actor = TCastToConstPtr< CScriptActor >(entity)) {
          if (actor->IsPlayerActor()) {
            x20c_lookAtId = id;
            if (it->x4_msg != kSM_Deactivate && it->x4_msg != kSM_Reset) {
              break;
            }
          }
        }
      }
    }
    break;
  case kSM_Activate:
    CalculateWaypoints(mgr);
    if ((x21c_flags & 1) == 0 && x220_24_ && x1b8_targets.size() == 0) {
      break;
    }
    x1ec_t = 0.f;
    Think(0.f, mgr);
    mgr.CameraManager()->AddCinemaCamera(GetUniqueId(), mgr);
    x1f4_passedViewPoint = 0;
    if (x1a8_viewPointArrivals.size() > 0) {
      SendArrivedMsg(x1a8_viewPointArrivals[x1f4_passedViewPoint], mgr);
    }
    x1f8_passedTarget = 0;
    if (x1c8_targetArrivals.size() > 0) {
      SendArrivedMsg(x1c8_targetArrivals[x1f8_passedTarget], mgr);
    }
    if ((x21c_flags & 0x100) != 0) {
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

  if ((x21c_flags & 0x100) != 0) {
    mgr.SetCinematicPause(false);
  }

  x188_viewPoints = rstl::vector< CVector3f >();
  x198_viewOrientations = rstl::vector< CQuaternion >();
  x1a8_viewPointArrivals = rstl::vector< TUniqueId >();
  x1b8_targets = rstl::vector< CVector3f >();
  x1c8_targetArrivals = rstl::vector< TUniqueId >();
  x1d8_viewHFovs = rstl::vector< float >();
}

void CCinematicCamera::CalculateWaypoints(CStateManager& mgr) {
  const SConnection* firstVP = nullptr;
  const SConnection* firstTarget = nullptr;

  rstl::vector< SConnection >::const_iterator iter = GetConnectionList().begin();

  for (; iter != GetConnectionList().end(); ++iter) {
    if (iter->x0_state == kSS_CameraPath && iter->x4_msg == kSM_Activate) {
      firstVP = &*iter;
    } else if (iter->x0_state == kSS_CameraTarget && iter->x4_msg == kSM_Activate) {
      firstTarget = &*iter;
    }
  }

  x188_viewPoints.clear();
  x188_viewPoints.reserve(3);
  x198_viewOrientations.clear();
  x198_viewOrientations.reserve(3);
  x1a8_viewPointArrivals.clear();
  x1a8_viewPointArrivals.reserve(3);
  x1b8_targets.clear();
  x1b8_targets.reserve(3);
  x1c8_targetArrivals.clear();
  x1c8_targetArrivals.reserve(3);
  x1d8_viewHFovs.clear();
  x1d8_viewHFovs.reserve(3);

  x220_24_ = false;

  if ((x21c_flags & 0x2) != 0 && (x21c_flags & 0x200) == 0) {
    GenerateMoveOutofIntoPoints(true, mgr);
  }

  if (firstVP) {
    const CActor* wp =
        TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetIdForScript(firstVP->x8_objId)));

    while (wp) {
      x188_viewPoints.reserve(x188_viewPoints.size() + 1);
      x188_viewPoints.push_back(wp->GetTranslation());
      x198_viewOrientations.reserve(x198_viewOrientations.size() + 1);
      x198_viewOrientations.push_back(wp->GetRotation());

      if (const CScriptCameraWaypoint* cwp = TCastToConstPtr< CScriptCameraWaypoint >(wp)) {
        x1d8_viewHFovs.reserve(x1d8_viewHFovs.size() + 1);
        x1d8_viewHFovs.push_back(cwp->GetHFov());
      }

      if (rstl::find(x1a8_viewPointArrivals.begin(), x1a8_viewPointArrivals.end(),
                     wp->GetUniqueId()) == x1a8_viewPointArrivals.end()) {
        x1a8_viewPointArrivals.reserve(x1a8_viewPointArrivals.size() + 1);
        x1a8_viewPointArrivals.push_back(wp->GetUniqueId());
        SConnection randConn(kSS_Active, kSM_UNKM0, kInvalidEditorId);
        if (PickRandomActiveConnection(wp->GetConnectionList(), randConn, mgr)) {
          wp = TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetIdForScript(randConn.x8_objId)));
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
        TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetIdForScript(firstTarget->x8_objId)));
    while (tgt) {
      x1b8_targets.reserve(x1b8_targets.size() + 1);
      x1b8_targets.push_back(tgt->GetTranslation());
      if (rstl::find(x1c8_targetArrivals.begin(), x1c8_targetArrivals.end(), tgt->GetUniqueId()) ==
          x1c8_targetArrivals.end()) {
        x1c8_targetArrivals.reserve(x1c8_targetArrivals.size() + 1);
        x1c8_targetArrivals.push_back(tgt->GetUniqueId());
        SConnection randConn(kSS_Active, kSM_UNKM0, kInvalidEditorId);
        if (PickRandomActiveConnection(tgt->GetConnectionList(), randConn, mgr)) {
          tgt = TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetIdForScript(randConn.x8_objId)));
        } else {
          break;
        }
      } else {
        break;
      }
    }
  }

  if ((x21c_flags & 0x4) != 0 && (x21c_flags & 0x200) == 0) {
    GenerateMoveOutofIntoPoints(false, mgr);
  }
}

void CCinematicCamera::GenerateMoveOutofIntoPoints(bool outOfEye, CStateManager& mgr) {
  const float distance = gpTweakPlayerRes->GetCinematicMoveOutofIntoPlayerDistance();
  const CPlayer* player = mgr.GetPlayer();
  x188_viewPoints.reserve(x188_viewPoints.size() + 2);
  x198_viewOrientations.reserve(x198_viewOrientations.size() + 2);
  x1a8_viewPointArrivals.reserve(x1a8_viewPointArrivals.size() + 2);
  x1b8_targets.reserve(x1b8_targets.size() + 2);
  x1c8_targetArrivals.reserve(x1c8_targetArrivals.size() + 2);

  const CQuaternion q = CQuaternion::FromMatrix(player->GetTransform());
  const CVector3f eyePos = player->GetEyePosition();
  CVector3f behindPos = eyePos;
  CVector3f behindDelta = q.Transform(CVector3f(0.f, -distance, 0.f));
  if (!outOfEye) {
    behindPos += 1.f * behindDelta;
    behindDelta *= -1.f;
  }
  for (int i = 0; i < 2; ++i) {
    x188_viewPoints.push_back(behindPos);
    x198_viewOrientations.push_back(q);
    x1a8_viewPointArrivals.push_back(player->GetUniqueId());
    x1b8_targets.push_back(eyePos);
    x1c8_targetArrivals.push_back(kInvalidUniqueId);
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
  if (x20c_lookAtId != kInvalidUniqueId) {
    CEntity* entity = mgr.ObjectById(x20c_lookAtId);
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
    x188_viewPoints[outOfEye ? i : x188_viewPoints.size() - fromEnd] = behindPos;
    x198_viewOrientations[outOfEye ? i : x198_viewOrientations.size() - fromEnd] = q;
    x1b8_targets[outOfEye ? i : x1b8_targets.size() - fromEnd] = eyePos;
    behindPos += behindDelta;
  }
  x210_moveIntoEyePos = eyePos;
}

bool CCinematicCamera::PickRandomActiveConnection(const rstl::vector< SConnection >& conns,
                                                  SConnection& randConn, CStateManager& mgr) {
  int count = 0;
  AUTO(it, conns.begin());
  for (; it != conns.end(); ++it) {
    if (it->x0_state == kSS_Arrived && it->x4_msg == kSM_Next) {
      if (const CActor* const actor =
              TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetIdForScript(it->x8_objId)))) {
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
    if (it->x0_state == kSS_Arrived && it->x4_msg == kSM_Next) {
      if (const CActor* const actor =
              TCastToConstPtr< CActor >(mgr.GetObjectById(mgr.GetIdForScript(it->x8_objId)))) {
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
    const float cycleT = CCast::ToReal32(fmod(tin, x1e8_duration));
    const float durPerPoint = x1e8_duration / float(count - 1);
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
    return x1fc_origOrientation;
  }
  if (count == 1) {
    return rotations[0];
  }
  const float cycleT = CCast::ToReal32(fmod(tin, x1e8_duration));
  const float durPerPoint = x1e8_duration / static_cast< float >(count - 1);
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
    result = x1f0_origFovy;
  } else if (count == 1) {
    result = fovs[0];
  } else {
    const float cycleT = CCast::ToReal32(fmod(tin, x1e8_duration));
    const float durPerPoint = x1e8_duration / float(count - 1);
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
  const float dist = (GetTranslation() - x210_moveIntoEyePos).Magnitude();
  float alpha = 0.f;
  if (dist >= startDist && dist <= endDist) {
    alpha = (dist - startDist) / (endDist - startDist);
  } else if (dist > endDist) {
    alpha = 1.f;
  }
  return alpha;
}
