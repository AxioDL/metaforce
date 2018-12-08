#include "CPathFindSearch.hpp"

namespace urde {

bool CPathFindSearch::SegmentOver(const zeus::CVector3f& p1) const {
  if (x4_waypoints.size() > 1 && xc8_curWaypoint < x4_waypoints.size() - 1) {
    const zeus::CVector3f& wp0 = x4_waypoints[xc8_curWaypoint];
    const zeus::CVector3f& wp1 = x4_waypoints[xc8_curWaypoint + 1];
    const zeus::CVector3f& wp2 = x4_waypoints[std::min(u32(x4_waypoints.size()) - 1, xc8_curWaypoint + 2)];
    return (p1 - wp1).dot(wp2 - wp0) >= 0.f;
  }
  return true;
}

void CPathFindSearch::GetSplinePoint(zeus::CVector3f& pOut, const zeus::CVector3f& p1, u32 wpIdx) const {
  if (x4_waypoints.size() > 1 && wpIdx < x4_waypoints.size() - 1) {
    zeus::CVector3f a = (wpIdx == 0) ? x4_waypoints[0] * 2.f - x4_waypoints[1] : x4_waypoints[wpIdx - 1];
    const zeus::CVector3f& b = x4_waypoints[wpIdx];
    const zeus::CVector3f& c = x4_waypoints[wpIdx + 1];
    zeus::CVector3f d = (wpIdx + 2 >= x4_waypoints.size())
                            ? x4_waypoints[x4_waypoints.size() - 1] * 2.f - x4_waypoints[x4_waypoints.size() - 2]
                            : x4_waypoints[wpIdx + 2];
    zeus::CVector3f delta = c - b;
    if (delta.isMagnitudeSafe())
      pOut = zeus::getCatmullRomSplinePoint(a, b, c, d, (p1 - b).dot(delta) / delta.magSquared());
    else
      pOut = b;
  }
}

void CPathFindSearch::GetSplinePoint(zeus::CVector3f& pOut, const zeus::CVector3f& p1) const {
  GetSplinePoint(pOut, p1, xc8_curWaypoint);
}

void CPathFindSearch::GetSplinePointWithLookahead(zeus::CVector3f& pOut, const zeus::CVector3f& p1, u32 wpIdx,
                                                  float lookahead) const {
  if (x4_waypoints.size() > 1 && wpIdx < x4_waypoints.size() - 1) {
    const zeus::CVector3f& wp0 = x4_waypoints[wpIdx];
    const zeus::CVector3f& wp1 = x4_waypoints[wpIdx + 1];
    zeus::CVector3f delta = wp1 - wp0;
    if (delta.isMagnitudeSafe()) {
      float deltaMag = delta.magnitude();
      delta = delta * (1.f / deltaMag);
      float bToPtProj = (p1 - wp0).dot(delta);
      if (bToPtProj + lookahead <= deltaMag)
        GetSplinePoint(pOut, delta * lookahead + p1, wpIdx);
      else if (wpIdx < x4_waypoints.size() - 2)
        GetSplinePointWithLookahead(pOut, wp1, wpIdx + 1, lookahead - (deltaMag - bToPtProj));
      else
        pOut = delta * (lookahead - (deltaMag - bToPtProj)) + wp1;
    } else {
      pOut = wp1;
    }
  }
}

void CPathFindSearch::GetSplinePointWithLookahead(zeus::CVector3f& pOut, const zeus::CVector3f& p1,
                                                  float lookahead) const {
  GetSplinePointWithLookahead(pOut, p1, xc8_curWaypoint, lookahead);
}

} // namespace urde