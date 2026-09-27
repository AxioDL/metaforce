#ifndef _CFLAMEWARP
#define _CFLAMEWARP

#include "Kyoto/Particles/CWarp.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/reserved_vector.hpp"

class CStateManager;
class CColor;

class CFlameWarp : public CWarp {
public:
  CFlameWarp(float maxInfluenceDist, const CVector3f& warpPoint, bool collisionWarp);
  ~CFlameWarp() override;

  const rstl::reserved_vector< CVector3f, 9 >& GetCollisionPoints() const {
    return mCollisionPoints;
  }
  float GetMinSize() const { return mMinSize; }
  float GetMaxSize() const { return mMaxSize; }
  void SetWarpPoint(const CVector3f& p) { mWarpPoint = p; }
  void SetFloatingPoint(const CVector3f& p) { mFloatingPoint = p; }
  const CVector3f& GetFloatingPoint() const { return mFloatingPoint; }
  void SetMaxDistSq(float d) { mMaxDistSq = d; }
  void SetStateManager(CStateManager& mgr) { mStateMgr = &mgr; }
  bool UpdateWarp() override;
  void ModifyParticles(int particleCount, int stripe, int*, CVector3f* particlePrevPos,
                       CVector3f* particlePos, CVector3f* particleVelocity, CColor* color,
                       float* lineLengthOrSize, float* lineWidthOrRota) override;
  void Activate(bool val) override { mActivated = val; }
  bool IsActivated() override;
  bool IsProcessed() const { return mProcessed; }
  FourCC Get4CharID() override;
  void ResetPosition(const CVector3f& pos);
  CAABox CalculateBounds() const;

private:
  rstl::reserved_vector< CVector3f, 9 > mCollisionPoints;
  CVector3f mWarpPoint;
  CVector3f mFloatingPoint;
  float mMaxDistSq;
  float mMinSize;
  float mMaxSize;
  float mMaxInfluenceDistSq;
  CStateManager* mStateMgr;
  bool mActivated : 1;
  bool mCollisionWarp : 1;
  bool mProcessed : 1;
};
CHECK_SIZEOF(CFlameWarp, 0xa4)

#endif // _CFLAMEWARP
