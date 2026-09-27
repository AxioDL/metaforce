#include "MetroidPrime/CFlameWarp.hpp"

#include "MetroidPrime/CStateManager.hpp"

#include "Collision/CMaterialFilter.hpp"
#include "Collision/CRayCastResult.hpp"
#include "Kyoto/Graphics/CColor.hpp"

#include "rstl/algorithm.hpp"
#include "rstl/vector.hpp"

static EMaterialTypes ProjectilePassthroughMaterial = kMT_ProjectilePassthrough;

// Particle fields are interleaved with a caller-supplied byte stride.
template < class T >
static inline T& GetParticleField(T* base, int stripe, int index) {
  return *reinterpret_cast< T* >(reinterpret_cast< char* >(base) + index * stripe);
}

CFlameWarp::CFlameWarp(float maxInfluenceDist, const CVector3f& warpPoint, bool collisionWarp)
: mCollisionPoints(warpPoint)
, mWarpPoint(warpPoint)
, mFloatingPoint(warpPoint)
, mMaxDistSq(0.f)
, mMinSize(FLT_MAX)
, mMaxSize(FLT_MIN)
, mMaxInfluenceDistSq(maxInfluenceDist * maxInfluenceDist)
, mStateMgr(nullptr)
, mActivated(false)
, mCollisionWarp(collisionWarp)
, mProcessed(false) {}

CFlameWarp::~CFlameWarp() {}

bool CFlameWarp::UpdateWarp() { return mActivated; }

struct SFlameParticleSortKey {
  float transp;
  uchar index;

  SFlameParticleSortKey(float transp, const uchar index) : transp(transp), index(index) {}

  bool operator<(const SFlameParticleSortKey& other) const { return transp < other.transp; }
};
CHECK_SIZEOF(SFlameParticleSortKey, 8)

void CFlameWarp::ModifyParticles(int particleCount, int stripe, int*, CVector3f* particlePrevPos,
                                 CVector3f* particlePos, CVector3f* particleVelocity, CColor* color,
                                 float* lineLengthOrSize, float* lineWidthOrRota) {
  if (mStateMgr == nullptr || particleCount < 9) {
    return;
  }

  rstl::vector< SFlameParticleSortKey > vec;
  vec.reserve(particleCount);

  mMinSize = FLT_MAX;
  mMaxSize = FLT_MIN;
  float maxTransp = 0.f;

  for (int i = 0; i < particleCount; ++i) {
    CVector3f& partPos = GetParticleField(particlePos, stripe, i);
    const float transp = 1.f - GetParticleField(color, stripe, i).GetAlpha();

    if (transp > maxTransp) {
      const CVector3f warpDelta = partPos - mWarpPoint;
      const float distSq = CVector3f::Dot(warpDelta, warpDelta);
      if (distSq > mMaxDistSq && distSq < mMaxInfluenceDistSq) {
        mMaxDistSq = distSq;
        maxTransp = transp;
        mFloatingPoint = partPos;
      }
    }

    if (GetParticleField(lineLengthOrSize, stripe, i) < mMinSize) {
      mMinSize = GetParticleField(lineLengthOrSize, stripe, i);
    }
    if (GetParticleField(lineLengthOrSize, stripe, i) > mMaxSize) {
      mMaxSize = GetParticleField(lineLengthOrSize, stripe, i);
    }

    vec.push_back(SFlameParticleSortKey(transp, i));
    if (mCollisionWarp) {
      CVector3f& partVel = GetParticleField(particleVelocity, stripe, i);
      const CVector3f delta = partPos - GetParticleField(particlePrevPos, stripe, i);

      if (!(delta.MagSquared() < 0.0011920929f)) {
        const CVector3f deltaNorm = delta.AsNormalized();
        const CVector3f behindPos = GetParticleField(particlePrevPos, stripe, i) - deltaNorm * 5.f;
        float fullDeltaMag = (partPos - behindPos).Magnitude();

        const CRayCastResult result = mStateMgr->RayStaticIntersection(
            behindPos, deltaNorm, fullDeltaMag,
            CMaterialFilter::MakeIncludeExclude(CMaterialList(SolidMaterial),
                                                CMaterialList(ProjectilePassthroughMaterial)));

        if (result.IsValid()) {
          const CUnitVector3f normal = result.GetPlane().GetNormal();
          const float dist = CVector3f::Dot(normal, partPos) - result.GetPlane().GetConstant();
          if (!(dist > 0.f)) {
            partPos -= dist * normal;
            if (CVector3f::Dot(normal, partVel) < 0.f) {
              const CVector3f& prevStepPos = partPos - partVel;
              const float factor =
                  -(CVector3f::Dot(prevStepPos, normal) - result.GetPlane().GetConstant()) /
                      CVector3f::Dot(partVel, normal) -
                  1.f;
              partPos = partPos + factor * partVel;
              partVel -= partVel * 0.001f;
            }
          }
        }
      }
    }
  }

  rstl::sort(vec.begin(), vec.end());

  int vecIdx = 0;
  const int pitch = particleCount / 9;

  for (int i = 0; i < mCollisionPoints.capacity(); ++i) {
    CVector3f& partPos = GetParticleField(particlePos, stripe, vec[vecIdx].index);
    mCollisionPoints[i] = partPos;
    if (i > 0) {
      const CVector3f delta = mCollisionPoints[i] - mCollisionPoints[i - 1];
      if (delta.Magnitude() < 0.0011920929f) {
        mCollisionPoints[i] += delta.AsNormalized() * 0.0011920929f;
      }
    }
    vecIdx += pitch;
  }

  mCollisionPoints[0] = mWarpPoint;
  mFloatingPoint = mCollisionPoints[8];
  mProcessed = true;
}

void CFlameWarp::ResetPosition(const CVector3f& pos) {
  rstl::reserved_vector< CVector3f, 9 >::iterator it;
  for (it = mCollisionPoints.begin(); it != mCollisionPoints.end(); ++it) {
    *it = pos;
  }
  mProcessed = false;
}

bool CFlameWarp::IsActivated() { return mActivated; }

FourCC CFlameWarp::Get4CharID() { return 'FWRP'; }
