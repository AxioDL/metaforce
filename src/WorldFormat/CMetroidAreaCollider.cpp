#include "WorldFormat/CMetroidAreaCollider.hpp"

#include "Collision/CMRay.hpp"
#include "Collision/CollisionUtil.hpp"
#include "WorldFormat/CCollisionEdge.hpp"

#include "Kyoto/Math/CMath.hpp"

#include <math.h>
#include <string.h>

static uint gCalledClip = 0;
static uint gRejectedByClip = 0;
static uint gTrianglesProcessed = 0;
static uint gDupTrianglesProcessed = 0;
ushort CMetroidAreaCollider::sDupPrimitiveCheckCount = 0;
ushort CMetroidAreaCollider::sDupVertexList[0x2800];
ushort CMetroidAreaCollider::sDupEdgeList[0x6000];
ushort CMetroidAreaCollider::sDupTriangleList[0x4000];

bool CMetroidAreaCollider::ConvexPolyCollision(const CPlane* planes, const CVector3f* verts,
                                               CAABox& aabb) {
  typedef rstl::reserved_vector< CVector3f, 20 > ClipVec;
  ClipVec vecs[2];
  ++gCalledClip;
  ++gRejectedByClip;
  int vecIdx = 0;
  int otherVecIdx = 1;

  for (int i = 0; i < 3; ++i) {
    vecs[0].push_back(verts[i]);
  }

  for (int i = 0; i < 6; ++i) {
    ClipVec& vec = vecs[vecIdx];
    ClipVec& otherVec = vecs[otherVecIdx];
    otherVec.clear();

    bool inFrontOf = planes[i].GetHeight(vec.front()) >= 0.f;
    for (int j = 0; j < vec.size(); ++j) {
      const CVector3f& b = vec[j == vec.size() - 1 ? 0 : j + 1];
      if (inFrontOf) {
        otherVec.push_back(vec[j]);
      }
      bool nextInFrontOf = planes[i].GetHeight(b) >= 0.f;
      if (nextInFrontOf ^ inFrontOf) {
        float f = -(CVector3f::Dot(vec[j], planes[i].GetNormal()) - planes[i].GetConstant()) /
                  CVector3f::Dot(b - vec[j], planes[i].GetNormal());
        otherVec.push_back((1.f - f) * (vec[j] - b) + b);
      }
      inFrontOf = nextInFrontOf;
    }

    if (otherVec.empty()) {
      return false;
    }

    otherVecIdx ^= 1;
    vecIdx ^= 1;
  }

  ClipVec& accumVec = vecs[otherVecIdx ^ 1];
  for (ClipVec::const_iterator it = accumVec.begin(); it != accumVec.end(); ++it) {
    aabb.AccumulateBounds(*it);
  }

  --gRejectedByClip;
  return true;
}

void CMetroidAreaCollider::ResetInternalCounters() {
  gCalledClip = 0;
  gRejectedByClip = 0;
  gTrianglesProcessed = 0;
  gDupTrianglesProcessed = 0;
  if (sDupPrimitiveCheckCount == 0xffff) {
    memset(sDupVertexList, 0, sizeof(sDupVertexList));
    memset(sDupEdgeList, 0, sizeof(sDupEdgeList));
    memset(sDupTriangleList, 0, sizeof(sDupTriangleList));
    ++sDupPrimitiveCheckCount;
  }
  ++sDupPrimitiveCheckCount;
}

bool CMetroidAreaCollider::AABoxCollisionCheck_Internal(const CAreaOctTree::Node& node,
                                                        CAABoxAreaCache& cache) {
  bool ret = false;

  switch (node.GetTreeType()) {
  case CAreaOctTree::Node::kTT_Invalid:
    return false;
  case CAreaOctTree::Node::kTT_Branch: {
    for (int i = 0; i < 8; ++i) {
      CAreaOctTree::Node ch = node.GetChild(i);
      CAABox box = ch.GetBoundingBox();
      if (box.DoBoundsOverlap(cache.mAabb))
        if (AABoxCollisionCheck_Internal(ch, cache))
          ret = true;
    }
    break;
  }
  case CAreaOctTree::Node::kTT_Leaf: {
    CAreaOctTree::TriListReference list = node.GetTriangleArray();
    int size = list.GetSize();
    const CAreaOctTree& owner = node.GetOwner();
    const CMaterialFilter& filter = cache.mFilter;
    const CPlane* planes = cache.mPlanes;
    for (int j = 0; j < size; ++j) {
      ++gTrianglesProcessed;
      ushort triIdx = list.GetAt(j);
      if (sDupPrimitiveCheckCount == sDupTriangleList[triIdx]) {
        ++gDupTrianglesProcessed;
      } else {
        sDupTriangleList[triIdx] = sDupPrimitiveCheckCount;
        const CCollisionSurface& surf = owner.GetMasterListTriangle(triIdx);
        CMaterialList material(surf.GetSurfaceFlags());
        if (filter.Passes(material)) {
          if (CollisionUtil::TriBoxOverlap(cache.mCenter, cache.mHalfExtent, surf.GetVert(0),
                                           surf.GetVert(1), surf.GetVert(2)) == true) {
            CAABox aabb = CAABox::MakeMaxInvertedBox();
            if (ConvexPolyCollision(planes, &surf.GetVert(0), aabb)) {
              CPlane plane = surf.GetPlane();
              cache.mCollisionList.Add(CCollisionInfo(aabb, cache.mMaterial, material,
                                                         plane.GetNormal(), -plane.GetNormal()),
                                          false);
              ret = true;
            }
          }
        }
      }
    }
    break;
  }
  default:
    break;
  }

  return ret;
}

bool CMetroidAreaCollider::AABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache,
                                                      const CAABox& aabb,
                                                      const CMaterialFilter& filter,
                                                      const CMaterialList& matList,
                                                      CCollisionInfoList& list) {
  const CUnitVector3f right(1.f, 0.f, 0.f);
  const CUnitVector3f forward(0.f, 1.f, 0.f);
  const CUnitVector3f up(0.f, 0.f, 1.f);
  const CVector3f min = aabb.GetMinPoint();
  const CVector3f max = aabb.GetMaxPoint();
  const CPlane planes[6] = {
      CPlane(min, right),   CPlane(max, -right),
      CPlane(min, forward), CPlane(max, -forward),
      CPlane(min, up),      CPlane(max, -up),
  };

  ResetInternalCounters();
  CVector3f center = aabb.GetCenterPoint();
  CVector3f halfExtent = aabb.GetHalfExtent();
  bool ret = false;

  for (int i = 0; i < leafCache.GetNumLeaves(); ++i) {
    const CAreaOctTree::Node& node = leafCache.GetLeaf(i);
    if (aabb.DoBoundsOverlap(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference listRef = node.GetTriangleArray();
      const CAreaOctTree& owner = node.GetOwner();
      int size = listRef.GetSize();
      for (int j = 0; j < size; ++j) {
        ++gTrianglesProcessed;
        ushort triIdx = listRef.GetAt(j);
        if (sDupPrimitiveCheckCount == sDupTriangleList[triIdx]) {
          ++gDupTrianglesProcessed;
        } else {
          sDupTriangleList[triIdx] = sDupPrimitiveCheckCount;
          const CCollisionSurface& surf = owner.GetMasterListTriangle(triIdx);
          CMaterialList material(surf.GetSurfaceFlags());
          if (filter.Passes(material)) {
            if (CollisionUtil::TriBoxOverlap(center, halfExtent, surf.GetVert(0), surf.GetVert(1),
                                             surf.GetVert(2)) == true) {
              CAABox aabb2 = CAABox::MakeMaxInvertedBox();
              if (ConvexPolyCollision(planes, &surf.GetVert(0), aabb2)) {
                CPlane plane = surf.GetPlane();
                list.Add(
                    CCollisionInfo(aabb2, matList, material, plane.GetNormal(), -plane.GetNormal()),
                    false);
                ret = true;
              }
            }
          }
        }
      }
    }
  }

  return ret;
}

CAABoxAreaCache::CAABoxAreaCache(const CAABox& aabb, const CPlane* pl,
                                 const CMaterialFilter& filter, const CMaterialList& material,
                                 CCollisionInfoList& collisionList)
: mAabb(aabb)
, mPlanes(pl)
, mFilter(filter)
, mMaterial(material)
, mCollisionList(collisionList)
, mCenter(aabb.GetCenterPoint())
, mHalfExtent(aabb.GetHalfExtent()) {}

bool CMetroidAreaCollider::AABoxCollisionCheck(const CAreaOctTree& octTree, const CAABox& aabb,
                                               const CMaterialFilter& filter,
                                               const CMaterialList& matList,
                                               CCollisionInfoList& list) {
  const CVector3f min = aabb.GetMinPoint();
  const CVector3f max = aabb.GetMaxPoint();
  const CUnitVector3f xAxis(1.f, 0.f, 0.f);
  const CUnitVector3f yAxis(0.f, 1.f, 0.f);
  const CUnitVector3f zAxis(0.f, 0.f, 1.f);
  CPlane planes[6] = {
      CPlane(min, xAxis),
      CPlane(max, -xAxis),
      CPlane(min, yAxis),
      CPlane(max, -yAxis),
      CPlane(min, zAxis),
      CPlane(max, -zAxis),
  };
  CAABoxAreaCache cache(aabb, planes, filter, matList, list);

  ResetInternalCounters();

  CAreaOctTree::Node node(octTree.GetTreeMemory(), octTree.GetBoundingBox(), octTree,
                          octTree.GetTreeType());
  return AABoxCollisionCheck_Internal(node, cache);
}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean_Internal(
    const CAreaOctTree::Node& node, const CBooleanAABoxAreaCache& cache) {
  for (int i = 0; i < 8; ++i) {
    CAreaOctTree::Node::ETreeType type = node.GetChildType(i);
    if (type != CAreaOctTree::Node::kTT_Invalid) {
      CAreaOctTree::Node ch = node.GetChild(i);
      if (cache.mAabb.DoBoundsOverlap(ch.GetBoundingBox())) {
        if (type == CAreaOctTree::Node::kTT_Leaf) {
          CAreaOctTree::TriListReference list = ch.GetTriangleArray();
          const CAreaOctTree& owner = ch.GetOwner();
          int size = list.GetSize();
          for (int j = 0; j < size; ++j) {
            ++gTrianglesProcessed;
            const CCollisionSurface& surf = owner.GetMasterListTriangle(list.GetAt(j));
            if (cache.mFilter.Passes(CMaterialList(surf.GetSurfaceFlags()))) {
              if (CollisionUtil::TriBoxOverlap(cache.mCenter, cache.mHalfExtent,
                                               surf.GetVert(0), surf.GetVert(1),
                                               surf.GetVert(2)) == true)
                return true;
            }
          }
        } else {
          if (AABoxCollisionCheckBoolean_Internal(ch, cache) == true)
            return true;
        }
      }
    }
  }
  return false;
}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache,
                                                             const CAABox& aabb,
                                                             const CMaterialFilter& filter) {
  CVector3f center = aabb.GetCenterPoint();
  CVector3f halfExtent = aabb.GetHalfExtent();

  for (int i = 0; i < leafCache.GetNumLeaves(); ++i) {
    const CAreaOctTree::Node& node = leafCache.GetLeaf(i);
    if (aabb.DoBoundsOverlap(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      const CAreaOctTree& owner = node.GetOwner();
      int size = list.GetSize();
      for (int j = 0; j < size; ++j) {
        ++gTrianglesProcessed;
        const CCollisionSurface& surf = owner.GetMasterListTriangle(list.GetAt(j));
        if (filter.Passes(CMaterialList(surf.GetSurfaceFlags()))) {
          if (CollisionUtil::TriBoxOverlap(center, halfExtent, surf.GetVert(0), surf.GetVert(1),
                                           surf.GetVert(2)) == true)
            return true;
        }
      }
    }
  }

  return false;
}

CBooleanAABoxAreaCache::CBooleanAABoxAreaCache(const CAABox& aabb, const CMaterialFilter& filter)
: mAabb(aabb)
, mFilter(filter)
, mCenter(aabb.GetCenterPoint())
, mHalfExtent(aabb.GetHalfExtent()) {}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean(const CAreaOctTree& octTree,
                                                      const CAABox& aabb,
                                                      const CMaterialFilter& filter) {
  CBooleanAABoxAreaCache cache(aabb, filter);
  CAreaOctTree::Node node(octTree.GetTreeMemory(), octTree.GetBoundingBox(), octTree,
                          octTree.GetTreeType());
  return AABoxCollisionCheckBoolean_Internal(node, cache);
}

bool CMetroidAreaCollider::SphereCollisionCheck_Internal(const CAreaOctTree::Node& node,
                                                         CSphereAreaCache& cache) {
  bool ret = false;
  CVector3f point = CVector3f::Zero();
  CVector3f normal = CVector3f::Zero();

  for (int i = 0; i < 8; ++i) {
    CAreaOctTree::Node::ETreeType chTp = node.GetChildType(i);
    if (chTp != CAreaOctTree::Node::kTT_Invalid) {
      CAreaOctTree::Node ch = node.GetChild(i);
      if (cache.mAabb.DoBoundsOverlap(ch.GetBoundingBox())) {
        if (chTp == CAreaOctTree::Node::kTT_Leaf) {
          CAreaOctTree::TriListReference list = ch.GetTriangleArray();
          const CAreaOctTree& owner = ch.GetOwner();
          int size = list.GetSize();
          for (int j = 0; j < size; ++j) {
            ++gTrianglesProcessed;
            ushort triIdx = list.GetAt(j);
            if (sDupPrimitiveCheckCount == sDupTriangleList[triIdx]) {
              ++gDupTrianglesProcessed;
            } else {
              sDupTriangleList[triIdx] = sDupPrimitiveCheckCount;
              const CCollisionSurface& surf = owner.GetMasterListTriangle(triIdx);
              CMaterialList material(surf.GetSurfaceFlags());
              if (cache.mFilter.Passes(material)) {
                if (CollisionUtil::TriSphereIntersection(cache.mSphere, surf.GetVert(0),
                                                         surf.GetVert(1), surf.GetVert(2), point,
                                                         normal)) {
                  cache.mCollisionList.Add(
                      CCollisionInfo(point, cache.mMaterial, material, normal), false);
                  ret = true;
                }
              }
            }
          }
        } else {
          if (SphereCollisionCheck_Internal(ch, cache) == true)
            ret = true;
        }
      }
    }
  }

  return ret;
}

bool CMetroidAreaCollider::SphereCollisionCheck_Cached(const COctreeLeafCache& leafCache,
                                                       const CAABox& aabb, const CSphere& sphere,
                                                       const CMaterialList& matList,
                                                       const CMaterialFilter& filter,
                                                       CCollisionInfoList& clist) {
  ResetInternalCounters();

  bool ret = false;
  CVector3f point = CVector3f::Zero();
  CVector3f normal = CVector3f::Zero();

  for (int i = 0; i < leafCache.GetNumLeaves(); ++i) {
    const CAreaOctTree::Node& node = leafCache.GetLeaf(i);
    if (aabb.DoBoundsOverlap(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      const CAreaOctTree& owner = node.GetOwner();
      int size = list.GetSize();
      for (int j = 0; j < size; ++j) {
        ++gTrianglesProcessed;
        ushort triIdx = list.GetAt(j);
        if (sDupPrimitiveCheckCount == sDupTriangleList[triIdx]) {
          ++gDupTrianglesProcessed;
        } else {
          sDupTriangleList[triIdx] = sDupPrimitiveCheckCount;
          const CCollisionSurface& surf = owner.GetMasterListTriangle(triIdx);
          CMaterialList material(surf.GetSurfaceFlags());
          if (filter.Passes(material)) {
            if (CollisionUtil::TriSphereIntersection(sphere, surf.GetVert(0), surf.GetVert(1),
                                                     surf.GetVert(2), point, normal)) {
              clist.Add(CCollisionInfo(point, matList, material, normal), false);
              ret = true;
            }
          }
        }
      }
    }
  }

  return ret;
}

bool CMetroidAreaCollider::SphereCollisionCheck(const CAreaOctTree& octTree, const CAABox& aabb,
                                                const CSphere& sphere, const CMaterialList& matList,
                                                const CMaterialFilter& filter,
                                                CCollisionInfoList& list) {
  CSphereAreaCache cache(aabb, sphere, filter, matList, list);
  ResetInternalCounters();
  CAreaOctTree::Node node(octTree.GetTreeMemory(), octTree.GetBoundingBox(), octTree,
                          octTree.GetTreeType());
  return SphereCollisionCheck_Internal(node, cache);
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean_Internal(
    const CAreaOctTree::Node& node, const CBooleanSphereAreaCache& cache) {
  for (int i = 0; i < 8; ++i) {
    CAreaOctTree::Node::ETreeType type = node.GetChildType(i);
    if (type != CAreaOctTree::Node::kTT_Invalid) {
      CAreaOctTree::Node ch = node.GetChild(i);
      if (cache.mAabb.DoBoundsOverlap(ch.GetBoundingBox())) {
        if (type == CAreaOctTree::Node::kTT_Leaf) {
          CAreaOctTree::TriListReference list = ch.GetTriangleArray();
          const CAreaOctTree& owner = ch.GetOwner();
          int size = list.GetSize();
          for (int j = 0; j < size; ++j) {
            ++gTrianglesProcessed;
            const CCollisionSurface& surf = owner.GetMasterListTriangle(list.GetAt(j));
            if (cache.mFilter.Passes(CMaterialList(surf.GetSurfaceFlags()))) {
              if (CollisionUtil::TriSphereOverlap(cache.mSphere, surf.GetVert(0), surf.GetVert(1),
                                                  surf.GetVert(2)) == true)
                return true;
            }
          }
        } else {
          if (SphereCollisionCheckBoolean_Internal(ch, cache) == true)
            return true;
        }
      }
    }
  }
  return false;
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache,
                                                              const CAABox& aabb,
                                                              const CSphere& sphere,
                                                              const CMaterialFilter& filter) {
  for (int i = 0; i < leafCache.GetNumLeaves(); ++i) {
    const CAreaOctTree::Node& node = leafCache.GetLeaf(i);
    if (aabb.DoBoundsOverlap(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      const CAreaOctTree& owner = node.GetOwner();
      int size = list.GetSize();
      for (int j = 0; j < size; ++j) {
        ++gTrianglesProcessed;
        const CCollisionSurface& surf = owner.GetMasterListTriangle(list.GetAt(j));
        if (filter.Passes(CMaterialList(surf.GetSurfaceFlags()))) {
          if (CollisionUtil::TriSphereOverlap(sphere, surf.GetVert(0), surf.GetVert(1),
                                              surf.GetVert(2)) == true)
            return true;
        }
      }
    }
  }

  return false;
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean(const CAreaOctTree& octTree,
                                                       const CAABox& aabb, const CSphere& sphere,
                                                       const CMaterialFilter& filter) {
  CBooleanSphereAreaCache cache(aabb, sphere, filter);
  CAreaOctTree::Node node(octTree.GetTreeMemory(), octTree.GetBoundingBox(), octTree,
                          octTree.GetTreeType());
  return SphereCollisionCheckBoolean_Internal(node, cache);
}

static inline CVector3f TriangleEdgeNormal(const CVector3f& lhs, const CVector3f& rhs) {
  const float lX = lhs.GetX();
  const float lY = lhs.GetY();
  const float lZ = lhs.GetZ();
  const float rX = rhs.GetX();
  const float rY = rhs.GetY();
  const float rZ = rhs.GetZ();

  return CVector3f(lY * rZ - rY * lZ, lZ * rX - rZ * lX, lX * rY - rX * lY);
}

bool CMetroidAreaCollider::MovingSphereCollisionCheck_Cached(
    const COctreeLeafCache& leafCache, const CAABox& aabb, const CSphere& sphere,
    const CMaterialFilter& filter, const CMaterialList& matList, CVector3f dir, float mag,
    CCollisionInfo& infoOut, double& dOut) {
  dOut = mag;
  ResetInternalCounters();

  CVector3f moveVec = mag * dir;
  CAABox movedAABB = aabb;
  movedAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);
  movedAABB.AccumulateBounds(aabb.GetMaxPoint() + moveVec);

  CVector3f center = movedAABB.GetCenterPoint();
  CVector3f extent = movedAABB.GetHalfExtent();
  bool ret = false;

  for (int i = 0; i < leafCache.GetNumLeaves(); ++i) {
    const CAreaOctTree::Node& node = leafCache.GetLeaf(i);
    if (movedAABB.DoBoundsOverlap(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      const CAreaOctTree& owner = node.GetOwner();
      int listSize = list.GetSize();
      for (int j = 0; j < listSize; ++j) {
        ushort triIdx = list.GetAt(j);
        if (sDupPrimitiveCheckCount != sDupTriangleList[triIdx]) {
          sDupTriangleList[triIdx] = sDupPrimitiveCheckCount;
          ++gTrianglesProcessed;
          uint matValue = owner.GetTriangleMaterial(triIdx);
          CMaterialList triMat(matValue);
          if (filter.Passes(triMat)) {
            ushort vertIndices[3];
            owner.GetTriangleVertexIndices(triIdx, vertIndices);
            CCollisionSurface surf(owner.GetVert(vertIndices[0]), owner.GetVert(vertIndices[1]),
                                   owner.GetVert(vertIndices[2]), matValue);

            if (CollisionUtil::TriBoxOverlap(center, extent, surf.GetVert(0), surf.GetVert(1),
                                             surf.GetVert(2)) == true) {
              CVector3f surfNormal = surf.GetNormal();
              CVector3f endDelta = sphere.GetCenter() + moveVec - surf.GetVert(0);
              float endHeight = CVector3f::Dot(endDelta, surfNormal);
              if (!(endHeight > sphere.GetRadius())) {
                bool triRet = false;

                double triMagD =
                    sphere.GetRadius() -
                    CVector3f::Dot(sphere.GetCenter() - surf.GetVert(0), surfNormal);
                triMagD /= CVector3f::Dot(dir, surfNormal);
                CVector3f intersectPoint = sphere.GetCenter() + static_cast< float >(triMagD) * dir;

                bool outsideEdges[3];
                outsideEdges[0] =
                    CVector3f::Dot(
                        intersectPoint - surf.GetVert(0),
                        TriangleEdgeNormal(surfNormal, surf.GetVert(1) - surf.GetVert(0))) < 0.f;
                outsideEdges[1] =
                    CVector3f::Dot(
                        intersectPoint - surf.GetVert(1),
                        TriangleEdgeNormal(surfNormal, surf.GetVert(2) - surf.GetVert(1))) < 0.f;
                outsideEdges[2] =
                    CVector3f::Dot(
                        intersectPoint - surf.GetVert(2),
                        TriangleEdgeNormal(surfNormal, surf.GetVert(0) - surf.GetVert(2))) < 0.f;

                if (triMagD >= 0.0 && !outsideEdges[0] && !outsideEdges[1] && !outsideEdges[2] &&
                    triMagD < dOut) {
                  triRet = true;
                  infoOut = CCollisionInfo(intersectPoint - sphere.GetRadius() * surfNormal,
                                           matList, triMat, surfNormal);
                  dOut = triMagD;
                  ret = true;
                }

                bool intersects = CVector3f::Dot(sphere.GetCenter() - surf.GetVert(0),
                                                 surfNormal) <= sphere.GetRadius();
                bool testVert[3] = {true, true, true};
                const ushort* edgeIndices = owner.GetTriangleEdgeIndices(triIdx);
                for (int k = 0; k < 3; ++k) {
                  if (intersects || outsideEdges[k]) {
                    int edgeIdx = edgeIndices[k];
                    if (sDupPrimitiveCheckCount != sDupEdgeList[edgeIdx]) {
                      sDupEdgeList[edgeIdx] = sDupPrimitiveCheckCount;
                      uint edgeMatVal = owner.GetEdgeMaterial(edgeIdx);
                      if (!(edgeMatVal & (1u << kMT_NoEdgeCollision))) {
                        static int mod3[4] = {0, 1, 2, 0};
                        CVector3f edgeVec = surf.GetVert(mod3[k + 1]) - surf.GetVert(k);
                        float edgeVecMag = edgeVec.Magnitude();
                        edgeVec *= 1.f / edgeVecMag;

                        CVector3f vertToSphere = sphere.GetCenter() - surf.GetVert(k);
                        float vtsDotEdge = CVector3f::Dot(vertToSphere, edgeVec);
                        CVector3f vtsRej = vertToSphere - vtsDotEdge * edgeVec;
                        float dirDotEdge = CVector3f::Dot(dir, edgeVec);
                        CVector3f edgeRej = dir - dirDotEdge * edgeVec;
                        float edgeRejMagSq = edgeRej.MagSquared();

                        if (edgeRejMagSq > 0.f) {
                          float tmp = 2.f * CVector3f::Dot(vtsRej, edgeRej);
                          float tmp2 =
                              tmp * tmp -
                              4.f * edgeRejMagSq *
                                  (vtsRej.MagSquared() - sphere.GetRadius() * sphere.GetRadius());
                          if (tmp2 >= 0.f) {
                            double invDenom = 0.5 / edgeRejMagSq;
                            double eMag = invDenom * (-tmp - sqrt(tmp2));
                            if (eMag >= 0.0) {
                              double t = eMag * dirDotEdge + vtsDotEdge;
                              if (t >= 0.0 && t <= edgeVecMag && eMag < dOut) {
                                triRet = true;
                                CVector3f ePoint =
                                    surf.GetVert(k) + static_cast< float >(t) * edgeVec;
                                CVector3f eNormal =
                                    (sphere.GetCenter() + static_cast< float >(eMag) * dir - ePoint)
                                        .AsNormalized();
                                infoOut = CCollisionInfo(ePoint, matList, CMaterialList(edgeMatVal),
                                                         eNormal);
                                dOut = eMag;
                                ret = true;
                                testVert[k] = false;
                                testVert[mod3[k + 1]] = false;
                              } else if (t < -sphere.GetRadius() && dirDotEdge <= 0.f) {
                                testVert[k] = false;
                              } else if (t > edgeVecMag + sphere.GetRadius() && dirDotEdge >= 0.f) {
                                testVert[mod3[k + 1]] = false;
                              }
                            }
                          } else {
                            testVert[k] = false;
                            testVert[mod3[k + 1]] = false;
                          }
                        }
                      }
                    }
                  }
                }

                for (int k = 0; k < 3; ++k) {
                  int vertIdx = vertIndices[k];
                  if (testVert[k]) {
                    if (sDupPrimitiveCheckCount != sDupVertexList[vertIdx]) {
                      sDupVertexList[vertIdx] = sDupPrimitiveCheckCount;
                      double d = dOut;
                      if (CollisionUtil::RaySphereIntersection_Double(
                              CSphere(surf.GetVert(k), sphere.GetRadius()), sphere.GetCenter(), dir,
                              d) &&
                          d >= 0.0) {
                        triRet = true;
                        CVector3f vNormal =
                            (sphere.GetCenter() + dir * static_cast< float >(d) - surf.GetVert(k))
                                .AsNormalized();
                        infoOut = CCollisionInfo(surf.GetVert(k), matList,
                                                 CMaterialList(owner.GetVertMaterial(vertIdx)),
                                                 vNormal);
                        dOut = d;
                        ret = true;
                      }
                    }
                  } else {
                    sDupVertexList[vertIdx] = sDupPrimitiveCheckCount;
                  }
                }

                if (triRet) {
                  moveVec = static_cast< float >(dOut) * dir;
                  movedAABB = aabb;
                  movedAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);
                  movedAABB.AccumulateBounds(aabb.GetMaxPoint() + moveVec);
                  center = movedAABB.GetCenterPoint();
                  extent = movedAABB.GetHalfExtent();
                }
              }
            } else {
              const ushort* edgeIndices = owner.GetTriangleEdgeIndices(triIdx);
              sDupEdgeList[edgeIndices[0]] = sDupPrimitiveCheckCount;
              sDupEdgeList[edgeIndices[1]] = sDupPrimitiveCheckCount;
              sDupEdgeList[edgeIndices[2]] = sDupPrimitiveCheckCount;
              sDupVertexList[vertIndices[0]] = sDupPrimitiveCheckCount;
              sDupVertexList[vertIndices[1]] = sDupPrimitiveCheckCount;
              sDupVertexList[vertIndices[2]] = sDupPrimitiveCheckCount;
            }
          }
        }
      }
    }
  }

  return ret;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_Cached(
    const COctreeLeafCache& leafCache, const CAABox& aabb, const CMaterialFilter& filter,
    const CMaterialList& matList, CVector3f dir, float mag, CCollisionInfo& infoOut, double& dOut) {
  dOut = mag;
  ResetInternalCounters();

  CVector3f moveVec = mag * dir;
  CMovingAABoxComponents components(aabb, dir);

  CAABox movedAABB = components.mAabb;
  movedAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);
  movedAABB.AccumulateBounds(aabb.GetMaxPoint() + moveVec);

  CVector3f center = movedAABB.GetCenterPoint();
  CVector3f extent = movedAABB.GetHalfExtent();
  bool ret = false;

  CVector3f normal(CVector3f::Zero());
  CVector3f point(CVector3f::Zero());

  for (int i = 0; i < leafCache.GetNumLeaves(); ++i) {
    const CAreaOctTree::Node& node = leafCache.GetLeaf(i);
    if (movedAABB.DoBoundsOverlap(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      const CAreaOctTree& owner = node.GetOwner();
      int listSize = list.GetSize();
      for (int j = 0; j < listSize; ++j) {
        ushort triIdx = list.GetAt(j);
        if (sDupPrimitiveCheckCount != sDupTriangleList[triIdx]) {
          sDupTriangleList[triIdx] = sDupPrimitiveCheckCount;
          ++gTrianglesProcessed;
          uint matValue = owner.GetTriangleMaterial(triIdx);
          CMaterialList triMat(matValue);
          if (filter.Passes(triMat)) {
            ushort vertIndices[3];
            owner.GetTriangleVertexIndices(triIdx, vertIndices);
            CCollisionSurface surf(owner.GetVert(vertIndices[0]), owner.GetVert(vertIndices[1]),
                                   owner.GetVert(vertIndices[2]), matValue);

            if (CollisionUtil::TriBoxOverlap(center, extent, surf.GetVert(0), surf.GetVert(1),
                                             surf.GetVert(2)) == true) {
              bool triRet = false;
              double d = dOut;
              if (MovingAABoxCollisionCheck_BoxVertexTri(surf, aabb, components.mVertIdxs, dir,
                                                         d, normal, point) &&
                  d < dOut) {
                triRet = true;
                infoOut = CCollisionInfo(point, matList, triMat, normal);
                ret = true;
                dOut = d;
              }

              for (int k = 0; k < 3; ++k) {
                int vertIdx = vertIndices[k];
                if (sDupPrimitiveCheckCount != sDupVertexList[vertIdx]) {
                  sDupVertexList[vertIdx] = sDupPrimitiveCheckCount;
                  const CVector3f& vtx = owner.GetVert(vertIdx);
                  if (movedAABB.PointInside(vtx)) {
                    d = dOut;
                    if (MovingAABoxCollisionCheck_TriVertexBox(vtx, aabb, dir, d, normal, point) &&
                        d < dOut) {
                      CMaterialList vertMat(owner.GetVertMaterial(vertIdx));
                      triRet = true;
                      infoOut = CCollisionInfo(point, matList, vertMat, normal);
                      ret = true;
                      dOut = d;
                    }
                  }
                }
              }

              const ushort* edgeIndices = owner.GetTriangleEdgeIndices(triIdx);
              for (int k = 0; k < 3; ++k) {
                int edgeIdx = edgeIndices[k];
                if (sDupPrimitiveCheckCount != sDupEdgeList[edgeIdx]) {
                  sDupEdgeList[edgeIdx] = sDupPrimitiveCheckCount;
                  uint edgeMat = owner.GetEdgeMaterial(edgeIdx);
                  if (!(edgeMat & (1u << kMT_NoEdgeCollision))) {
                    d = dOut;
                    const CCollisionEdge& edge = owner.GetEdge(edgeIdx);
                    if (MovingAABoxCollisionCheck_Edge(owner.GetVert(edge.GetVertIndex1()),
                                                       owner.GetVert(edge.GetVertIndex2()),
                                                       components.mEdges, dir, d, normal,
                                                       point) &&
                        d < dOut) {
                      triRet = true;
                      infoOut = CCollisionInfo(point, matList, CMaterialList(edgeMat), normal);
                      ret = true;
                      dOut = d;
                    }
                  }
                }
              }

              if (triRet) {
                moveVec = static_cast< float >(dOut) * dir;
                movedAABB = components.mAabb;
                movedAABB.AccumulateBounds(aabb.GetMinPoint() + moveVec);
                movedAABB.AccumulateBounds(aabb.GetMaxPoint() + moveVec);
                center = movedAABB.GetCenterPoint();
                extent = movedAABB.GetHalfExtent();
              }
            } else {
              const ushort* edgeIndices = owner.GetTriangleEdgeIndices(triIdx);
              sDupEdgeList[edgeIndices[0]] = sDupPrimitiveCheckCount;
              sDupEdgeList[edgeIndices[1]] = sDupPrimitiveCheckCount;
              sDupEdgeList[edgeIndices[2]] = sDupPrimitiveCheckCount;
              sDupVertexList[vertIndices[0]] = sDupPrimitiveCheckCount;
              sDupVertexList[vertIndices[1]] = sDupPrimitiveCheckCount;
              sDupVertexList[vertIndices[2]] = sDupPrimitiveCheckCount;
            }
          }
        }
      }
    }
  }

  return ret;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_TriVertexBox(const CVector3f& vert,
                                                                  const CAABox& aabb, CVector3f dir,
                                                                  double& dOut, CVector3f& normal,
                                                                  CVector3f& point) {
  bool ret = false;
  float rayLen = static_cast< float >(dOut);
  CMRay ray(vert, -dir, rayLen);
  CVector3f norm(CVector3f::Zero());
  double d;
  if (CollisionUtil::RayAABoxIntersection_Double(ray, aabb, norm, d) == 2) {
    double nd = d * dOut;
    if (nd < dOut) {
      ret = true;
      normal = -norm;
      dOut = nd;
      point = vert;
    }
  }
  return ret;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_BoxVertexTri(
    const CCollisionSurface& surf, const CAABox& aabb,
    const rstl::reserved_vector< uint, 8 >& vertIndices, CVector3f dir, double& d,
    CVector3f& normalOut, CVector3f& pointOut) {
  bool ret = false;
  for (int i = 0; i < vertIndices.size(); ++i) {
    CVector3f point = aabb.GetPoint(vertIndices[i]);
    if (CollisionUtil::RayTriangleIntersection_Double(point, dir, &surf.GetVert(0), d)) {
      pointOut = point + dir * static_cast< float >(d);
      normalOut = surf.GetNormal();
      ret = true;
    }
  }
  return ret;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_Edge(
    const CVector3f& ev0, const CVector3f& ev1, const rstl::reserved_vector< SBoxEdge, 12 >& edges,
    CVector3f dir, double& d, CVector3f& normal, CVector3f& point) {
  bool ret = false;

  for (int i = 0; i < edges.size(); ++i) {
    const SBoxEdge& edge = edges[i];
    CVector3d ev0d = ev0;
    CVector3d ev1d = ev1;
    if ((CVector3d::Dot(edge.mCoDir, ev0d) >= edge.mDirCoDirDot) ==
        (CVector3d::Dot(edge.mCoDir, ev1d) >= edge.mDirCoDirDot))
      continue;

    CVector3d delta = ev0d - ev1d;
    CVector3d cross0 = CVector3d::Cross(edge.mDelta, delta);
    if (cross0.MagSquared() < FLT_EPSILON)
      continue;

    CVector3d cross0Norm = cross0.AsNormalized();
    if (CVector3d::Dot(cross0Norm, dir) >= 0.0) {
      ev1d = ev0;
      ev0d = ev1;
      delta = ev0d - ev1d;
      cross0 = CVector3d::Cross(edge.mDelta, delta);
      cross0Norm = cross0.AsNormalized();
    }

    CVector3d clipped = ev0d + (-(CVector3d::Dot(ev0d, edge.mCoDir) - edge.mDirCoDirDot) /
                                CVector3d::Dot(delta, edge.mCoDir)) *
                                   delta;
    int maxCompIdx;
    if (CMath::AbsD(edge.mCoDir.GetX()) > CMath::AbsD(edge.mCoDir.GetY()))
      maxCompIdx = 0;
    else
      maxCompIdx = 1;
    if (CMath::AbsD(edge.mCoDir[maxCompIdx]) < CMath::AbsD(edge.mCoDir.GetZ()))
      maxCompIdx = 2;

    int ci0, ci1;
    if (maxCompIdx == 0) {
      ci0 = 1;
      ci1 = 2;
    } else if (maxCompIdx == 1) {
      ci0 = 0;
      ci1 = 2;
    } else {
      ci0 = 0;
      ci1 = 1;
    }

    const float& dir0 = dir[ci0];
    const float& dir1 = dir[ci1];
    const double& edgeDelta0 = edge.mDelta[ci0];
    const double& edgeDelta1 = edge.mDelta[ci1];
    const double denominator = edgeDelta0 * dir1 - edgeDelta1 * dir0;
    double eMag = (edge.mDelta[ci0] * (clipped[ci1] - edge.mStart[ci1]) -
                   edge.mDelta[ci1] * (clipped[ci0] - edge.mStart[ci0])) /
                  denominator;

    if (!(eMag < 0.0) && !(eMag >= d)) {
      CVector3d clippedMag = clipped - eMag * CVector3d(dir);
      double dotCheck =
          (edge.mStart.GetX() - clippedMag.GetX()) * (edge.mEnd.GetX() - clippedMag.GetX()) +
          (edge.mStart.GetY() - clippedMag.GetY()) * (edge.mEnd.GetY() - clippedMag.GetY()) +
          (edge.mStart.GetZ() - clippedMag.GetZ()) * (edge.mEnd.GetZ() - clippedMag.GetZ());
      if (dotCheck < 0.0 && eMag < d) {
        normal = cross0Norm.AsCVector3f();
        d = eMag;
        point = clipped.AsCVector3f();
        ret = true;
      }
    }
  }

  return ret;
}

CMetroidAreaCollider::COctreeLeafCache::COctreeLeafCache(const CAreaOctTree& octTree)
: mOctTree(octTree), mOverflow(false) {}

void CMetroidAreaCollider::COctreeLeafCache::AddLeaf(const CAreaOctTree::Node& node) {
  if (mNodeCache.size() == mNodeCache.capacity()) {
    mOverflow = true;
    return;
  }
  mNodeCache.push_back(node);
}

CAreaCollisionCache::CAreaCollisionCache(const CAABox& aabb)
: mAabb(aabb), mLeafOverflow(false), mCacheOverflow(false) {}

void CAreaCollisionCache::AddOctreeLeafCache(
    const CMetroidAreaCollider::COctreeLeafCache& leafCache) {
  if (!leafCache.GetNumLeaves())
    return;

  if (leafCache.HasCacheOverflowed())
    mLeafOverflow = true;

  if (mLeafCaches.size() < 3) {
    mLeafCaches.push_back(leafCache);
  } else {
    mLeafOverflow = true;
    mCacheOverflow = true;
  }
}

void CAreaCollisionCache::SetCacheBounds(const CAABox& aabb) { mAabb = aabb; }

void CAreaCollisionCache::ClearCache() {
  mLeafCaches.clear();
  mLeafOverflow = false;
  mCacheOverflow = false;
}

void CMetroidAreaCollider::BuildOctreeLeafCache(const CAreaOctTree::Node& node, const CAABox& aabb,
                                                COctreeLeafCache& cache) {
  for (int i = 0; i < 8; ++i) {
    CAreaOctTree::Node::ETreeType type = node.GetChildType(i);
    if (type != CAreaOctTree::Node::kTT_Invalid) {
      CAreaOctTree::Node ch = node.GetChild(i);
      if (aabb.DoBoundsOverlap(ch.GetBoundingBox())) {
        if (type == CAreaOctTree::Node::kTT_Leaf)
          cache.AddLeaf(ch);
        else
          BuildOctreeLeafCache(ch, aabb, cache);
      }
    }
  }
}

static void FlagEdgeIndicesForFace(uint face, bool* edgeFlags) {
  switch (face) {
  case 0:
    edgeFlags[10] = true;
    edgeFlags[11] = true;
    edgeFlags[2] = true;
    edgeFlags[4] = true;
    return;
  case 1:
    edgeFlags[8] = true;
    edgeFlags[9] = true;
    edgeFlags[0] = true;
    edgeFlags[6] = true;
    return;
  case 2:
    edgeFlags[4] = true;
    edgeFlags[5] = true;
    edgeFlags[6] = true;
    edgeFlags[7] = true;
    return;
  case 3:
    edgeFlags[0] = true;
    edgeFlags[1] = true;
    edgeFlags[2] = true;
    edgeFlags[3] = true;
    return;
  case 4:
    edgeFlags[7] = true;
    edgeFlags[8] = true;
    edgeFlags[3] = true;
    edgeFlags[11] = true;
    return;
  case 5:
    edgeFlags[1] = true;
    edgeFlags[5] = true;
    edgeFlags[9] = true;
    edgeFlags[10] = true;
    return;
  default:
    break;
  }
}

static void FlagVertexIndicesForFace(uint face, bool* vertFlags) {
  switch (face) {
  case 0:
    vertFlags[1] = true;
    vertFlags[3] = true;
    vertFlags[5] = true;
    vertFlags[7] = true;
    return;
  case 1:
    vertFlags[0] = true;
    vertFlags[2] = true;
    vertFlags[4] = true;
    vertFlags[6] = true;
    return;
  case 2:
    vertFlags[2] = true;
    vertFlags[3] = true;
    vertFlags[6] = true;
    vertFlags[7] = true;
    return;
  case 3:
    vertFlags[0] = true;
    vertFlags[1] = true;
    vertFlags[4] = true;
    vertFlags[5] = true;
    return;
  case 4:
    vertFlags[4] = true;
    vertFlags[5] = true;
    vertFlags[6] = true;
    vertFlags[7] = true;
    return;
  case 5:
    vertFlags[0] = true;
    vertFlags[1] = true;
    vertFlags[2] = true;
    vertFlags[3] = true;
    return;
  default:
    break;
  }
}

CMetroidAreaCollider::SBoxEdge::SBoxEdge(const CAABox& aabb, int idx, const CVector3f& dir)
: mSeg(aabb.GetEdge(static_cast< CAABox::EBoxEdgeId >(idx)))
, mStart(mSeg.GetRefPoint())
, mEnd(mSeg.GetEndPoint())
, mDelta(mEnd - mStart)
, mCoDir(CVector3d::Cross(mDelta, CVector3d(dir)).AsNormalized())
, mDirCoDirDot(CVector3d::Dot(mStart, mCoDir)) {}

CMetroidAreaCollider::CMovingAABoxComponents::CMovingAABoxComponents(const CAABox& aabb,
                                                                     const CVector3f& dir)
: mAabb(aabb) {
  bool edgeFlags[12] = {};
  bool vertFlags[8] = {};
  uint useFaces = 0;

  for (int i = 0; i < 3; ++i) {
    if (dir[i] != 0.f) {
      uint face = i * 2 + (dir[i] < 0.f);
      FlagEdgeIndicesForFace(face, edgeFlags);
      FlagVertexIndicesForFace(face, vertFlags);
      useFaces += 1;
    }
  }

  for (int i = 0; i < 12; ++i) {
    if (edgeFlags[i]) {
      mEdges.push_back(SBoxEdge(aabb, i, dir));
    }
  }

  for (int i = 0; i < 8; ++i) {
    if (vertFlags[i]) {
      mVertIdxs.push_back(i);
    }
  }

  if (useFaces == 1) {
    mAabb = CAABox::MakeMaxInvertedBox();
    mAabb.AccumulateBounds(aabb.GetPoint(mVertIdxs[0]));
    mAabb.AccumulateBounds(aabb.GetPoint(mVertIdxs[1]));
    mAabb.AccumulateBounds(aabb.GetPoint(mVertIdxs[2]));
    mAabb.AccumulateBounds(aabb.GetPoint(mVertIdxs[3]));
  }
}
