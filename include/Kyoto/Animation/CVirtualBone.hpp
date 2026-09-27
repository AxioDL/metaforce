#ifndef _CVIRTUALBONE
#define _CVIRTUALBONE

#include "Kyoto/Animation/CSegId.hpp"
#include "Kyoto/Math/CMatrix3f.hpp"
#include "Kyoto/Math/CTransform4f.hpp"

struct SSkinWeighting {
  CSegId mId;
  float mWeight;
  explicit SSkinWeighting(CInputStream& in)
  : mId(in.Get< int >()), mWeight(in.Get< float >()) {}
};

class CPoseAsTransforms;
class CVirtualBone {
public:
  explicit CVirtualBone(CInputStream& in);
  const rstl::reserved_vector< SSkinWeighting, 3 >& GetWeights() const { return mWeights; }
  int GetNumIndices() const { return mVertexCount; }
  const CTransform4f& GetTransform() const { return mXf; }

  void BuildPoints(const ushort*, volatile void*, int) const;
  void BuildNormals(const ushort*, volatile void*, int) const;
  void BuildNormals(const CVector3f*, CVector3f*, int) const;
  void BuildAccumulatedTransform(const CPoseAsTransforms& pose, const CVector3f* points) const;
  void BuildFinalPosMatrix(const CPoseAsTransforms& pose, const CVector3f* points) const;

private:
  rstl::reserved_vector< SSkinWeighting, 3 > mWeights;
  int mVertexCount;
  mutable CTransform4f mXf;
  mutable CMatrix3f mRotation;
};

CHECK_SIZEOF(CVirtualBone, 0x74)

#endif // _CVIRTUALBONE
