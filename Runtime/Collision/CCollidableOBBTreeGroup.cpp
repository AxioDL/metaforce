#include "CCollidableOBBTreeGroup.hpp"
#include "CCollidableOBBTree.hpp"
#include "CCollidableAABox.hpp"
#include "CCollidableSphere.hpp"
#include "CInternalRayCastStructure.hpp"
#include "CollisionUtil.hpp"
#include "CToken.hpp"

namespace urde {
constexpr CCollisionPrimitive::Type sType(CCollidableOBBTreeGroup::SetStaticTableIndex, "CCollidableOBBTreeGroup");
u32 CCollidableOBBTreeGroup::sTableIndex = -1;

CCollidableOBBTreeGroupContainer::CCollidableOBBTreeGroupContainer(CInputStream& in) {
  const u32 treeCount = in.readUint32Big();
  x0_trees.reserve(treeCount);

  for (u32 i = 0; i < treeCount; i++) {
    auto tree = std::make_unique<COBBTree>(in);
    x0_trees.push_back(std::move(tree));
  }

  x10_aabbs.reserve(x0_trees.size());

  for (const std::unique_ptr<COBBTree>& tree : x0_trees) {
    x10_aabbs.push_back(CCollidableOBBTree(tree.get(), CMaterialList()).CalculateLocalAABox());
    x20_aabox.accumulateBounds(x10_aabbs.back());
  }
}

CCollidableOBBTreeGroupContainer::CCollidableOBBTreeGroupContainer(const zeus::CVector3f& extent, const zeus::CVector3f& center) {
  x0_trees.push_back(COBBTree::BuildOrientedBoundingBoxTree(extent, center));

  for (const std::unique_ptr<COBBTree>& tree : x0_trees) {
    x10_aabbs.push_back(CCollidableOBBTree(tree.get(), CMaterialList()).CalculateLocalAABox());
    x20_aabox.accumulateBounds(x10_aabbs.back());
  }
}

CCollidableOBBTreeGroup::CCollidableOBBTreeGroup(const CCollidableOBBTreeGroupContainer* container,
                                                 const CMaterialList& matList)
: CCollisionPrimitive(matList), x10_container(container) {}

void CCollidableOBBTreeGroup::ResetTestStats() const { /* Remove me? */ }

u32 CCollidableOBBTreeGroup::GetTableIndex() const { return sTableIndex; }

zeus::CAABox CCollidableOBBTreeGroup::CalculateAABox(const zeus::CTransform& xf) const {
  return x10_container->x20_aabox.getTransformedAABox(xf);
}

zeus::CAABox CCollidableOBBTreeGroup::CalculateLocalAABox() const { return x10_container->x20_aabox; }

FourCC CCollidableOBBTreeGroup::GetPrimType() const { return SBIG('OBTG'); }

CRayCastResult CCollidableOBBTreeGroup::CastRayInternal(const CInternalRayCastStructure& rayCast) const {
  CRayCastResult ret;

  zeus::CMRay xfRay = rayCast.GetRay().getInvUnscaledTransformRay(rayCast.GetTransform());
  auto aabbIt = x10_container->x10_aabbs.cbegin();
  float mag = rayCast.GetMaxTime();
  for (const std::unique_ptr<COBBTree>& tree : x10_container->x0_trees) {
    CCollidableOBBTree obbTree(tree.get(), GetMaterial());
    float tMin = 0.f;
    float tMax = 0.f;
    if (CollisionUtil::RayAABoxIntersection(xfRay, *aabbIt++, tMin, tMax)) {
      CInternalRayCastStructure localCast(xfRay.start, xfRay.dir, mag, zeus::CTransform(),
                                          rayCast.GetFilter());
      CRayCastResult localResult = obbTree.CastRayInternal(localCast);
      if (localResult.IsValid()) {
        if (ret.IsInvalid() || localResult.GetT() < ret.GetT()) {
          ret = localResult;
          mag = localResult.GetT();
        }
      }
    }
  }

  ret.Transform(rayCast.GetTransform());
  return ret;
}

const CCollisionPrimitive::Type& CCollidableOBBTreeGroup::GetType() { return sType; }

void CCollidableOBBTreeGroup::SetStaticTableIndex(u32 index) { sTableIndex = index; }

bool CCollidableOBBTreeGroup::SphereCollide(const CInternalCollisionStructure& collision, CCollisionInfoList& list) {
  bool ret = false;
  const CCollidableSphere& p0 = static_cast<const CCollidableSphere&>(collision.GetLeft().GetPrim());
  const CCollidableOBBTreeGroup& p1 = static_cast<const CCollidableOBBTreeGroup&>(collision.GetRight().GetPrim());

  zeus::CSphere s0 = p0.Transform(collision.GetLeft().GetTransform());
  zeus::COBBox obb1 = zeus::COBBox::FromAABox(p0.CalculateLocalAABox(), collision.GetRight().GetTransform().inverse() *
                                                                            collision.GetLeft().GetTransform());

  for (const std::unique_ptr<COBBTree>& tree : p1.x10_container->x0_trees) {
    CCollidableOBBTree obbTree(tree.get(), p1.GetMaterial());
    if (obbTree.SphereCollision(obbTree.x10_tree->GetRoot(), collision.GetRight().GetTransform(), s0, obb1,
                                p0.GetMaterial(), collision.GetLeft().GetFilter(), list))
      ret = true;
  }

  return ret;
}

bool CCollidableOBBTreeGroup::SphereCollideBoolean(const CInternalCollisionStructure& collision) {
  const CCollidableSphere& p0 = static_cast<const CCollidableSphere&>(collision.GetLeft().GetPrim());
  const CCollidableOBBTreeGroup& p1 = static_cast<const CCollidableOBBTreeGroup&>(collision.GetRight().GetPrim());

  zeus::CSphere s0 = p0.Transform(collision.GetLeft().GetTransform());
  zeus::COBBox obb1 = zeus::COBBox::FromAABox(p0.CalculateLocalAABox(), collision.GetRight().GetTransform().inverse() *
                                                                            collision.GetLeft().GetTransform());

  for (const std::unique_ptr<COBBTree>& tree : p1.x10_container->x0_trees) {
    CCollidableOBBTree obbTree(tree.get(), p1.GetMaterial());
    if (obbTree.SphereCollisionBoolean(obbTree.x10_tree->GetRoot(), collision.GetRight().GetTransform(), s0, obb1,
                                       collision.GetLeft().GetFilter()))
      return true;
  }

  return false;
}

bool CCollidableOBBTreeGroup::CollideMovingSphere(const CInternalCollisionStructure& collision,
                                                  const zeus::CVector3f& dir, double& mag, CCollisionInfo& info) {
  bool ret = false;
  const CCollidableSphere& p0 = static_cast<const CCollidableSphere&>(collision.GetLeft().GetPrim());
  const CCollidableOBBTreeGroup& p1 = static_cast<const CCollidableOBBTreeGroup&>(collision.GetRight().GetPrim());

  zeus::CSphere s0 = p0.Transform(collision.GetLeft().GetTransform());

  zeus::CAABox movedAABB = p0.CalculateLocalAABox();
  zeus::CVector3f moveVec = float(mag) * dir;
  movedAABB.accumulateBounds(movedAABB.min + moveVec);
  movedAABB.accumulateBounds(movedAABB.max + moveVec);

  zeus::COBBox p0Obb = zeus::COBBox::FromAABox(movedAABB, collision.GetRight().GetTransform().inverse() *
                                                              collision.GetLeft().GetTransform());

  for (const std::unique_ptr<COBBTree>& tree : p1.x10_container->x0_trees) {
    CCollidableOBBTree obbTree(tree.get(), p1.GetMaterial());
    CMetroidAreaCollider::ResetInternalCounters();
    if (obbTree.SphereCollisionMoving(obbTree.x10_tree->GetRoot(), collision.GetRight().GetTransform(), s0, p0Obb,
                                      p0.GetMaterial(), collision.GetLeft().GetFilter(), dir, mag, info))
      ret = true;
  }

  return ret;
}

bool CCollidableOBBTreeGroup::AABoxCollide(const CInternalCollisionStructure& collision, CCollisionInfoList& list) {
  bool ret = false;
  const CCollidableAABox& p0 = static_cast<const CCollidableAABox&>(collision.GetLeft().GetPrim());
  const CCollidableOBBTreeGroup& p1 = static_cast<const CCollidableOBBTreeGroup&>(collision.GetRight().GetPrim());

  zeus::CAABox b0 = p0.CalculateAABox(collision.GetLeft().GetTransform());
  zeus::COBBox p0Obb = zeus::COBBox::FromAABox(p0.CalculateLocalAABox(), collision.GetRight().GetTransform().inverse() *
                                                                             collision.GetLeft().GetTransform());

  zeus::CPlane planes[] = {{zeus::skRight, b0.min.dot(zeus::skRight)},
                           {zeus::skLeft, b0.max.dot(zeus::skLeft)},
                           {zeus::skForward, b0.min.dot(zeus::skForward)},
                           {zeus::skBack, b0.max.dot(zeus::skBack)},
                           {zeus::skUp, b0.min.dot(zeus::skUp)},
                           {zeus::skDown, b0.max.dot(zeus::skDown)}};

  for (const std::unique_ptr<COBBTree>& tree : p1.x10_container->x0_trees) {
    CCollidableOBBTree obbTree(tree.get(), p1.GetMaterial());
    if (obbTree.AABoxCollision(obbTree.x10_tree->GetRoot(), collision.GetRight().GetTransform(), b0, p0Obb,
                               p0.GetMaterial(), collision.GetLeft().GetFilter(), planes, list))
      ret = true;
  }

  return ret;
}

bool CCollidableOBBTreeGroup::AABoxCollideBoolean(const CInternalCollisionStructure& collision) {
  const CCollidableAABox& p0 = static_cast<const CCollidableAABox&>(collision.GetLeft().GetPrim());
  const CCollidableOBBTreeGroup& p1 = static_cast<const CCollidableOBBTreeGroup&>(collision.GetRight().GetPrim());

  zeus::CAABox b0 = p0.CalculateAABox(collision.GetLeft().GetTransform());
  zeus::COBBox p0Obb = zeus::COBBox::FromAABox(p0.CalculateLocalAABox(), collision.GetRight().GetTransform().inverse() *
                                                                             collision.GetLeft().GetTransform());

  for (const std::unique_ptr<COBBTree>& tree : p1.x10_container->x0_trees) {
    CCollidableOBBTree obbTree(tree.get(), p1.GetMaterial());
    if (obbTree.AABoxCollisionBoolean(obbTree.x10_tree->GetRoot(), collision.GetRight().GetTransform(), b0, p0Obb,
                                      collision.GetLeft().GetFilter()))
      return true;
  }

  return false;
}

bool CCollidableOBBTreeGroup::CollideMovingAABox(const CInternalCollisionStructure& collision,
                                                 const zeus::CVector3f& dir, double& mag, CCollisionInfo& info) {
  bool ret = false;
  const CCollidableAABox& p0 = static_cast<const CCollidableAABox&>(collision.GetLeft().GetPrim());
  const CCollidableOBBTreeGroup& p1 = static_cast<const CCollidableOBBTreeGroup&>(collision.GetRight().GetPrim());

  zeus::CAABox b0 = p0.CalculateAABox(collision.GetLeft().GetTransform());

  CMovingAABoxComponents components(b0, dir);

  zeus::CAABox movedAABB = p0.CalculateLocalAABox();
  zeus::CVector3f moveVec = float(mag) * dir;
  movedAABB.accumulateBounds(movedAABB.min + moveVec);
  movedAABB.accumulateBounds(movedAABB.max + moveVec);

  zeus::COBBox p0Obb = zeus::COBBox::FromAABox(movedAABB, collision.GetRight().GetTransform().inverse() *
                                                              collision.GetLeft().GetTransform());

  for (const std::unique_ptr<COBBTree>& tree : p1.x10_container->x0_trees) {
    CCollidableOBBTree obbTree(tree.get(), p1.GetMaterial());
    CMetroidAreaCollider::ResetInternalCounters();
    if (obbTree.AABoxCollisionMoving(obbTree.x10_tree->GetRoot(), collision.GetRight().GetTransform(), b0, p0Obb,
                                     p0.GetMaterial(), collision.GetLeft().GetFilter(), components, dir, mag, info))
      ret = true;
  }

  return ret;
}

CFactoryFnReturn FCollidableOBBTreeGroupFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                                CObjectReference* selfRef) {
  return TToken<CCollidableOBBTreeGroupContainer>::GetIObjObjectFor(
      std::make_unique<CCollidableOBBTreeGroupContainer>(in));
}

} // namespace urde
