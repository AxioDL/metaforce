#pragma once

#include "World/CEntityInfo.hpp"
#include "zeus/CVector3f.hpp"

namespace urde {
class CStateManager;
class CMaterialFilter;

class CCameraSpline {
  friend class CBallCamera;
  std::vector<zeus::CVector3f> x4_positions;
  std::vector<TUniqueId> x14_wpTracker;
  std::vector<float> x24_t;
  std::vector<zeus::CVector3f> x34_directions;
  float x44_length = 0.f;
  bool x48_closedLoop = false;
  bool GetSurroundingPoints(size_t idx, rstl::reserved_vector<zeus::CVector3f, 4>& positions,
                            rstl::reserved_vector<zeus::CVector3f, 4>& directions) const;

public:
  explicit CCameraSpline(bool closedLoop);
  void CalculateKnots(TUniqueId cameraId, const std::vector<SConnection>& connections, CStateManager& mgr);
  void Initialize(TUniqueId cameraId, const std::vector<SConnection>& connections, CStateManager& mgr);
  void Reset(size_t size);
  void AddKnot(const zeus::CVector3f& pos, const zeus::CVector3f& dir);
  void SetKnotPosition(size_t idx, const zeus::CVector3f& pos);
  const zeus::CVector3f& GetKnotPosition(size_t idx) const;
  float GetKnotT(size_t idx) const;
  float CalculateSplineLength();
  void UpdateSplineLength() { x44_length = CalculateSplineLength(); }
  zeus::CTransform GetInterpolatedSplinePointByLength(float pos) const;
  zeus::CVector3f GetInterpolatedSplinePointByTime(float time, float range) const;
  float FindClosestLengthOnSpline(float time, const zeus::CVector3f& p) const;
  float ValidateLength(float t) const;
  float ClampLength(const zeus::CVector3f& pos, bool collide, const CMaterialFilter& filter,
                    const CStateManager& mgr) const;
  s32 GetSize() const { return x4_positions.size(); }
  float GetLength() const { return x44_length; }
  bool IsClosedLoop() const { return x48_closedLoop; }
};

} // namespace urde
