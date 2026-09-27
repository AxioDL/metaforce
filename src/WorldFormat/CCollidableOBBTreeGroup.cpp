#include "WorldFormat/CCollidableOBBTreeGroup.hpp"

#include "Collision/CCollidableAABox.hpp"
#include "Collision/CCollidableSphere.hpp"
#include "Collision/CCollisionInfoList.hpp"
#include "Collision/COBBox.hpp"
#include "Collision/CollisionUtil.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "WorldFormat/CCollidableOBBTree.hpp"
#include "WorldFormat/COBBTree.hpp"

uint CCollidableOBBTreeGroup::sTableIndex = -1;

const CFactoryFnReturn FCollidableOBBTreeGroupFactory(const SObjectTag& tag, CInputStream& in,
                                                const CVParamTransfer& xfer) {
  return rs_new CCollidableOBBTreeGroupContainer(in);
}

CCollidableOBBTreeGroupContainer::CCollidableOBBTreeGroupContainer(CInputStream& in)
: mAabox(CAABox::MakeMaxInvertedBox()) {
  int obbCount = in.ReadInt32();
  mTrees.reserve(obbCount);
  for (uint i = 0; i < obbCount; ++i) {
    mTrees.push_back(rs_new COBBTree(in));
  }

  mAabbs.reserve(mTrees.size());
  for (AUTO(it, mTrees.begin()); it != mTrees.end(); ++it) {
    CCollidableOBBTree tree(it->get(), CMaterialList());
    CAABox box = tree.CalculateLocalAABox();
    mAabbs.push_back(box);
    mAabox.AccumulateBounds(box.GetMinPoint());
    mAabox.AccumulateBounds(box.GetMaxPoint());
  }
}

CCollidableOBBTreeGroupContainer::CCollidableOBBTreeGroupContainer(const CVector3f& extent,
                                                                   const CVector3f& center)
: mAabox(CAABox::MakeMaxInvertedBox()) {
  mTrees.reserve(1);
  mTrees.push_back(
      rstl::auto_ptr< COBBTree >(COBBTree::BuildOrientedBoundingBoxTree(extent, center)));
  mAabbs.reserve(1);
  for (AUTO(it, mTrees.begin()); it != mTrees.end(); ++it) {
    CCollidableOBBTree tree(it->get(), CMaterialList());
    CAABox box = tree.CalculateLocalAABox();
    mAabbs.push_back(box);
    mAabox.AccumulateBounds(box.GetMinPoint());
    mAabox.AccumulateBounds(box.GetMaxPoint());
  }
}

void CCollidableOBBTreeGroup::SetStaticTableIndex(uint index) { sTableIndex = index; }

CCollisionPrimitive::Type CCollidableOBBTreeGroup::GetType() {
  return Type(SetStaticTableIndex, "CCollidableOBBTreeGroup");
}

CCollidableOBBTreeGroup::CCollidableOBBTreeGroup(CCollidableOBBTreeGroupContainer* container,
                                                 const CMaterialList& material)
: CCollisionPrimitive(material)
, mContainer(container) {}

COBBTree* CCollidableOBBTreeGroup::GetOBBTreeAABox(int idx) const {
  return mContainer->mTrees[idx].get();
}

void CRayCastResult::Transform(const CTransform4f& xf) {
  mPoint = xf * mPoint;
  CVector3f normal = xf.Rotate(mPlane.GetNormal());
  mPlane = CPlane(mPoint, CUnitVector3f(normal.GetX(), normal.GetY(), normal.GetZ()));
}

CRayCastResult
CCollidableOBBTreeGroup::CastRayInternal(const CInternalRayCastStructure& rayCast) const {
  CRayCastResult result;
  float mag = rayCast.GetMaxTime();
  AUTO(treeIt, mContainer->mTrees.begin());
  AUTO(aabbIt, mContainer->mAabbs.begin());
  CMRay ray = rayCast.GetRay().GetInvUnscaledTransformRay(rayCast.GetTransform());

  for (; treeIt != mContainer->mTrees.end(); ++treeIt, ++aabbIt) {
    CCollidableOBBTree tree(treeIt->get(), GetMaterial());
    float tMin = 0.f;
    float tMax = 0.f;
    if (CollisionUtil::RayAABoxIntersection(ray, *aabbIt, tMin, tMax)) {
      const CRayCastResult& localResult = tree.CastRay(
          ray.GetStart(), ray.GetDirection(), mag, rayCast.GetFilter(), CTransform4f::Identity());
      if (localResult.IsValid()) {
        if (result.IsValid()) {
          if (localResult.GetTime() < result.GetTime()) {
            result = localResult;
            mag = localResult.GetTime();
          }
        } else {
          result = localResult;
          mag = localResult.GetTime();
        }
      }
    }
  }
  result.Transform(rayCast.GetTransform());
  return result;
}

bool CCollidableOBBTreeGroup::AABoxCollide(const CInternalCollisionStructure& collision,
                                           CCollisionInfoList& list) {
  const CCollisionPrimitive& left = collision.GetLeft().GetPrim();
  const CCollidableOBBTreeGroup& right =
      static_cast< const CCollidableOBBTreeGroup& >(collision.GetRight().GetPrim());

  CAABox bounds = left.CalculateAABox(collision.GetLeft().GetTransform());
  CTransform4f xf = collision.GetRight().GetTransform();
  CTransform4f relativeXf = xf.GetQuickInverse() * collision.GetLeft().GetTransform();
  COBBox obb = COBBox::FromAABox(collision.GetLeft().GetPrim().CalculateLocalAABox(), relativeXf);

  const CVector3f min = bounds.GetMinPoint();
  const CVector3f max = bounds.GetMaxPoint();
  const CUnitVector3f rightNormal(1.f, 0.f, 0.f);
  const CUnitVector3f forwardNormal(0.f, 1.f, 0.f);
  const CUnitVector3f upNormal(0.f, 0.f, 1.f);
  CPlane planes[6] = {CPlane(min, rightNormal),   CPlane(max, -rightNormal),
                      CPlane(min, forwardNormal), CPlane(max, -forwardNormal),
                      CPlane(min, upNormal),      CPlane(max, -upNormal)};
  bool result = false;

  for (int i = 0; i < right.GetContainer()->NumTrees(); ++i) {
    CCollidableOBBTree tree(right.GetOBBTreeAABox(i), right.GetMaterial());
    if (tree.AABoxCollision(tree.GetOBBTree().GetRoot(), xf, bounds, obb,
                            collision.GetLeft().GetPrim().GetMaterial(),
                            collision.GetLeft().GetFilter(), planes, list)) {
      result = true;
    }
  }
  return result;
}

bool CCollidableOBBTreeGroup::AABoxCollideBoolean(const CInternalCollisionStructure& collision) {
  const CCollisionPrimitive& left = collision.GetLeft().GetPrim();
  const CCollidableOBBTreeGroup& right =
      static_cast< const CCollidableOBBTreeGroup& >(collision.GetRight().GetPrim());

  CAABox bounds = left.CalculateAABox(collision.GetLeft().GetTransform());
  CTransform4f xf = collision.GetRight().GetTransform();
  CTransform4f relativeXf = xf.GetQuickInverse() * collision.GetLeft().GetTransform();
  COBBox obb = COBBox::FromAABox(collision.GetLeft().GetPrim().CalculateLocalAABox(), relativeXf);

  for (int i = 0; i < right.GetContainer()->NumTrees(); ++i) {
    CCollidableOBBTree tree(right.GetOBBTreeAABox(i), right.GetMaterial());
    if (tree.AABoxCollisionBoolean(tree.GetOBBTree().GetRoot(), xf, bounds, obb,
                                   collision.GetLeft().GetFilter())) {
      return true;
    }
  }
  return false;
}

bool CCollidableOBBTreeGroup::SphereCollide(const CInternalCollisionStructure& collision,
                                            CCollisionInfoList& list) {
  const CCollidableOBBTreeGroup& right =
      static_cast< const CCollidableOBBTreeGroup& >(collision.GetRight().GetPrim());
  const CCollidableSphere& left =
      static_cast< const CCollidableSphere& >(collision.GetLeft().GetPrim());

  CSphere sphere = left.Transform(collision.GetLeft().GetTransform());
  CTransform4f xf = collision.GetRight().GetTransform();
  CTransform4f relativeXf = xf.GetQuickInverse() * collision.GetLeft().GetTransform();
  COBBox obb = COBBox::FromAABox(left.CalculateLocalAABox(), relativeXf);
  bool result = false;

  for (int i = 0; i < right.GetContainer()->NumTrees(); ++i) {
    CCollidableOBBTree tree(right.GetOBBTreeAABox(i), right.GetMaterial());
    if (tree.SphereCollision(tree.GetOBBTree().GetRoot(), xf, sphere, obb, left.GetMaterial(),
                             collision.GetLeft().GetFilter(), list)) {
      result = true;
    }
  }
  return result;
}

bool CCollidableOBBTreeGroup::SphereCollideBoolean(const CInternalCollisionStructure& collision) {
  const CCollidableSphere& left =
      static_cast< const CCollidableSphere& >(collision.GetLeft().GetPrim());
  const CCollidableOBBTreeGroup& right =
      static_cast< const CCollidableOBBTreeGroup& >(collision.GetRight().GetPrim());

  CSphere sphere = left.Transform(collision.GetLeft().GetTransform());
  CTransform4f xf = collision.GetRight().GetTransform();
  CTransform4f relativeXf = xf.GetQuickInverse() * collision.GetLeft().GetTransform();
  COBBox obb = COBBox::FromAABox(left.CalculateLocalAABox(), relativeXf);

  for (int i = 0; i < right.GetContainer()->NumTrees(); ++i) {
    CCollidableOBBTree tree(right.GetOBBTreeAABox(i), right.GetMaterial());
    if (tree.SphereCollisionBoolean(tree.GetOBBTree().GetRoot(), xf, sphere, obb,
                                    collision.GetLeft().GetFilter())) {
      return true;
    }
  }
  return false;
}

bool CCollidableOBBTreeGroup::CollideMovingAABox(const CInternalCollisionStructure& collision,
                                                 const CVector3f& dir, double& mag,
                                                 CCollisionInfo& info) {
  const CCollisionPrimitive& left = collision.GetLeft().GetPrim();
  const CCollidableOBBTreeGroup* right =
      static_cast< const CCollidableOBBTreeGroup* >(&collision.GetRight().GetPrim());

  CAABox bounds = left.CalculateAABox(collision.GetLeft().GetTransform());
  CTransform4f xf = collision.GetRight().GetTransform();
  CTransform4f relativeXf = xf.GetQuickInverse() * collision.GetLeft().GetTransform();
  CMetroidAreaCollider::CMovingAABoxComponents components(bounds, dir);
  CAABox movedBounds = collision.GetLeft().GetPrim().CalculateLocalAABox();
  CVector3f move = CCast::ToReal32(mag) * dir;
  movedBounds.AccumulateBounds(movedBounds.GetMaxPoint() + move);
  movedBounds.AccumulateBounds(movedBounds.GetMinPoint() + move);
  COBBox obb = COBBox::FromAABox(movedBounds, relativeXf);
  bool result = false;

  for (int i = 0; i < right->GetContainer()->NumTrees(); ++i) {
    CCollidableOBBTree tree(right->GetOBBTreeAABox(i), right->GetMaterial());
    CMetroidAreaCollider::ResetInternalCounters();
    if (tree.AABoxCollisionMoving(tree.GetOBBTree().GetRoot(), xf, bounds, obb,
                                  collision.GetLeft().GetPrim().GetMaterial(),
                                  collision.GetLeft().GetFilter(), components, dir, mag, info)) {
      result = true;
    }
  }
  return result;
}

bool CCollidableOBBTreeGroup::CollideMovingSphere(const CInternalCollisionStructure& collision,
                                                  const CVector3f& dir, double& mag,
                                                  CCollisionInfo& info) {
  const CCollisionPrimitive& left = collision.GetLeft().GetPrim();
  const CCollidableOBBTreeGroup& right =
      static_cast< const CCollidableOBBTreeGroup& >(collision.GetRight().GetPrim());

  CSphere sphere =
      static_cast< const CCollidableSphere& >(left).Transform(collision.GetLeft().GetTransform());
  CTransform4f xf = collision.GetRight().GetTransform();
  CTransform4f relativeXf = xf.GetQuickInverse() * collision.GetLeft().GetTransform();
  CAABox movedBounds = collision.GetLeft().GetPrim().CalculateLocalAABox();
  CVector3f move = CCast::ToReal32(mag) * dir;
  movedBounds.AccumulateBounds(movedBounds.GetMaxPoint() + move);
  movedBounds.AccumulateBounds(movedBounds.GetMinPoint() + move);
  COBBox obb = COBBox::FromAABox(movedBounds, relativeXf);
  bool result = false;

  for (int i = 0; i < right.GetContainer()->NumTrees(); ++i) {
    CCollidableOBBTree tree(right.GetOBBTreeAABox(i), right.GetMaterial());
    CMetroidAreaCollider::ResetInternalCounters();
    if (tree.SphereCollisionMoving(tree.GetOBBTree().GetRoot(), xf, sphere, obb,
                                   collision.GetLeft().GetPrim().GetMaterial(),
                                   collision.GetLeft().GetFilter(), dir, mag, info)) {
      result = true;
    }
  }
  return result;
}

CAABox CCollidableOBBTreeGroup::CalculateAABox(const CTransform4f& xf) const {
  return mContainer->mAabox.GetTransformedAABox(xf);
}

CAABox CCollidableOBBTreeGroup::CalculateLocalAABox() const { return mContainer->mAabox; }

FourCC CCollidableOBBTreeGroup::GetPrimType() const { return 'OBTG'; }

uint CCollidableOBBTreeGroup::GetTableIndex() const { return sTableIndex; }
