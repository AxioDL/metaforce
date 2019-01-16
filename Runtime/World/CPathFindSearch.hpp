#pragma once

#include "RetroTypes.hpp"
#include "CPathFindArea.hpp"
#include "Graphics/CLineRenderer.hpp"

namespace urde {
class CPathFindSearch;

class CPathFindVisualizer {
  CLineRenderer m_spline = {CLineRenderer::EPrimitiveMode::LineStrip, 16, {}, true};
public:
  void Draw(const CPathFindSearch& path);
};

class CPathFindSearch {
public:
  enum class EResult { Success, InvalidArea, NoSourcePoint, NoDestPoint, NoPath };

private:
  CPFArea* x0_area;
  rstl::reserved_vector<zeus::CVector3f, 16> x4_waypoints;
  u32 xc8_curWaypoint = 0;
  EResult xcc_result;
  float xd0_chHeight;
  float xd4_chRadius;
  float xd8_padding = 10.f;
  u32 xdc_flags; // 0x2: flyer, 0x4: path-always-exists (swimmers)
  u32 xe0_indexMask;
  mutable std::experimental::optional<CPathFindVisualizer> m_viz;
  bool Search(rstl::reserved_vector<CPFRegion*, 4>& regs1, const zeus::CVector3f& p1,
              rstl::reserved_vector<CPFRegion*, 4>& regs2, const zeus::CVector3f& p2);
  void GetSplinePoint(zeus::CVector3f& pOut, const zeus::CVector3f& p1, u32 wpIdx) const;
  void GetSplinePointWithLookahead(zeus::CVector3f& pOut, const zeus::CVector3f& p1, u32 wpIdx, float lookahead) const;

public:
  CPathFindSearch(CPFArea* area, u32 flags, u32 index, float chRadius, float chHeight);
  EResult Search(const zeus::CVector3f& p1, const zeus::CVector3f& p2);
  EResult FindClosestReachablePoint(const zeus::CVector3f& p1, zeus::CVector3f& p2) const;
  EResult PathExists(const zeus::CVector3f& p1, const zeus::CVector3f& p2) const;
  EResult OnPath(const zeus::CVector3f& p1) const;
  EResult GetResult() const { return xcc_result; }
  u32 GetCurrentWaypoint() const { return xc8_curWaypoint; }
  void SetCurrentWaypoint(u32 wp) { xc8_curWaypoint = wp; }
  const rstl::reserved_vector<zeus::CVector3f, 16>& GetWaypoints() const { return x4_waypoints; }
  bool IsOver() const { return GetCurrentWaypoint() >= x4_waypoints.size() - 1; }
  bool IsShagged() const { return GetResult() != EResult::Success; }
  bool SegmentOver(const zeus::CVector3f& p1) const;
  void GetSplinePoint(zeus::CVector3f& pOut, const zeus::CVector3f& p1) const;
  void GetSplinePointWithLookahead(zeus::CVector3f& pOut, const zeus::CVector3f& p1, float lookahead) const;
  void SetArea(CPFArea* area) { x0_area = area; }
  float GetCharacterHeight() const { return xd0_chHeight; }
  void SetCharacterHeight(float h) { xd0_chHeight = h; }
  float RemainingPathDistance(const zeus::CVector3f& pos) const;
  void DebugDraw() const;
};

} // namespace urde
