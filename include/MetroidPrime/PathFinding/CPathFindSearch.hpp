#ifndef _CPATHFINDSEARCH
#define _CPATHFINDSEARCH

#include <Kyoto/Math/CVector3f.hpp>
#include <rstl/reserved_vector.hpp>
#include <types.h>
class CPFArea;
class CPFRegion;
class CBeetle;

class CPathFindSearch {
  friend class CBeetle;

public:
  enum EResult {
    kR_Success,
    kR_InvalidArea,
    kR_NoSourcePoint,
    kR_NoDestPoint,
    kR_NoPath,
  };

  CPathFindSearch(CPFArea* area, uint flags, uint index, float chRadius, float chHeight);

  void SetCharacterRadius(float radius) { mChRadius = radius; }
  float GetCharacterHeight() const { return mChHeight; }
  const rstl::reserved_vector< CVector3f, 16 >& GetWaypoints() const { return mWaypoints; }
  float RemainingPathDistance(const CVector3f& pos) const;
  void SetCharacterHeight(float height) { mChHeight = height; }

  void SetArea(CPFArea* area) { mArea = area; }
  void SetPadding(const float pad) { mPadding = pad; }

  const CVector3f& GetPoint() const {
    return mCurWaypoint + 1 < mWaypoints.size() ? mWaypoints[mCurWaypoint + 1]
                                                     : mWaypoints[mCurWaypoint];
  }
  EResult PathExists(const CVector3f& source, const CVector3f& destination) const;
  EResult OnPath(const CVector3f& pos) const;
  EResult Search(const CVector3f& source, const CVector3f& destination);
  EResult FindClosestReachablePoint(const CVector3f& source, CVector3f& destination) const;
  EResult GetResult() const { return mResult; }

  void Advance() { ++mCurWaypoint; }
  const bool SegmentOver(const CVector3f& pos) const;
  void GetSplinePointWithLookahead(CVector3f& point, const CVector3f& pos, float lookahead) const;
  int GetCurrentWaypoint() const { return mCurWaypoint; }
  bool IsOver() const { return mCurWaypoint >= mWaypoints.size() - 1; }
  void GetSplinePoint(CVector3f& point, const CVector3f& pos) const;

  bool IsShagged() const { return mResult != kR_Success; }

private:
  void GetSplinePoint(CVector3f& point, const CVector3f& pos, int waypoint) const;
  void GetSplinePointWithLookahead(CVector3f& point, const CVector3f& pos, int waypoint,
                                   float lookahead) const;
  bool Search(rstl::reserved_vector< CPFRegion*, 4 >& sourceRegions, const CVector3f& source,
              rstl::reserved_vector< CPFRegion*, 4 >& destRegions, const CVector3f& destination);
  CPFArea* mArea;

  rstl::reserved_vector< CVector3f, 16 > mWaypoints;
  int mCurWaypoint;
  EResult mResult;
  float mChHeight;
  float mChRadius;
  float mPadding;
  u32 mFlags; // 0x2: flyer, 0x4: path-always-exists (swimmers)
  u32 mIndexMask;
};
CHECK_SIZEOF(CPathFindSearch, 0xe4)
#endif // _CPATHFINDSEARCH
