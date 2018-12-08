#include "CCinematicCamera.hpp"
#include "CStateManager.hpp"
#include "World/CPlayer.hpp"
#include "World/CScriptActor.hpp"
#include "TCastTo.hpp"
#include "World/CScriptCameraWaypoint.hpp"
#include "GameGlobalObjects.hpp"
#include "Character/CAnimTreeNode.hpp"

namespace urde {

CCinematicCamera::CCinematicCamera(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                                   const zeus::CTransform& xf, bool active, float shotDuration, float fovy, float znear,
                                   float zfar, float aspect, u32 flags)
: CGameCamera(uid, active, name, info, xf, fovy, znear, zfar, aspect, kInvalidUniqueId, flags & 0x20, 0)
, x1e8_duration(shotDuration)
, x1f0_origFovy(fovy)
, x1fc_origOrientation(zeus::CQuaternion(xf.basis))
, x21c_flags(flags) {
  x220_24_ = false;
}

void CCinematicCamera::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CCinematicCamera::ProcessInput(const CFinalInput&, CStateManager& mgr) {
  // Empty
}

void CCinematicCamera::Reset(const zeus::CTransform&, CStateManager& mgr) {
  // Empty
}

void CCinematicCamera::WasDeactivated(CStateManager& mgr) {
  mgr.GetCameraManager()->RemoveCinemaCamera(GetUniqueId(), mgr);
  mgr.GetPlayer().GetMorphBall()->LoadMorphBallModel(mgr);
  if (x21c_flags & 0x100)
    mgr.SetCinematicPause(false);
  x188_viewPoints.clear();
  x198_viewOrientations.clear();
  x1a8_viewPointArrivals.clear();
  x1b8_targets.clear();
  x1c8_targetArrivals.clear();
  x1d8_viewHFovs.clear();
}

zeus::CVector3f CCinematicCamera::GetInterpolatedSplinePoint(const std::vector<zeus::CVector3f>& points, int& idxOut,
                                                             float tin) const {
  if (points.size() > 0) {
    float cycleT = std::fmod(tin, x1e8_duration);
    float durPerPoint = x1e8_duration / float(points.size() - 1);
    idxOut = int(cycleT / durPerPoint);
    float t = (cycleT - idxOut * durPerPoint) / durPerPoint;

    if (points.size() == 1)
      return points.front();
    else if (points.size() == 2)
      return (points[1] - points[0]) * t + points[0];

    zeus::CVector3f ptA;
    if (idxOut > 0)
      ptA = points[idxOut - 1];
    else
      ptA = points[0] - (points[1] - points[0]);

    zeus::CVector3f ptB = points[idxOut];
    zeus::CVector3f ptC;
    if (idxOut + 1 >= points.size()) {
      const zeus::CVector3f& tmpA = points[points.size() - 1];
      const zeus::CVector3f& tmpB = points[points.size() - 2];
      ptC = tmpA - (tmpB - tmpA);
    } else {
      ptC = points[idxOut + 1];
    }

    zeus::CVector3f ptD;
    if (idxOut + 2 >= points.size()) {
      const zeus::CVector3f& tmpA = points[points.size() - 1];
      const zeus::CVector3f& tmpB = points[points.size() - 2];
      ptD = tmpA - (tmpB - tmpA);
    } else {
      ptD = points[idxOut + 2];
    }

    return zeus::getCatmullRomSplinePoint(ptA, ptB, ptC, ptD, t);
  }

  return {};
}

zeus::CQuaternion CCinematicCamera::GetInterpolatedOrientation(const std::vector<zeus::CQuaternion>& rotations,
                                                               float tin) const {
  if (rotations.size() == 0)
    return x1fc_origOrientation;
  else if (rotations.size() == 1)
    return rotations.front();

  float cycleT = std::fmod(tin, x1e8_duration);
  float durPerPoint = x1e8_duration / float(rotations.size() - 1);
  int idx = int(cycleT / durPerPoint);
  float t = (cycleT - idx * durPerPoint) / durPerPoint;
  return zeus::CQuaternion::slerp(rotations[idx], rotations[idx + 1], t);
}

float CCinematicCamera::GetInterpolatedHFov(const std::vector<float>& fovs, float tin) const {
  if (fovs.size() == 0)
    return x1f0_origFovy;
  else if (fovs.size() == 1)
    return fovs.front();

  float cycleT = std::fmod(tin, x1e8_duration);
  float durPerPoint = x1e8_duration / float(fovs.size() - 1);
  int idx = int(cycleT / durPerPoint);
  float t = (cycleT - idx * durPerPoint) / durPerPoint;
  return (fovs[idx + 1] - fovs[idx]) * t + fovs[idx];
}

float CCinematicCamera::GetMoveOutofIntoAlpha() const {
  float startDist = 0.25f + x160_znear;
  float endDist = 1.f * startDist;
  float deltaMag = (GetTranslation() - x210_moveIntoEyePos).magnitude();
  if (deltaMag >= startDist && deltaMag <= endDist)
    return (deltaMag - startDist) / (endDist - startDist);
  if (deltaMag > endDist)
    return 1.f;
  return 0.f;
}

void CCinematicCamera::DeactivateSelf(CStateManager& mgr) {
  SetActive(false);
  SendScriptMsgs(EScriptObjectState::Inactive, mgr, EScriptObjectMessage::None);
  WasDeactivated(mgr);
}

void CCinematicCamera::Think(float dt, CStateManager& mgr) {
  if (GetActive()) {
    zeus::CVector3f viewPoint = GetTranslation();
    if (x188_viewPoints.size() > 0) {
      int idx = 0;
      viewPoint = GetInterpolatedSplinePoint(x188_viewPoints, idx, x1ec_t);
      if (idx > x1f4_passedViewPoint) {
        x1f4_passedViewPoint = idx;
        SendArrivedMsg(x1a8_viewPointArrivals[x1f4_passedViewPoint], mgr);
      }
    }

    zeus::CQuaternion orientation = GetInterpolatedOrientation(x198_viewOrientations, x1ec_t);

    if ((x21c_flags & 0x1) == 0) {
      if (x1b8_targets.size() > 0) {
        int idx = 0;
        zeus::CVector3f target = GetInterpolatedSplinePoint(x1b8_targets, idx, x1ec_t);
        if (x1b8_targets.size() == 1) {
          if (TCastToConstPtr<CActor> act = mgr.GetObjectById(x1c8_targetArrivals.front()))
            target = act->GetTranslation();
          else
            x1ec_t = x1e8_duration;
        }
        if (idx > x1f8_passedTarget) {
          x1f8_passedTarget = idx;
          SendArrivedMsg(x1c8_targetArrivals[x1f8_passedTarget], mgr);
        }
        zeus::CVector3f upVec = orientation.transform(zeus::CVector3f::skUp);
        if ((target - viewPoint).toVec2f().magnitude() < 0.0011920929f)
          SetTranslation(target);
        else
          SetTransform(zeus::lookAt(viewPoint, target, upVec));
      } else {
        SetTransform(zeus::CTransform(orientation, viewPoint));
      }
    } else {
      zeus::CVector3f target = mgr.GetPlayer().GetTranslation();
      if (mgr.GetPlayer().GetMorphballTransitionState() == CPlayer::EPlayerMorphBallState::Morphed)
        target.z() += mgr.GetPlayer().GetMorphBall()->GetBallRadius();
      else
        target.z() += mgr.GetPlayer().GetEyeHeight();

      zeus::CVector3f upVec = orientation.transform(zeus::CVector3f::skUp);
      if ((target - viewPoint).toVec2f().magnitude() < 0.0011920929f)
        SetTranslation(target);
      else
        SetTransform(zeus::lookAt(viewPoint, target, upVec));
    }

    x15c_currentFov = GetInterpolatedHFov(x1d8_viewHFovs, x1ec_t) / x168_aspect;
    x170_24_perspDirty = true;

    if (x20c_lookAtId != kInvalidUniqueId)
      if (TCastToPtr<CScriptActor> act = mgr.ObjectById(x20c_lookAtId))
        if (act->IsPlayerActor())
          act->SetDrawFlags({5, 0, 3, zeus::CColor(1.f, GetMoveOutofIntoAlpha())});

    x1ec_t += dt;
    if (x1ec_t > x1e8_duration) {
      for (int i = x1f4_passedViewPoint + 1; i < x1a8_viewPointArrivals.size(); ++i)
        SendArrivedMsg(x1a8_viewPointArrivals[i], mgr);
      for (int i = x1f8_passedTarget + 1; i < x1c8_targetArrivals.size(); ++i)
        SendArrivedMsg(x1c8_targetArrivals[i], mgr);
      DeactivateSelf(mgr);
    }
  }
}

void CCinematicCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) {
  CGameCamera::AcceptScriptMsg(msg, uid, mgr);
  switch (msg) {
  case EScriptObjectMessage::InitializedInArea:
    if (x21c_flags & 0x4 || x21c_flags & 0x2) {
      for (const SConnection& conn : x20_conns) {
        TUniqueId id = mgr.GetIdForScript(conn.x8_objId);
        if (TCastToPtr<CScriptActor> act = mgr.ObjectById(id)) {
          if (act->IsPlayerActor()) {
            x20c_lookAtId = id;
            if (conn.x4_msg != EScriptObjectMessage::Deactivate && conn.x4_msg != EScriptObjectMessage::Reset)
              break;
          }
        }
      }
    }
    break;
  case EScriptObjectMessage::Activate:
    CalculateWaypoints(mgr);
    if ((x21c_flags & 1) == 0 && x220_24_ && x1b8_targets.empty())
      break;
    x1ec_t = 0.f;
    Think(0.f, mgr);
    mgr.GetCameraManager()->AddCinemaCamera(GetUniqueId(), mgr);
    x1f4_passedViewPoint = 0;
    if (x1a8_viewPointArrivals.size() > 0)
      SendArrivedMsg(x1a8_viewPointArrivals[x1f4_passedViewPoint], mgr);
    x1f8_passedTarget = 0;
    if (x1c8_targetArrivals.size() > 0)
      SendArrivedMsg(x1c8_targetArrivals[x1f8_passedTarget], mgr);
    if (x21c_flags & 0x100)
      mgr.SetCinematicPause(true);
    break;
  case EScriptObjectMessage::Deactivate:
    WasDeactivated(mgr);
    break;
  default:
    break;
  }
}

void CCinematicCamera::CalculateMoveOutofIntoEyePosition(bool outOfEye, CStateManager& mgr) {
  zeus::CQuaternion q(mgr.GetPlayer().GetTransform().basis);
  zeus::CVector3f eyePos = mgr.GetPlayer().GetEyePosition();
  if (x20c_lookAtId != kInvalidUniqueId) {
    if (TCastToConstPtr<CScriptActor> act = mgr.GetObjectById(x20c_lookAtId)) {
      if (act->IsPlayerActor()) {
        if (const CModelData* mData = act->GetModelData()) {
          if (const CAnimData* aData = mData->GetAnimationData()) {
            if (const CAnimTreeNode* root = aData->GetRootAnimationTree().get()) {
              CSegId lEye = aData->GetLocatorSegId("L_eye"sv);
              CSegId rEye = aData->GetLocatorSegId("R_eye"sv);
              if (lEye != 0xff && rEye != 0xff) {
                CCharAnimTime time = outOfEye ? CCharAnimTime(0.f) : root->VGetSteadyStateAnimInfo().GetDuration();
                CCharAnimTime* pTime = outOfEye ? nullptr : &time;
                eyePos = ((act->GetTransform() * mData->GetScaledLocatorTransformDynamic("L_eye"sv, pTime)).origin +
                          (act->GetTransform() * mData->GetScaledLocatorTransformDynamic("R_eye"sv, pTime)).origin) *
                         0.5f;
                q = zeus::CQuaternion(act->GetTransform().basis);
              }
            }
          }
        }
      }
    }
  }

  zeus::CVector3f behindPos = eyePos;
  zeus::CVector3f behindDelta = q.transform({0.f, -g_tweakPlayerRes->xf0_cinematicMoveOutofIntoPlayerDistance, 0.f});
  if (!outOfEye) {
    behindPos += behindDelta;
    behindDelta = -behindDelta;
  }

  for (int i = 0; i < 2; ++i) {
    x188_viewPoints[outOfEye ? i : x188_viewPoints.size() - (2 - i)] = behindPos;
    x198_viewOrientations[outOfEye ? i : x198_viewOrientations.size() - (2 - i)] = q;
    x1b8_targets[outOfEye ? i : x1b8_targets.size() - (2 - i)] = eyePos;
    behindPos += behindDelta;
  }

  x210_moveIntoEyePos = eyePos;
}

void CCinematicCamera::GenerateMoveOutofIntoPoints(bool outOfEye, CStateManager& mgr) {
  zeus::CQuaternion q(mgr.GetPlayer().GetTransform().basis);
  zeus::CVector3f eyePos = mgr.GetPlayer().GetEyePosition();
  zeus::CVector3f behindDelta = q.transform({0.f, -g_tweakPlayerRes->xf0_cinematicMoveOutofIntoPlayerDistance, 0.f});
  zeus::CVector3f behindPos = eyePos;
  if (!outOfEye) {
    behindPos += behindDelta;
    behindDelta = -behindDelta;
  }
  for (int i = 0; i < 2; ++i) {
    x188_viewPoints.push_back(behindPos);
    x198_viewOrientations.push_back(q);
    x1a8_viewPointArrivals.push_back(mgr.GetPlayer().GetUniqueId());
    x1b8_targets.push_back(eyePos);
    x1c8_targetArrivals.push_back(kInvalidUniqueId);
    behindPos += behindDelta;
  }
  CalculateMoveOutofIntoEyePosition(outOfEye, mgr);
}

bool CCinematicCamera::PickRandomActiveConnection(const std::vector<SConnection>& conns, SConnection& randConn,
                                                  CStateManager& mgr) {
  int count = 0;
  for (const SConnection& conn : conns)
    if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Next)
      if (TCastToConstPtr<CActor> act = mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId)))
        if (act->GetActive())
          ++count;

  if (!count)
    return false;

  int randIdx = mgr.GetActiveRandom()->Next() % count;
  int idx = 0;
  for (const SConnection& conn : conns)
    if (conn.x0_state == EScriptObjectState::Arrived && conn.x4_msg == EScriptObjectMessage::Next)
      if (TCastToConstPtr<CActor> act = mgr.GetObjectById(mgr.GetIdForScript(conn.x8_objId)))
        if (act->GetActive()) {
          if (randIdx == idx) {
            randConn = conn;
            break;
          }
          ++idx;
        }

  return true;
}

void CCinematicCamera::CalculateWaypoints(CStateManager& mgr) {
  const SConnection* firstVP = nullptr;
  const SConnection* firstTarget = nullptr;
  for (const SConnection& conn : x20_conns) {
    if (conn.x0_state == EScriptObjectState::CameraPath && conn.x4_msg == EScriptObjectMessage::Activate)
      firstVP = &conn;
    else if (conn.x0_state == EScriptObjectState::CameraTarget && conn.x4_msg == EScriptObjectMessage::Activate)
      firstTarget = &conn;
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

  if ((x21c_flags & 0x2) != 0 && (x21c_flags & 0x200) == 0)
    GenerateMoveOutofIntoPoints(true, mgr);

  if (firstVP) {
    TCastToConstPtr<CActor> wp = mgr.GetObjectById(mgr.GetIdForScript(firstVP->x8_objId));
    while (wp) {
      x188_viewPoints.push_back(wp->GetTranslation());
      x198_viewOrientations.emplace_back(wp->GetTransform().basis);
      if (TCastToConstPtr<CScriptCameraWaypoint> cwp = wp.GetPtr())
        x1d8_viewHFovs.push_back(cwp->GetHFov());
      auto search = std::find_if(x1a8_viewPointArrivals.begin(), x1a8_viewPointArrivals.end(),
                                 [&](TUniqueId id) { return id == wp->GetUniqueId(); });
      if (search == x1a8_viewPointArrivals.end()) {
        x1a8_viewPointArrivals.push_back(wp->GetUniqueId());
        SConnection randConn;
        if (PickRandomActiveConnection(wp->GetConnectionList(), randConn, mgr))
          wp = mgr.GetObjectById(mgr.GetIdForScript(randConn.x8_objId));
        else
          break;
      } else
        break;
    }
  }

  if (firstTarget) {
    TCastToConstPtr<CActor> tgt = mgr.GetObjectById(mgr.GetIdForScript(firstTarget->x8_objId));
    while (tgt) {
      x1b8_targets.push_back(tgt->GetTranslation());
      auto search = std::find_if(x1c8_targetArrivals.begin(), x1c8_targetArrivals.end(),
                                 [&](TUniqueId id) { return id == tgt->GetUniqueId(); });
      if (search == x1c8_targetArrivals.end()) {
        x1c8_targetArrivals.push_back(tgt->GetUniqueId());
        SConnection randConn;
        if (PickRandomActiveConnection(tgt->GetConnectionList(), randConn, mgr))
          tgt = mgr.GetObjectById(mgr.GetIdForScript(randConn.x8_objId));
        else
          break;
      } else
        break;
    }
  }

  if ((x21c_flags & 0x4) != 0 && (x21c_flags & 0x200) == 0)
    GenerateMoveOutofIntoPoints(false, mgr);
}

void CCinematicCamera::SendArrivedMsg(TUniqueId reciever, CStateManager& mgr) {
  mgr.SendScriptMsgAlways(reciever, GetUniqueId(), EScriptObjectMessage::Arrived);
}
} // namespace urde
