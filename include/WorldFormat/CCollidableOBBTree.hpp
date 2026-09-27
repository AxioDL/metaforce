#ifndef _CCOLLIDABLEOBBTREE
#define _CCOLLIDABLEOBBTREE

#include "Collision/CCollisionPrimitive.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"

#include "Kyoto/Math/CPlane.hpp"

#include "WorldFormat/CMetroidAreaCollider.hpp"
#include "WorldFormat/COBBTree.hpp"

class CCollisionInfo;
class CCollisionInfoList;
class CMRay;
class COBBox;
class CSphere;

class CRayCastInfo {
  friend class CCollidableOBBTree;
  const CMRay& mRay;
  const CMaterialFilter& mFilter;
  float mMag;
  CPlane mPlane;
  CMaterialList mMaterial;

public:
  CRayCastInfo(const CMRay& ray, const CMaterialFilter& filter, float mag)
  : mRay(ray)
  , mFilter(filter)
  , mMag(mag)
  , mPlane(CVector3f::Zero(), CUnitVector3f(CVector3f(0.f, 0.f, 1.f), CUnitVector3f::kN_Yes))
  , mMaterial() {}

  const CMRay& GetRay() const { return mRay; }
  const CMaterialFilter& GetMaterialFilter() const { return mFilter; }
  float GetMagnitude() const { return mMag; }
  float& Magnitude() { return mMag; }
  const CPlane& GetPlane() const { return mPlane; }
  CPlane& Plane() { return mPlane; }
  const CMaterialList& GetMaterial() const { return mMaterial; }
  CMaterialList& Material() { return mMaterial; }
};

CHECK_SIZEOF(CRayCastInfo, 0x28)

class CCollidableOBBTree : public CCollisionPrimitive {
public:
  uint GetTableIndex() const override;
  CAABox CalculateAABox(const CTransform4f& xf) const override;
  CAABox CalculateLocalAABox() const override;
  FourCC GetPrimType() const override;
  CRayCastResult CastRayInternal(const CInternalRayCastStructure& rayCast) const override;

  CCollidableOBBTree(COBBTree* tree, const CMaterialList& list);

  const COBBTree& GetOBBTree() const { return *mTree; }

  bool AABoxCollision(const COBBTree::CNode& node, const CTransform4f& xf, const CAABox& aabb,
                      const COBBox& obb, const CMaterialList& material,
                      const CMaterialFilter& filter, const CPlane* planes,
                      CCollisionInfoList& infoList) const;
  bool AABoxCollideWithLeaf(const COBBTree::CLeafData& leaf, const CTransform4f& xf,
                            const CAABox& aabb, const CMaterialList& material,
                            const CMaterialFilter& filter, const CPlane* planes,
                            CCollisionInfoList& infoList) const;
  bool SphereCollision(const COBBTree::CNode& node, const CTransform4f& xf, const CSphere& sphere,
                       const COBBox& obb, const CMaterialList& material,
                       const CMaterialFilter& filter, CCollisionInfoList& infoList) const;
  bool SphereCollideWithLeaf(const COBBTree::CLeafData& leaf, const CTransform4f& xf,
                             const CSphere& sphere, const CMaterialList& material,
                             const CMaterialFilter& filter, CCollisionInfoList& infoList) const;
  bool AABoxCollisionBoolean(const COBBTree::CNode& node, const CTransform4f& xf,
                             const CAABox& aabb, const COBBox& obb,
                             const CMaterialFilter& filter) const;
  bool SphereCollisionBoolean(const COBBTree::CNode& node, const CTransform4f& xf,
                              const CSphere& sphere, const COBBox& obb,
                              const CMaterialFilter& filter) const;
  bool AABoxCollisionMoving(const COBBTree::CNode& node, const CTransform4f& xf, const CAABox& aabb,
                            const COBBox& obb, const CMaterialList& material,
                            const CMaterialFilter& filter,
                            const CMetroidAreaCollider::CMovingAABoxComponents& components,
                            const CVector3f& dir, double& dOut, CCollisionInfo& info) const;
  bool AABoxCollideWithLeafMoving(const COBBTree::CLeafData& leaf, const CTransform4f& xf,
                                  const CAABox& aabb, const CMaterialList& material,
                                  const CMaterialFilter& filter,
                                  const CMetroidAreaCollider::CMovingAABoxComponents& components,
                                  const CVector3f& dir, double& dOut, CCollisionInfo& info) const;
  bool SphereCollisionMoving(const COBBTree::CNode& node, const CTransform4f& xf,
                             const CSphere& sphere, const COBBox& obb,
                             const CMaterialList& material, const CMaterialFilter& filter,
                             const CVector3f& dir, double& dOut, CCollisionInfo& info) const;
  bool SphereCollideWithLeafMoving(const COBBTree::CLeafData& leaf, const CTransform4f& xf,
                                   const CSphere& sphere, const CMaterialList& material,
                                   const CMaterialFilter& filter, const CVector3f& dir,
                                   double& dOut, CCollisionInfo& info) const;

  CRayCastResult LineIntersectsTree(const CMRay& ray, const CMaterialFilter& filter, float maxTime,
                                    const CTransform4f& xf) const;
  bool LineIntersectsOBBTree(const COBBTree::CNode* node, CRayCastInfo& info) const;
  bool LineIntersectsOBBTree(const COBBTree::CNode* n0, const COBBTree::CNode* n1,
                             CRayCastInfo& info) const;
  bool LineIntersectsLeaf(const COBBTree::CLeafData& leaf, CRayCastInfo& info) const;

  static void SetStaticTableIndex(uint idx) { sTableIndex = idx; }

private:
  COBBTree* mTree;
  mutable uint mTries;
  mutable uint mMisses;
  mutable uint mHits;

  static uint sTableIndex;
};
CHECK_SIZEOF(CCollidableOBBTree, 0x20)

#endif // _CCOLLIDABLEOBBTREE
