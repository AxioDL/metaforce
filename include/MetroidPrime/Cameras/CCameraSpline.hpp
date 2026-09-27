#ifndef _CCAMERASPLINE
#define _CCAMERASPLINE

#include "Kyoto/Math/CTransform4f.hpp"
#include "types.h"

#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/Math/CVector3f.hpp"

#include "MetroidPrime/CEntityInfo.hpp"
#include "rstl/reserved_vector.hpp"
#include "rstl/vector.hpp"

class CMaterialFilter;
class CStateManager;
class CCameraSpline {
public:
  CCameraSpline(bool closedLoop);
  virtual ~CCameraSpline();

  void Initialise(TUniqueId uid, const rstl::vector< SConnection >& connections,
                  CStateManager& mgr);
  void CalculateKnots(TUniqueId uid, const rstl::vector< SConnection >& connections,
                      CStateManager& mgr);
  float GetKnotT(int idx) const;
  CVector3f GetKnotPosition(int idx) const;
  void SetKnotPosition(int idx, CVector3f pos);
  void AddKnot(CVector3f pos, CVector3f dir);
  void Reset(int size);
  bool GetSurroundingPoints(int idx, rstl::reserved_vector< CVector3f, 4 >& positions,
                            rstl::reserved_vector< CVector3f, 4 >& directions);
  float ClampLength(const CVector3f pos, bool collide, const CMaterialFilter filter,
                    const CStateManager& mgr) const;
  float CalculateSplineLength();
  void UpdateSplineLength() { mLength = CalculateSplineLength(); }
  float ValidateLength(float t) const;
  CVector3f GetInterpolatedSplinePointByTime(float time, float range) const;
  CTransform4f GetInterpolatedSplinePointByLength(float length) const;
  float FindClosestLengthOnSpline(float t, CVector3f point) const;

  int GetSize() const { return mPositions.size(); }
  float GetLength() const { return mLength; }
  bool IsClosedLoop() const { return mClosedLoop; }

private:
  rstl::vector< CVector3f > mPositions;
  rstl::vector< TUniqueId > mWpTracker;
  rstl::vector< float > mT;
  rstl::vector< CVector3f > mDirections;
  float mLength;
  bool mClosedLoop;
};
CHECK_SIZEOF(CCameraSpline, 0x4c)

#endif // _CCAMERASPLINE
