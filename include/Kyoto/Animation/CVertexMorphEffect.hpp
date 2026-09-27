#ifndef _CVERTEXMORPHEFFECT
#define _CVERTEXMORPHEFFECT

#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/TToken.hpp"

#include "rstl/vector.hpp"

class CRandom16;

class CSkinRules;
class CPoseAsTransforms;
class CVertexMorphEffect {
public:
  CVertexMorphEffect(const CUnitVector3f& dir, const CVector3f& pos, float duration,
                     float diagExtent, CRandom16& random)
  : mDir(dir)
  , mPos(pos)
  , mDuration(duration)
  , mElapsed(0.f)
  , mDiagExtent(diagExtent)
  , mRandom(random) {}
  void Update(float dt);

  void MorphVertices(CVector3f* workspace, const CVector3f* averageNormals,
                     const TLockedToken< CSkinRules >& skinRules, const CPoseAsTransforms& pose,
                     uint vertexCount) const;
  void Reset(const CUnitVector3f& dir, const CVector3f& pos, float duration);

private:
  CUnitVector3f mDir;
  CVector3f mPos;
  float mDuration;
  float mElapsed;
  float mDiagExtent;
  CRandom16& mRandom;
  mutable rstl::vector< uint > mIndices;
  mutable rstl::vector< float > mFloats;
};

CHECK_SIZEOF(CVertexMorphEffect, 0x48)

#endif // _CVERTEXMORPHEFFECT
