#include "Camera/CCameraSpline.hpp"
#include "CStateManager.hpp"
#include "World/CScriptCameraWaypoint.hpp"
#include "TCastTo.hpp" // Generated file, do not modify include path

namespace urde {
CCameraSpline::CCameraSpline(bool closedLoop) : x48_closedLoop(closedLoop) {}

void CCameraSpline::CalculateKnots(TUniqueId cameraId, const std::vector<SConnection>& connections,
                                   CStateManager& mgr) {
  const SConnection* lastConn = nullptr;

  for (const SConnection& conn : connections) {
    if (conn.x0_state == EScriptObjectState::CameraPath && conn.x4_msg == EScriptObjectMessage::Follow)
      lastConn = &conn;
  }

  if (lastConn) {
    TCastToPtr<CScriptCameraWaypoint> waypoint = mgr.ObjectById(mgr.GetIdForScript(lastConn->x8_objId));
    x14_wpTracker.clear();
    x14_wpTracker.reserve(4);
    while (waypoint) {
      auto search = std::find_if(x14_wpTracker.begin(), x14_wpTracker.end(),
                                 [&waypoint](const auto& a) { return a == waypoint->GetUniqueId(); });
      if (search == x14_wpTracker.end()) {
        x14_wpTracker.push_back(waypoint->GetUniqueId());
        waypoint = mgr.ObjectById(waypoint->GetRandomNextWaypointId(mgr));
      }
    }
    Reset(x14_wpTracker.size());
    x14_wpTracker.clear();

    waypoint = mgr.ObjectById(mgr.GetIdForScript(lastConn->x8_objId));
    while (waypoint) {
      auto search = std::find_if(x14_wpTracker.begin(), x14_wpTracker.end(),
                                 [&waypoint](const auto& a) { return a == waypoint->GetUniqueId(); });
      if (search == x14_wpTracker.end()) {
        x14_wpTracker.push_back(waypoint->GetUniqueId());
        AddKnot(waypoint->GetTranslation(), waypoint->GetTransform().basis[1]);
        waypoint = mgr.ObjectById(waypoint->GetRandomNextWaypointId(mgr));
      }
    }
  }
}

void CCameraSpline::Initialize(TUniqueId camId, const std::vector<SConnection>& connections, CStateManager& mgr) {
  CalculateKnots(camId, connections, mgr);
  x44_length = CalculateSplineLength();
}

void CCameraSpline::Reset(int size) {
  x4_positions.clear();
  x24_t.clear();
  x34_directions.clear();
  if (size != 0) {
    x4_positions.reserve(size);
    x24_t.reserve(size);
    x34_directions.reserve(size);
  }
}

void CCameraSpline::AddKnot(const zeus::CVector3f& pos, const zeus::CVector3f& dir) {
  x4_positions.push_back(pos);
  x34_directions.push_back(dir);
}

void CCameraSpline::SetKnotPosition(int idx, const zeus::CVector3f& pos) {
  if (idx >= x4_positions.size())
    return;
  x4_positions[idx] = pos;
}

const zeus::CVector3f& CCameraSpline::GetKnotPosition(int idx) const {
  if (idx >= x4_positions.size())
    return zeus::skZero3f;
  return x4_positions[idx];
}

float CCameraSpline::GetKnotT(int idx) const {
  if (idx >= x4_positions.size())
    return 0.f;
  return x24_t[idx];
}

float CCameraSpline::CalculateSplineLength() {
  float ret = 0.f;
  x24_t.clear();
  if (x4_positions.size() > 0) {
    zeus::CVector3f prevPoint = x4_positions[0];
    float tDiv = 1.f / float(x4_positions.size() - 1);
    for (size_t i = 0; i < x4_positions.size(); ++i) {
      float subT = 0.f;
      float baseT = i * tDiv;
      x24_t.push_back(ret);
      while (subT <= tDiv) {
        subT += tDiv * 0.03125f;
        zeus::CVector3f nextPoint = GetInterpolatedSplinePointByTime(baseT + subT, 1.f);
        zeus::CVector3f delta = nextPoint - prevPoint;
        if (delta.canBeNormalized()) {
          prevPoint = nextPoint;
          ret += delta.magnitude();
        }
      }
    }

    x24_t.push_back(ret);
    if (x48_closedLoop) {
      zeus::CVector3f delta = x4_positions[0] - x4_positions[x4_positions.size() - 1];
      if (delta.canBeNormalized())
        ret += delta.magnitude();
    }

    return ret;
  }
  return 0.f;
}

bool CCameraSpline::GetSurroundingPoints(int idx, rstl::reserved_vector<zeus::CVector3f, 4>& positions,
                                         rstl::reserved_vector<zeus::CVector3f, 4>& directions) const {
  if (x4_positions.size() <= 3 || idx < 0 || idx >= x4_positions.size())
    return false;

  if (idx > 0) {
    positions.push_back(x4_positions[idx - 1]);
    directions.push_back(x34_directions[idx - 1]);
  } else if (x48_closedLoop) {
    positions.push_back(x4_positions[x4_positions.size() - 1]);
    directions.push_back(x34_directions[x4_positions.size() - 1]);
  } else {
    positions.push_back(x4_positions[0] - (x4_positions[1] - x4_positions[0]));
    directions.push_back(x34_directions[0]);
  }

  positions.push_back(x4_positions[idx]);
  directions.push_back(x34_directions[idx]);

  if (idx + 1 >= x4_positions.size()) {
    if (x48_closedLoop) {
      positions.push_back(x4_positions[idx - x4_positions.size()]);
      directions.push_back(x34_directions[idx - x4_positions.size()]);
    } else {
      positions.push_back(x4_positions[x4_positions.size() - 1] -
                          (x4_positions[x4_positions.size() - 2] - x4_positions[x4_positions.size() - 1]));
      directions.push_back(x34_directions[x4_positions.size() - 1]);
    }
  } else {
    positions.push_back(x4_positions[idx + 1]);
    directions.push_back(x34_directions[idx + 1]);
  }

  if (idx + 2 >= x4_positions.size()) {
    if (x48_closedLoop) {
      positions.push_back(x4_positions[idx + 2 - x4_positions.size()]);
      directions.push_back(x34_directions[idx + 2 - x4_positions.size()]);
    } else {
      positions.push_back(x4_positions[x4_positions.size() - 1] -
                          (x4_positions[x4_positions.size() - 2] - x4_positions[x4_positions.size() - 1]));
      directions.push_back(x34_directions[x4_positions.size() - 1]);
    }
  } else {
    positions.push_back(x4_positions[idx + 2]);
    directions.push_back(x34_directions[idx + 2]);
  }

  return true;
}

zeus::CTransform CCameraSpline::GetInterpolatedSplinePointByLength(float pos) const {
  if (x4_positions.empty())
    return zeus::CTransform();

  size_t baseIdx = 0;
  size_t i;
  for (i = 1; i < x4_positions.size(); ++i) {
    if (x24_t[i] > pos) {
      baseIdx = i - 1;
      break;
    }
  }

  if (i == x4_positions.size())
    baseIdx = i - 1;

  if (pos < 0.f)
    baseIdx = 0;

  if (pos >= x44_length) {
    if (x48_closedLoop) {
      pos -= x44_length;
      baseIdx = 0;
    } else {
      baseIdx = x4_positions.size() - 2;
      pos = x44_length;
    }
  }

  float range;
  if (baseIdx == x4_positions.size() - 1) {
    if (x48_closedLoop)
      range = x44_length - x24_t[baseIdx];
    else
      range = x44_length - x24_t[x4_positions.size() - 2];
  } else {
    range = x24_t[baseIdx + 1] - x24_t[baseIdx];
  }

  float t = zeus::clamp(0.f, (pos - x24_t[baseIdx]) / range, 1.f);

  rstl::reserved_vector<zeus::CVector3f, 4> positions;
  rstl::reserved_vector<zeus::CVector3f, 4> directions;
  if (GetSurroundingPoints(baseIdx, positions, directions)) {
    float f1 = zeus::clamp(-1.f, directions[1].dot(directions[2]), 1.f);
    if (f1 >= 1.f) {
      zeus::CTransform ret = zeus::lookAt(zeus::skZero3f, directions[2]);
      ret.origin = zeus::getCatmullRomSplinePoint(positions[0], positions[1], positions[2], positions[3], t);
      return ret;
    } else {
      zeus::CTransform ret = zeus::lookAt(
          zeus::skZero3f,
          zeus::CQuaternion::lookAt(directions[1], directions[2], std::acos(f1) * t).transform(directions[1]));
      ret.origin = zeus::getCatmullRomSplinePoint(positions[0], positions[1], positions[2], positions[3], t);
      return ret;
    }
  }

  return zeus::CTransform();
}

zeus::CVector3f CCameraSpline::GetInterpolatedSplinePointByTime(float time, float range) const {
  if (x4_positions.empty())
    return {};

  rstl::reserved_vector<zeus::CVector3f, 4> positions;
  rstl::reserved_vector<zeus::CVector3f, 4> directions;
  float rangeFac = range / float(x4_positions.size() - 1);
  int baseIdx = std::min(int(x4_positions.size() - 1), int(time / rangeFac));
  if (GetSurroundingPoints(baseIdx, positions, directions))
    return zeus::getCatmullRomSplinePoint(positions[0], positions[1], positions[2], positions[3],
                                          (time - float(baseIdx) * rangeFac) / rangeFac);

  return {};
}

float CCameraSpline::FindClosestLengthOnSpline(float time, const zeus::CVector3f& p) const {
  float ret = -1.f;
  float minLenDelta = 10000.f;
  float minMag = 10000.f;

  size_t iterations = x4_positions.size() - 1;
  if (x48_closedLoop)
    iterations += 1;

  for (size_t i = 0; i < iterations; ++i) {
    const zeus::CVector3f& thisPos = x4_positions[i];
    const zeus::CVector3f* nextPos;
    if (!x48_closedLoop) {
      nextPos = &x4_positions[i + 1];
    } else {
      if (i == x4_positions.size() - 1)
        nextPos = &x4_positions[0];
      else
        nextPos = &x4_positions[i + 1];
    }

    zeus::CVector3f delta = *nextPos - thisPos;
    zeus::CVector3f nextDelta;
    zeus::CVector3f revDelta = thisPos - *nextPos;
    zeus::CVector3f nextRevDelta;

    if (i != 0) {
      nextDelta = delta + thisPos - x4_positions[i - 1];
    } else {
      zeus::CVector3f extrap = x4_positions[0] - x4_positions[1] + x4_positions[0];
      if (x48_closedLoop)
        extrap = x4_positions.back();
      nextDelta = delta + thisPos - extrap;
    }
    nextDelta.normalize();

    if (i < x4_positions.size() - 2) {
      nextRevDelta = revDelta + *nextPos - x4_positions[i + 2];
    } else {
      zeus::CVector3f extrap;
      if (x48_closedLoop) {
        if (i == iterations - 1)
          extrap = x4_positions[1];
        else
          extrap = x4_positions[0];
      } else {
        extrap = x4_positions[i + 1] - x4_positions[i] + x4_positions[i + 1];
      }
      nextRevDelta = revDelta + *nextPos - extrap;
    }
    nextRevDelta.normalize();

    nextDelta.normalize();
    nextRevDelta.normalize();
    zeus::CVector3f ptToPlayer = p - thisPos;
    float proj = ptToPlayer.dot(nextDelta) / nextDelta.dot(delta.normalized());
    zeus::CVector3f nextPtToPlayer = p - *nextPos;
    float nextProj = nextPtToPlayer.dot(nextRevDelta) / nextRevDelta.dot(revDelta.normalized());
    float t = proj / (proj + nextProj);

    if (!x48_closedLoop) {
      if (i == 0 && t < 0.f)
        t = 0.f;
      if (i == x4_positions.size() - 2 && t > 1.f)
        t = 1.f;
    }

    if (t >= 0.f && t <= 1.f) {
      float tLen;
      if (i == x4_positions.size() - 1)
        tLen = x44_length - x24_t[i];
      else
        tLen = x24_t[i + 1] - x24_t[i];

      float lenT = t * tLen + x24_t[i];
      zeus::CVector3f pointDelta = p - GetInterpolatedSplinePointByLength(lenT).origin;
      float mag = 0.f;
      if (pointDelta.canBeNormalized())
        mag = pointDelta.magnitude();
      float lenDelta = std::fabs(lenT - time);
      if (x48_closedLoop && lenDelta > x44_length - lenDelta)
        lenDelta = x44_length - lenDelta;
      if (zeus::close_enough(std::fabs(mag - minMag), 0.f)) {
        if (lenDelta < minLenDelta) {
          ret = lenT;
          minLenDelta = lenDelta;
        }
      } else {
        if (mag < minMag) {
          ret = lenT;
          minLenDelta = lenDelta;
          minMag = mag;
        }
      }
    }
  }

  return std::max(ret, 0.f);
}

float CCameraSpline::ValidateLength(float t) const {
  if (x48_closedLoop) {
    while (t >= x44_length)
      t -= x44_length;
    while (t < 0.f)
      t += x44_length;
    return t;
  } else {
    return zeus::clamp(0.f, t, x44_length);
  }
}

float CCameraSpline::ClampLength(const zeus::CVector3f& pos, bool collide, const CMaterialFilter& filter,
                                 const CStateManager& mgr) const {
  if (x4_positions.empty())
    return 0.f;

  if (x48_closedLoop)
    return 0.f;

  zeus::CVector3f deltaA = pos - x4_positions.front();
  zeus::CVector3f deltaB = pos - x4_positions.back();
  float magA = deltaA.magnitude();
  float magB = deltaB.magnitude();
  if (!deltaA.canBeNormalized())
    return 0.f;
  if (!deltaB.canBeNormalized())
    return x44_length;

  if (collide) {
    bool collideA = mgr.RayStaticIntersection(x4_positions.front(), deltaA.normalized(), magA, filter).IsValid();
    bool collideB = mgr.RayStaticIntersection(x4_positions.back(), deltaB.normalized(), magB, filter).IsValid();
    if (collideA)
      return x44_length;
    if (collideB)
      return 0.f;
  }

  if (magA < magB)
    return 0.f;
  else
    return x44_length;
}

} // namespace urde
