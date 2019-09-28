#pragma once

#include "Runtime/Collision/CCollisionPrimitive.hpp"
#include "Runtime/Collision/CMetroidAreaCollider.hpp"
#include "Runtime/Collision/COBBTree.hpp"

#include <zeus/CMRay.hpp>
#include <zeus/COBBox.hpp>
#include <zeus/CPlane.hpp>

namespace urde {
class CRayCastInfo {
  const zeus::CMRay& x0_ray;
  const CMaterialFilter& x4_filter;
  float x8_mag;
  zeus::CPlane xc_plane = {zeus::skUp, 0.f};
  CMaterialList x20_material;

public:
  CRayCastInfo(const zeus::CMRay& ray, const CMaterialFilter& filter, float mag)
  : x0_ray(ray), x4_filter(filter), x8_mag(mag) {}
  const zeus::CMRay& GetRay() const { return x0_ray; }
  const CMaterialFilter& GetMaterialFilter() const { return x4_filter; }
  float GetMagnitude() const { return x8_mag; }
  float& Magnitude() { return x8_mag; }
  const zeus::CPlane& GetPlane() const { return xc_plane; }
  zeus::CPlane& Plane() { return xc_plane; }
  const CMaterialList& GetMaterial() const { return x20_material; }
  CMaterialList& Material() { return x20_material; }
};

class CCollidableOBBTree : public CCollisionPrimitive {
  friend class CCollidableOBBTreeGroup;
  COBBTree* x10_tree = nullptr;
  u32 x14_tries = 0;
  u32 x18_misses = 0;
  u32 x1c_hits = 0;
  static u32 sTableIndex;
  bool LineIntersectsLeaf(const COBBTree::CLeafData& leaf, CRayCastInfo& info) const;
  bool LineIntersectsOBBTree(const COBBTree::CNode& n0, const COBBTree::CNode& n1, CRayCastInfo& info) const;
  bool LineIntersectsOBBTree(const COBBTree::CNode& node, CRayCastInfo& info) const;
  CRayCastResult LineIntersectsTree(const zeus::CMRay& ray, const CMaterialFilter& filter, float maxTime,
                                    const zeus::CTransform& xf) const;
  static zeus::CPlane TransformPlane(const zeus::CPlane& pl, const zeus::CTransform& xf);
  bool SphereCollideWithLeafMoving(const COBBTree::CLeafData& leaf, const zeus::CTransform& xf,
                                   const zeus::CSphere& sphere, const CMaterialList& material,
                                   const CMaterialFilter& filter, const zeus::CVector3f& dir, double& dOut,
                                   CCollisionInfo& info) const;
  bool SphereCollisionMoving(const COBBTree::CNode& node, const zeus::CTransform& xf, const zeus::CSphere& sphere,
                             const zeus::COBBox& obb, const CMaterialList& material, const CMaterialFilter& filter,
                             const zeus::CVector3f& dir, double& dOut, CCollisionInfo& info) const;
  bool AABoxCollideWithLeafMoving(const COBBTree::CLeafData& leaf, const zeus::CTransform& xf, const zeus::CAABox& aabb,
                                  const CMaterialList& material, const CMaterialFilter& filter,
                                  const CMovingAABoxComponents& components, const zeus::CVector3f& dir, double& dOut,
                                  CCollisionInfo& info) const;
  bool AABoxCollisionMoving(const COBBTree::CNode& node, const zeus::CTransform& xf, const zeus::CAABox& aabb,
                            const zeus::COBBox& obb, const CMaterialList& material, const CMaterialFilter& filter,
                            const CMovingAABoxComponents& components, const zeus::CVector3f& dir, double& dOut,
                            CCollisionInfo& info) const;
  bool SphereCollisionBoolean(const COBBTree::CNode& node, const zeus::CTransform& xf, const zeus::CSphere& sphere,
                              const zeus::COBBox& obb, const CMaterialFilter& filter) const;
  bool AABoxCollisionBoolean(const COBBTree::CNode& node, const zeus::CTransform& xf, const zeus::CAABox& aabb,
                             const zeus::COBBox& obb, const CMaterialFilter& filter) const;
  bool SphereCollideWithLeaf(const COBBTree::CLeafData& leaf, const zeus::CTransform& xf, const zeus::CSphere& sphere,
                             const CMaterialList& material, const CMaterialFilter& filter,
                             CCollisionInfoList& infoList) const;
  bool SphereCollision(const COBBTree::CNode& node, const zeus::CTransform& xf, const zeus::CSphere& sphere,
                       const zeus::COBBox& obb, const CMaterialList& material, const CMaterialFilter& filter,
                       CCollisionInfoList& infoList) const;
  bool AABoxCollideWithLeaf(const COBBTree::CLeafData& leaf, const zeus::CTransform& xf, const zeus::CAABox& aabb,
                            const CMaterialList& material, const CMaterialFilter& filter, const zeus::CPlane* planes,
                            CCollisionInfoList& infoList) const;
  bool AABoxCollision(const COBBTree::CNode& node, const zeus::CTransform& xf, const zeus::CAABox& aabb,
                      const zeus::COBBox& obb, const CMaterialList& material, const CMaterialFilter& filter,
                      const zeus::CPlane* planes, CCollisionInfoList& infoList) const;

public:
  CCollidableOBBTree(const COBBTree* tree, const CMaterialList& material);
  ~CCollidableOBBTree() override = default;
  void ResetTestStats() const;
  void ResetTestStatsRecurse(const COBBTree::CNode&) const;
  u32 GetTableIndex() const override { return sTableIndex; }
  zeus::CAABox CalculateAABox(const zeus::CTransform&) const override;
  zeus::CAABox CalculateLocalAABox() const override;
  FourCC GetPrimType() const override;
  CRayCastResult CastRayInternal(const CInternalRayCastStructure&) const override;
};

} // namespace urde
