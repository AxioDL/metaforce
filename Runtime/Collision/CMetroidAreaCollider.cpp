#include "Runtime/Collision/CMetroidAreaCollider.hpp"

#include "Runtime/Collision/CCollisionInfoList.hpp"
#include "Runtime/Collision/CMaterialFilter.hpp"
#include "Runtime/Collision/CollisionUtil.hpp"

namespace urde {

u32 CMetroidAreaCollider::g_CalledClip = 0;
u32 CMetroidAreaCollider::g_RejectedByClip = 0;
u32 CMetroidAreaCollider::g_TrianglesProcessed = 0;
u32 CMetroidAreaCollider::g_DupTrianglesProcessed = 0;
u16 CMetroidAreaCollider::g_DupPrimitiveCheckCount = 0;
std::array<u16, 0x5000> CMetroidAreaCollider::g_DupVertexList{};
std::array<u16, 0xC000> CMetroidAreaCollider::g_DupEdgeList{};
std::array<u16, 0x4000> CMetroidAreaCollider::g_DupTriangleList{};

CAABoxAreaCache::CAABoxAreaCache(const zeus::CAABox& aabb, const std::array<zeus::CPlane, 6>& pl,
                                 const CMaterialFilter& filter, const CMaterialList& material,
                                 CCollisionInfoList& collisionList)
: x0_aabb(aabb)
, x4_planes(pl)
, x8_filter(filter)
, xc_material(material)
, x10_collisionList(collisionList)
, x14_center(aabb.center())
, x20_halfExtent(aabb.extents()) {}

CBooleanAABoxAreaCache::CBooleanAABoxAreaCache(const zeus::CAABox& aabb, const CMaterialFilter& filter)
: x0_aabb(aabb), x4_filter(filter), x8_center(aabb.center()), x14_halfExtent(aabb.extents()) {}

CSphereAreaCache::CSphereAreaCache(const zeus::CAABox& aabb, const zeus::CSphere& sphere, const CMaterialFilter& filter,
                                   const CMaterialList& material, CCollisionInfoList& collisionList)
: x0_aabb(aabb), x4_sphere(sphere), x8_filter(filter), xc_material(material), x10_collisionList(collisionList) {}

CBooleanSphereAreaCache::CBooleanSphereAreaCache(const zeus::CAABox& aabb, const zeus::CSphere& sphere,
                                                 const CMaterialFilter& filter)
: x0_aabb(aabb), x4_sphere(sphere), x8_filter(filter) {}

SBoxEdge::SBoxEdge(const zeus::CAABox& aabb, int idx, const zeus::CVector3f& dir)
: x0_seg(aabb.getEdge(zeus::CAABox::EBoxEdgeId(idx)))
, x28_start(x0_seg.x0_start)
, x40_end(x0_seg.x18_end)
, x58_delta(x40_end - x28_start)
, x70_coDir(x58_delta.cross(dir).asNormalized())
, x88_dirCoDirDot(x28_start.dot(x70_coDir)) {}

static void FlagEdgeIndicesForFace(int face, std::array<bool, 12>& edgeFlags) {
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

static void FlagVertexIndicesForFace(int face, std::array<bool, 8>& vertFlags) {
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

CMovingAABoxComponents::CMovingAABoxComponents(const zeus::CAABox& aabb, const zeus::CVector3f& dir) : x6e8_aabb(aabb) {
  std::array<bool, 12> edgeFlags{};
  std::array<bool, 8> vertFlags{};
  int useFaces = 0;

  for (int i = 0; i < 3; ++i) {
    if (dir[i] != 0.f) {
      const int face = i * 2 + (dir[i] < 0.f);
      FlagEdgeIndicesForFace(face, edgeFlags);
      FlagVertexIndicesForFace(face, vertFlags);
      useFaces += 1;
    }
  }

  for (size_t i = 0; i < edgeFlags.size(); ++i) {
    if (edgeFlags[i]) {
      x0_edges.emplace_back(aabb, s32(i), dir);
    }
  }

  for (size_t i = 0; i < vertFlags.size(); ++i) {
    if (vertFlags[i]) {
      x6c4_vertIdxs.push_back(u32(i));
    }
  }

  if (useFaces == 1) {
    x6e8_aabb = zeus::CAABox();
    x6e8_aabb.accumulateBounds(aabb.getPoint(x6c4_vertIdxs[0]));
    x6e8_aabb.accumulateBounds(aabb.getPoint(x6c4_vertIdxs[1]));
    x6e8_aabb.accumulateBounds(aabb.getPoint(x6c4_vertIdxs[2]));
    x6e8_aabb.accumulateBounds(aabb.getPoint(x6c4_vertIdxs[3]));
  }
}

CMetroidAreaCollider::COctreeLeafCache::COctreeLeafCache(const CAreaOctTree& octTree) : x0_octTree(octTree) {}

void CMetroidAreaCollider::COctreeLeafCache::AddLeaf(const CAreaOctTree::Node& node) {
  if (x4_nodeCache.size() == x4_nodeCache.capacity()) {
    x908_24_overflow = true;
    return;
  }

  x4_nodeCache.push_back(node);
}

void CMetroidAreaCollider::BuildOctreeLeafCache(const CAreaOctTree::Node& node, const zeus::CAABox& aabb,
                                                CMetroidAreaCollider::COctreeLeafCache& cache) {
  for (int i = 0; i < 8; ++i) {
    CAreaOctTree::Node::ETreeType type = node.GetChildType(i);
    if (type != CAreaOctTree::Node::ETreeType::Invalid) {
      CAreaOctTree::Node ch = node.GetChild(i);
      if (aabb.intersects(ch.GetBoundingBox())) {
        if (type == CAreaOctTree::Node::ETreeType::Leaf)
          cache.AddLeaf(ch);
        else
          BuildOctreeLeafCache(ch, aabb, cache);
      }
    }
  }
}

static zeus::CVector3f ClipRayToPlane(const zeus::CVector3f& a, const zeus::CVector3f& b, const zeus::CPlane& plane) {
  return (1.f - -plane.pointToPlaneDist(a) / (b - a).dot(plane.normal())) * (a - b) + b;
}

bool CMetroidAreaCollider::ConvexPolyCollision(const std::array<zeus::CPlane, 6>& planes,
                                               const std::array<zeus::CVector3f, 3>& verts, zeus::CAABox& aabb) {
  std::array<rstl::reserved_vector<zeus::CVector3f, 20>, 2> vecs;

  g_CalledClip += 1;
  g_RejectedByClip -= 1;

  vecs[0].push_back(verts[0]);
  vecs[0].push_back(verts[1]);
  vecs[0].push_back(verts[2]);

  int vecIdx = 0;
  int otherVecIdx = 1;
  for (int i = 0; i < 6; ++i) {
    rstl::reserved_vector<zeus::CVector3f, 20>& vec = vecs[vecIdx];
    rstl::reserved_vector<zeus::CVector3f, 20>& otherVec = vecs[otherVecIdx];
    otherVec.clear();

    bool inFrontOf = planes[i].pointToPlaneDist(vec.front()) >= 0.f;
    for (size_t j = 0; j < vec.size(); ++j) {
      const zeus::CVector3f& b = vec[(j + 1) % vec.size()];
      if (inFrontOf)
        otherVec.push_back(vec[j]);
      bool nextInFrontOf = planes[i].pointToPlaneDist(b) >= 0.f;
      if (nextInFrontOf ^ inFrontOf)
        otherVec.push_back(ClipRayToPlane(vec[j], b, planes[i]));
      inFrontOf = nextInFrontOf;
    }

    if (otherVec.empty())
      return false;

    vecIdx ^= 1;
    otherVecIdx ^= 1;
  }

  rstl::reserved_vector<zeus::CVector3f, 20>& accumVec = vecs[otherVecIdx ^ 1];
  for (const zeus::CVector3f& point : accumVec)
    aabb.accumulateBounds(point);

  g_RejectedByClip -= 1;
  return true;
}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache,
                                                             const zeus::CAABox& aabb, const CMaterialFilter& filter) {
  CBooleanAABoxAreaCache cache(aabb, filter);

  for (const CAreaOctTree::Node& node : leafCache.x4_nodeCache) {
    if (cache.x0_aabb.intersects(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      for (int j = 0; j < list.GetSize(); ++j) {
        ++g_TrianglesProcessed;
        CCollisionSurface surf = node.GetOwner().GetMasterListTriangle(list.GetAt(j));
        if (cache.x4_filter.Passes(CMaterialList(surf.GetSurfaceFlags()))) {
          if (CollisionUtil::TriBoxOverlap(cache.x8_center, cache.x14_halfExtent, surf.GetVert(0), surf.GetVert(1),
                                           surf.GetVert(2)))
            return true;
        }
      }
    }
  }

  return false;
}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean_Internal(const CAreaOctTree::Node& node,
                                                               const CBooleanAABoxAreaCache& cache) {
  for (int i = 0; i < 8; ++i) {
    CAreaOctTree::Node::ETreeType type = node.GetChildType(i);
    if (type != CAreaOctTree::Node::ETreeType::Invalid) {
      CAreaOctTree::Node ch = node.GetChild(i);
      if (cache.x0_aabb.intersects(ch.GetBoundingBox())) {
        if (type == CAreaOctTree::Node::ETreeType::Leaf) {
          CAreaOctTree::TriListReference list = ch.GetTriangleArray();
          for (int j = 0; j < list.GetSize(); ++j) {
            ++g_TrianglesProcessed;
            CCollisionSurface surf = ch.GetOwner().GetMasterListTriangle(list.GetAt(j));
            if (cache.x4_filter.Passes(CMaterialList(surf.GetSurfaceFlags()))) {
              if (CollisionUtil::TriBoxOverlap(cache.x8_center, cache.x14_halfExtent, surf.GetVert(0), surf.GetVert(1),
                                               surf.GetVert(2)))
                return true;
            }
          }
        } else {
          if (AABoxCollisionCheckBoolean_Internal(ch, cache))
            return true;
        }
      }
    }
  }
  return false;
}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                                      const CMaterialFilter& filter) {
  CBooleanAABoxAreaCache cache(aabb, filter);
  return AABoxCollisionCheckBoolean_Internal(octTree.GetRootNode(), cache);
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache,
                                                              const zeus::CAABox& aabb, const zeus::CSphere& sphere,
                                                              const CMaterialFilter& filter) {
  CBooleanSphereAreaCache cache(aabb, sphere, filter);

  for (const CAreaOctTree::Node& node : leafCache.x4_nodeCache) {
    if (cache.x0_aabb.intersects(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      for (int j = 0; j < list.GetSize(); ++j) {
        ++g_TrianglesProcessed;
        CCollisionSurface surf = node.GetOwner().GetMasterListTriangle(list.GetAt(j));
        if (cache.x8_filter.Passes(CMaterialList(surf.GetSurfaceFlags()))) {
          if (CollisionUtil::TriSphereOverlap(cache.x4_sphere, surf.GetVert(0), surf.GetVert(1), surf.GetVert(2)))
            return true;
        }
      }
    }
  }

  return false;
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean_Internal(const CAreaOctTree::Node& node,
                                                                const CBooleanSphereAreaCache& cache) {
  for (int i = 0; i < 8; ++i) {
    CAreaOctTree::Node::ETreeType type = node.GetChildType(i);
    if (type != CAreaOctTree::Node::ETreeType::Invalid) {
      CAreaOctTree::Node ch = node.GetChild(i);
      if (cache.x0_aabb.intersects(ch.GetBoundingBox())) {
        if (type == CAreaOctTree::Node::ETreeType::Leaf) {
          CAreaOctTree::TriListReference list = ch.GetTriangleArray();
          for (int j = 0; j < list.GetSize(); ++j) {
            ++g_TrianglesProcessed;
            CCollisionSurface surf = ch.GetOwner().GetMasterListTriangle(list.GetAt(j));
            if (cache.x8_filter.Passes(CMaterialList(surf.GetSurfaceFlags()))) {
              if (CollisionUtil::TriSphereOverlap(cache.x4_sphere, surf.GetVert(0), surf.GetVert(1), surf.GetVert(2)))
                return true;
            }
          }
        } else {
          if (SphereCollisionCheckBoolean_Internal(ch, cache))
            return true;
        }
      }
    }
  }
  return false;
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                                       const zeus::CSphere& sphere, const CMaterialFilter& filter) {
  CAreaOctTree::Node node = octTree.GetRootNode();
  CBooleanSphereAreaCache cache(aabb, sphere, filter);
  return SphereCollisionCheckBoolean_Internal(node, cache);
}

bool CMetroidAreaCollider::AABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                      const CMaterialFilter& filter, const CMaterialList& matList,
                                                      CCollisionInfoList& list) {
  bool ret = false;
  const std::array<zeus::CPlane, 6> planes{{
      {zeus::skRight, aabb.min.dot(zeus::skRight)},
      {zeus::skLeft, aabb.max.dot(zeus::skLeft)},
      {zeus::skForward, aabb.min.dot(zeus::skForward)},
      {zeus::skBack, aabb.max.dot(zeus::skBack)},
      {zeus::skUp, aabb.min.dot(zeus::skUp)},
      {zeus::skDown, aabb.max.dot(zeus::skDown)},
  }};
  CAABoxAreaCache cache(aabb, planes, filter, matList, list);

  ResetInternalCounters();

  for (const CAreaOctTree::Node& node : leafCache.x4_nodeCache) {
    if (aabb.intersects(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference listRef = node.GetTriangleArray();
      for (int j = 0; j < listRef.GetSize(); ++j) {
        ++g_TrianglesProcessed;
        u16 triIdx = listRef.GetAt(j);
        if (g_DupPrimitiveCheckCount == g_DupTriangleList[triIdx]) {
          g_DupTrianglesProcessed += 1;
        } else {
          g_DupTriangleList[triIdx] = g_DupPrimitiveCheckCount;
          CCollisionSurface surf = node.GetOwner().GetMasterListTriangle(triIdx);
          CMaterialList material(surf.GetSurfaceFlags());
          if (cache.x8_filter.Passes(material)) {
            if (CollisionUtil::TriBoxOverlap(cache.x14_center, cache.x20_halfExtent, surf.GetVert(0), surf.GetVert(1),
                                             surf.GetVert(2))) {
              zeus::CAABox aabb2 = zeus::CAABox();
              if (ConvexPolyCollision(cache.x4_planes, surf.GetVerts(), aabb2)) {
                zeus::CPlane plane = surf.GetPlane();
                CCollisionInfo collision(aabb2, cache.xc_material, material, plane.normal(), -plane.normal());
                cache.x10_collisionList.Add(collision, false);
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

bool CMetroidAreaCollider::AABoxCollisionCheck_Internal(const CAreaOctTree::Node& node, const CAABoxAreaCache& cache) {
  bool ret = false;

  switch (node.GetTreeType()) {
  case CAreaOctTree::Node::ETreeType::Invalid:
    return false;
  case CAreaOctTree::Node::ETreeType::Branch: {
    for (int i = 0; i < 8; ++i) {
      CAreaOctTree::Node ch = node.GetChild(i);
      if (ch.GetBoundingBox().intersects(cache.x0_aabb))
        if (AABoxCollisionCheck_Internal(ch, cache))
          ret = true;
    }
    break;
  }
  case CAreaOctTree::Node::ETreeType::Leaf: {
    CAreaOctTree::TriListReference list = node.GetTriangleArray();
    for (int j = 0; j < list.GetSize(); ++j) {
      ++g_TrianglesProcessed;
      u16 triIdx = list.GetAt(j);
      if (g_DupPrimitiveCheckCount == g_DupTriangleList[triIdx]) {
        g_DupTrianglesProcessed += 1;
      } else {
        g_DupTriangleList[triIdx] = g_DupPrimitiveCheckCount;
        CCollisionSurface surf = node.GetOwner().GetMasterListTriangle(triIdx);
        CMaterialList material(surf.GetSurfaceFlags());
        if (cache.x8_filter.Passes(material)) {
          if (CollisionUtil::TriBoxOverlap(cache.x14_center, cache.x20_halfExtent, surf.GetVert(0), surf.GetVert(1),
                                           surf.GetVert(2))) {
            zeus::CAABox aabb = zeus::CAABox();
            if (ConvexPolyCollision(cache.x4_planes, surf.GetVerts(), aabb)) {
              zeus::CPlane plane = surf.GetPlane();
              CCollisionInfo collision(aabb, cache.xc_material, material, plane.normal(), -plane.normal());
              cache.x10_collisionList.Add(collision, false);
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

bool CMetroidAreaCollider::AABoxCollisionCheck(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                               const CMaterialFilter& filter, const CMaterialList& matList,
                                               CCollisionInfoList& list) {
  const std::array<zeus::CPlane, 6> planes{{
      {zeus::skRight, aabb.min.dot(zeus::skRight)},
      {zeus::skLeft, aabb.max.dot(zeus::skLeft)},
      {zeus::skForward, aabb.min.dot(zeus::skForward)},
      {zeus::skBack, aabb.max.dot(zeus::skBack)},
      {zeus::skUp, aabb.min.dot(zeus::skUp)},
      {zeus::skDown, aabb.max.dot(zeus::skDown)},
  }};
  const CAABoxAreaCache cache(aabb, planes, filter, matList, list);

  ResetInternalCounters();

  const CAreaOctTree::Node node = octTree.GetRootNode();
  return AABoxCollisionCheck_Internal(node, cache);
}

bool CMetroidAreaCollider::SphereCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                       const zeus::CSphere& sphere, const CMaterialList& matList,
                                                       const CMaterialFilter& filter, CCollisionInfoList& clist) {
  ResetInternalCounters();

  bool ret = false;
  zeus::CVector3f point, normal;

  for (const CAreaOctTree::Node& node : leafCache.x4_nodeCache) {
    if (aabb.intersects(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      for (int j = 0; j < list.GetSize(); ++j) {
        ++g_TrianglesProcessed;
        u16 triIdx = list.GetAt(j);
        if (g_DupPrimitiveCheckCount == g_DupTriangleList[triIdx]) {
          g_DupTrianglesProcessed += 1;
        } else {
          g_DupTriangleList[triIdx] = g_DupPrimitiveCheckCount;
          CCollisionSurface surf = node.GetOwner().GetMasterListTriangle(triIdx);
          CMaterialList material(surf.GetSurfaceFlags());
          if (filter.Passes(material)) {
            if (CollisionUtil::TriSphereIntersection(sphere, surf.GetVert(0), surf.GetVert(1), surf.GetVert(2), point,
                                                     normal)) {
              CCollisionInfo collision(point, matList, material, normal);
              clist.Add(collision, false);
              ret = true;
            }
          }
        }
      }
    }
  }

  return ret;
}

bool CMetroidAreaCollider::SphereCollisionCheck_Internal(const CAreaOctTree::Node& node,
                                                         const CSphereAreaCache& cache) {
  bool ret = false;
  zeus::CVector3f point, normal;

  for (int i = 0; i < 8; ++i) {
    CAreaOctTree::Node::ETreeType chTp = node.GetChildType(i);
    if (chTp != CAreaOctTree::Node::ETreeType::Invalid) {
      CAreaOctTree::Node ch = node.GetChild(i);
      if (cache.x0_aabb.intersects(ch.GetBoundingBox())) {
        if (chTp == CAreaOctTree::Node::ETreeType::Leaf) {
          CAreaOctTree::TriListReference list = ch.GetTriangleArray();
          for (int j = 0; j < list.GetSize(); ++j) {
            ++g_TrianglesProcessed;
            u16 triIdx = list.GetAt(j);
            if (g_DupPrimitiveCheckCount == g_DupTriangleList[triIdx]) {
              g_DupTrianglesProcessed += 1;
            } else {
              g_DupTriangleList[triIdx] = g_DupPrimitiveCheckCount;
              CCollisionSurface surf = ch.GetOwner().GetMasterListTriangle(triIdx);
              CMaterialList material(surf.GetSurfaceFlags());
              if (cache.x8_filter.Passes(material)) {
                if (CollisionUtil::TriSphereIntersection(cache.x4_sphere, surf.GetVert(0), surf.GetVert(1),
                                                         surf.GetVert(2), point, normal)) {
                  CCollisionInfo collision(point, cache.xc_material, material, normal);
                  cache.x10_collisionList.Add(collision, false);
                  ret = true;
                }
              }
            }
          }
        } else {
          if (SphereCollisionCheck_Internal(ch, cache))
            ret = true;
        }
      }
    }
  }

  return ret;
}

bool CMetroidAreaCollider::SphereCollisionCheck(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                                const zeus::CSphere& sphere, const CMaterialList& matList,
                                                const CMaterialFilter& filter, CCollisionInfoList& list) {
  CSphereAreaCache cache(aabb, sphere, filter, matList, list);
  ResetInternalCounters();
  CAreaOctTree::Node node = octTree.GetRootNode();
  return SphereCollisionCheck_Internal(node, cache);
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_BoxVertexTri(
    const CCollisionSurface& surf, const zeus::CAABox& aabb, const rstl::reserved_vector<u32, 8>& vertIndices,
    const zeus::CVector3f& dir, double& d, zeus::CVector3f& normalOut, zeus::CVector3f& pointOut) {
  bool ret = false;
  for (u32 idx : vertIndices) {
    zeus::CVector3f point = aabb.getPoint(idx);
    if (CollisionUtil::RayTriangleIntersection_Double(point, dir, surf.GetVerts(), d)) {
      pointOut = float(d) * dir + point;
      normalOut = surf.GetNormal();
      ret = true;
    }
  }
  return ret;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_TriVertexBox(const zeus::CVector3f& vert, const zeus::CAABox& aabb,
                                                                  const zeus::CVector3f& dir, double& dOut,
                                                                  zeus::CVector3f& normal, zeus::CVector3f& point) {
  zeus::CMRay ray(vert, -dir, dOut);
  zeus::CVector3f norm;
  double d;
  if (CollisionUtil::RayAABoxIntersection_Double(ray, aabb, norm, d) == 2) {
    d *= dOut;
    if (d < dOut) {
      normal = -norm;
      dOut = d;
      point = vert;
      return true;
    }
  }
  return false;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_Edge(const zeus::CVector3f& ev0, const zeus::CVector3f& ev1,
                                                          const rstl::reserved_vector<SBoxEdge, 12>& edges,
                                                          const zeus::CVector3f& dir, double& d,
                                                          zeus::CVector3f& normal, zeus::CVector3f& point) {
  bool ret = false;

  for (const SBoxEdge& edge : edges) {
    zeus::CVector3d ev0d = ev0;
    zeus::CVector3d ev1d = ev1;
    if ((edge.x70_coDir.dot(ev1d) >= edge.x88_dirCoDirDot) != (edge.x70_coDir.dot(ev0d) >= edge.x88_dirCoDirDot)) {
      zeus::CVector3d delta = ev0d - ev1d;
      zeus::CVector3d cross0 = edge.x58_delta.cross(delta);
      if (cross0.magSquared() >= DBL_EPSILON) {
        zeus::CVector3d cross0Norm = cross0.asNormalized();
        if (cross0Norm.dot(dir) >= 0.0) {
          ev1d = ev0;
          ev0d = ev1;
          delta = ev0d - ev1d;
          cross0Norm = edge.x58_delta.cross(delta).asNormalized();
        }

        zeus::CVector3d clipped =
            ev0d + (-(ev0d.dot(edge.x70_coDir) - edge.x88_dirCoDirDot) / delta.dot(edge.x70_coDir)) * delta;
        int maxCompIdx = (std::fabs(edge.x70_coDir.x()) > std::fabs(edge.x70_coDir.y())) ? 0 : 1;
        if (std::fabs(edge.x70_coDir[maxCompIdx]) < std::fabs(edge.x70_coDir.z()))
          maxCompIdx = 2;

        int ci0, ci1;
        switch (maxCompIdx) {
        case 0:
          ci0 = 1;
          ci1 = 2;
          break;
        case 1:
          ci0 = 0;
          ci1 = 2;
          break;
        default:
          ci0 = 0;
          ci1 = 1;
          break;
        }

        double mag = (edge.x58_delta[ci0] * (clipped[ci1] - edge.x28_start[ci1]) -
                      edge.x58_delta[ci1] * (clipped[ci0] - edge.x28_start[ci0])) /
                     (edge.x58_delta[ci0] * dir[ci1] - edge.x58_delta[ci1] * dir[ci0]);
        if (mag >= 0.0 && mag < d) {
          zeus::CVector3d clippedMag = clipped - mag * zeus::CVector3d(dir);
          if ((edge.x28_start - clippedMag).dot(edge.x40_end - clippedMag) < 0.0 && mag < d) {
            normal = cross0Norm.asCVector3f();
            d = mag;
            point = clipped.asCVector3f();
            ret = true;
          }
        }
      }
    }
  }

  return ret;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                            const CMaterialFilter& filter, const CMaterialList& matList,
                                                            const zeus::CVector3f& dir, float mag,
                                                            CCollisionInfo& infoOut, double& dOut) {
  bool ret = false;
  ResetInternalCounters();
  dOut = mag;

  CMovingAABoxComponents components(aabb, dir);

  zeus::CAABox movedAABB = components.x6e8_aabb;
  zeus::CVector3f moveVec = mag * dir;
  movedAABB.accumulateBounds(aabb.min + moveVec);
  movedAABB.accumulateBounds(aabb.max + moveVec);

  zeus::CVector3f center = movedAABB.center();
  zeus::CVector3f extent = movedAABB.extents();

  zeus::CVector3f normal, point;

  for (const CAreaOctTree::Node& node : leafCache.x4_nodeCache) {
    if (movedAABB.intersects(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      for (int j = 0; j < list.GetSize(); ++j) {
        u16 triIdx = list.GetAt(j);
        if (g_DupPrimitiveCheckCount != g_DupTriangleList[triIdx]) {
          g_TrianglesProcessed += 1;
          g_DupTriangleList[triIdx] = g_DupPrimitiveCheckCount;
          CMaterialList triMat(node.GetOwner().GetTriangleMaterial(triIdx));
          if (filter.Passes(triMat)) {
            std::array<u16, 3> vertIndices;
            node.GetOwner().GetTriangleVertexIndices(triIdx, vertIndices.data());
            CCollisionSurface surf(node.GetOwner().GetVert(vertIndices[0]), node.GetOwner().GetVert(vertIndices[1]),
                                   node.GetOwner().GetVert(vertIndices[2]), triMat.GetValue());

            if (CollisionUtil::TriBoxOverlap(center, extent, surf.GetVert(0), surf.GetVert(1), surf.GetVert(2))) {
              bool triRet = false;
              double d = dOut;
              if (MovingAABoxCollisionCheck_BoxVertexTri(surf, aabb, components.x6c4_vertIdxs, dir, d, normal, point) &&
                  d < dOut) {
                triRet = true;
                ret = true;
                infoOut = CCollisionInfo(point, matList, triMat, normal);
                dOut = d;
              }

              for (const u16 vertIdx : vertIndices) {
                zeus::CVector3f vtx = node.GetOwner().GetVert(vertIdx);
                if (g_DupPrimitiveCheckCount != g_DupVertexList[vertIdx]) {
                  g_DupVertexList[vertIdx] = g_DupPrimitiveCheckCount;
                  if (movedAABB.pointInside(vtx)) {
                    d = dOut;
                    if (MovingAABoxCollisionCheck_TriVertexBox(vtx, aabb, dir, d, normal, point) && d < dOut) {
                      CMaterialList vertMat(node.GetOwner().GetVertMaterial(vertIdx));
                      triRet = true;
                      ret = true;
                      infoOut = CCollisionInfo(point, matList, vertMat, normal);
                      dOut = d;
                    }
                  }
                }
              }

              const u16* edgeIndices = node.GetOwner().GetTriangleEdgeIndices(triIdx);
              for (int k = 0; k < 3; ++k) {
                u16 edgeIdx = edgeIndices[k];
                if (g_DupPrimitiveCheckCount != g_DupEdgeList[edgeIdx]) {
                  g_DupEdgeList[edgeIdx] = g_DupPrimitiveCheckCount;
                  CMaterialList edgeMat(node.GetOwner().GetEdgeMaterial(edgeIdx));
                  if (!edgeMat.HasMaterial(EMaterialTypes::NoEdgeCollision)) {
                    d = dOut;
                    const CCollisionEdge& edge = node.GetOwner().GetEdge(edgeIdx);
                    if (MovingAABoxCollisionCheck_Edge(node.GetOwner().GetVert(edge.GetVertIndex1()),
                                                       node.GetOwner().GetVert(edge.GetVertIndex2()),
                                                       components.x0_edges, dir, d, normal, point) &&
                        d < dOut) {
                      triRet = true;
                      ret = true;
                      infoOut = CCollisionInfo(point, matList, edgeMat, normal);
                      dOut = d;
                    }
                  }
                }
              }

              if (triRet) {
                moveVec = float(dOut) * dir;
                movedAABB = components.x6e8_aabb;
                movedAABB.accumulateBounds(aabb.min + moveVec);
                movedAABB.accumulateBounds(aabb.max + moveVec);
                center = movedAABB.center();
                extent = movedAABB.extents();
              }
            } else {
              const u16* edgeIndices = node.GetOwner().GetTriangleEdgeIndices(triIdx);
              g_DupEdgeList[edgeIndices[0]] = g_DupPrimitiveCheckCount;
              g_DupEdgeList[edgeIndices[1]] = g_DupPrimitiveCheckCount;
              g_DupEdgeList[edgeIndices[2]] = g_DupPrimitiveCheckCount;
              g_DupVertexList[vertIndices[0]] = g_DupPrimitiveCheckCount;
              g_DupVertexList[vertIndices[1]] = g_DupPrimitiveCheckCount;
              g_DupVertexList[vertIndices[2]] = g_DupPrimitiveCheckCount;
            }
          }
        }
      }
    }
  }

  return ret;
}

bool CMetroidAreaCollider::MovingSphereCollisionCheck_Cached(const COctreeLeafCache& leafCache,
                                                             const zeus::CAABox& aabb, const zeus::CSphere& sphere,
                                                             const CMaterialFilter& filter,
                                                             const CMaterialList& matList, const zeus::CVector3f& dir,
                                                             float mag, CCollisionInfo& infoOut, double& dOut) {
  bool ret = false;
  ResetInternalCounters();
  dOut = mag;

  zeus::CAABox movedAABB = aabb;
  zeus::CVector3f moveVec = mag * dir;
  movedAABB.accumulateBounds(aabb.min + moveVec);
  movedAABB.accumulateBounds(aabb.max + moveVec);

  zeus::CVector3f center = movedAABB.center();
  zeus::CVector3f extent = movedAABB.extents();

  for (const CAreaOctTree::Node& node : leafCache.x4_nodeCache) {
    if (movedAABB.intersects(node.GetBoundingBox())) {
      CAreaOctTree::TriListReference list = node.GetTriangleArray();
      for (int j = 0; j < list.GetSize(); ++j) {
        u16 triIdx = list.GetAt(j);
        if (g_DupPrimitiveCheckCount != g_DupTriangleList[triIdx]) {
          g_TrianglesProcessed += 1;
          g_DupTriangleList[triIdx] = g_DupPrimitiveCheckCount;
          CMaterialList triMat(node.GetOwner().GetTriangleMaterial(triIdx));
          if (filter.Passes(triMat)) {
            std::array<u16, 3> vertIndices;
            node.GetOwner().GetTriangleVertexIndices(triIdx, vertIndices.data());
            CCollisionSurface surf(node.GetOwner().GetVert(vertIndices[0]), node.GetOwner().GetVert(vertIndices[1]),
                                   node.GetOwner().GetVert(vertIndices[2]), triMat.GetValue());

            if (CollisionUtil::TriBoxOverlap(center, extent, surf.GetVert(0), surf.GetVert(1), surf.GetVert(2))) {
              zeus::CVector3f surfNormal = surf.GetNormal();
              if ((sphere.position + moveVec - surf.GetVert(0)).dot(surfNormal) <= sphere.radius) {
                bool triRet = false;

                float mag = (sphere.radius - (sphere.position - surf.GetVert(0)).dot(surfNormal)) / dir.dot(surfNormal);
                zeus::CVector3f intersectPoint = sphere.position + mag * dir;

                const std::array<bool, 3> outsideEdges{
                    (intersectPoint - surf.GetVert(0)).dot(surfNormal.cross(surf.GetVert(1) - surf.GetVert(0))) < 0.f,
                    (intersectPoint - surf.GetVert(1)).dot(surfNormal.cross(surf.GetVert(2) - surf.GetVert(1))) < 0.f,
                    (intersectPoint - surf.GetVert(2)).dot(surfNormal.cross(surf.GetVert(0) - surf.GetVert(2))) < 0.f,
                };

                if (mag >= 0.f && !outsideEdges[0] && !outsideEdges[1] && !outsideEdges[2] && mag < dOut) {
                  infoOut = CCollisionInfo(intersectPoint - sphere.radius * surfNormal, matList, triMat, surfNormal);
                  dOut = mag;
                  triRet = true;
                  ret = true;
                }

                bool intersects = (sphere.position - surf.GetVert(0)).dot(surfNormal) <= sphere.radius;
                std::array<bool, 3> testVert{true, true, true};
                const u16* edgeIndices = node.GetOwner().GetTriangleEdgeIndices(triIdx);
                for (int k = 0; k < 3; ++k) {
                  if (intersects || outsideEdges[k]) {
                    u16 edgeIdx = edgeIndices[k];
                    if (g_DupPrimitiveCheckCount != g_DupEdgeList[edgeIdx]) {
                      g_DupEdgeList[edgeIdx] = g_DupPrimitiveCheckCount;
                      CMaterialList edgeMat(node.GetOwner().GetEdgeMaterial(edgeIdx));
                      if (!edgeMat.HasMaterial(EMaterialTypes::NoEdgeCollision)) {
                        int nextIdx = (k + 1) % 3;
                        zeus::CVector3f edgeVec = surf.GetVert(nextIdx) - surf.GetVert(k);
                        float edgeVecMag = edgeVec.magnitude();
                        edgeVec *= zeus::CVector3f(1.f / edgeVecMag);
                        float dirDotEdge = dir.dot(edgeVec);
                        zeus::CVector3f edgeRej = dir - dirDotEdge * edgeVec;
                        float edgeRejMagSq = edgeRej.magSquared();
                        zeus::CVector3f vertToSphere = sphere.position - surf.GetVert(k);
                        float vtsDotEdge = vertToSphere.dot(edgeVec);
                        zeus::CVector3f vtsRej = vertToSphere - vtsDotEdge * edgeVec;
                        if (edgeRejMagSq > 0.f) {
                          float tmp = 2.f * vtsRej.dot(edgeRej);
                          float tmp2 =
                              4.f * edgeRejMagSq * (vtsRej.magSquared() - sphere.radius * sphere.radius) - tmp * tmp;
                          if (tmp2 >= 0.f) {
                            float mag = 0.5f / edgeRejMagSq * (-tmp - std::sqrt(tmp2));
                            if (mag >= 0.f) {
                              float t = mag * dirDotEdge + vtsDotEdge;
                              if (t >= 0.f && t <= edgeVecMag && mag < dOut) {
                                zeus::CVector3f point = surf.GetVert(k) + t * edgeVec;
                                infoOut = CCollisionInfo(point, matList, edgeMat,
                                                         (sphere.position + mag * dir - point).normalized());
                                dOut = mag;
                                triRet = true;
                                ret = true;
                                testVert[k] = false;
                                testVert[nextIdx] = false;
                              } else if (t < -sphere.radius && dirDotEdge <= 0.f) {
                                testVert[k] = false;
                              } else if (t > edgeVecMag + sphere.radius && dirDotEdge >= 0.0) {
                                testVert[nextIdx] = false;
                              }
                            }
                          } else {
                            testVert[k] = false;
                            testVert[nextIdx] = false;
                          }
                        }
                      }
                    }
                  }
                }

                for (int k = 0; k < 3; ++k) {
                  u16 vertIdx = vertIndices[k];
                  if (testVert[k]) {
                    if (g_DupPrimitiveCheckCount != g_DupVertexList[vertIdx]) {
                      g_DupVertexList[vertIdx] = g_DupPrimitiveCheckCount;
                      double d = dOut;
                      if (CollisionUtil::RaySphereIntersection_Double(zeus::CSphere(surf.GetVert(k), sphere.radius),
                                                                      sphere.position, dir, d) &&
                          d >= 0.0) {
                        infoOut = CCollisionInfo(surf.GetVert(k), matList, node.GetOwner().GetVertMaterial(vertIdx),
                                                 (sphere.position + dir * d - surf.GetVert(k)).normalized());
                        dOut = d;
                        triRet = true;
                        ret = true;
                      }
                    }
                  } else {
                    g_DupVertexList[vertIdx] = g_DupPrimitiveCheckCount;
                  }
                }

                if (triRet) {
                  moveVec = float(dOut) * dir;
                  movedAABB = aabb;
                  movedAABB.accumulateBounds(aabb.min + moveVec);
                  movedAABB.accumulateBounds(aabb.max + moveVec);
                  center = movedAABB.center();
                  extent = movedAABB.extents();
                }
              }
            } else {
              const u16* edgeIndices = node.GetOwner().GetTriangleEdgeIndices(triIdx);
              g_DupEdgeList[edgeIndices[0]] = g_DupPrimitiveCheckCount;
              g_DupEdgeList[edgeIndices[1]] = g_DupPrimitiveCheckCount;
              g_DupEdgeList[edgeIndices[2]] = g_DupPrimitiveCheckCount;
              g_DupVertexList[vertIndices[0]] = g_DupPrimitiveCheckCount;
              g_DupVertexList[vertIndices[1]] = g_DupPrimitiveCheckCount;
              g_DupVertexList[vertIndices[2]] = g_DupPrimitiveCheckCount;
            }
          }
        }
      }
    }
  }

  return ret;
}

void CMetroidAreaCollider::ResetInternalCounters() {
  g_CalledClip = 0;
  g_RejectedByClip = 0;
  g_TrianglesProcessed = 0;
  g_DupTrianglesProcessed = 0;
  if (g_DupPrimitiveCheckCount == 0xffff) {
    g_DupVertexList.fill(0);
    g_DupEdgeList.fill(0);
    g_DupTriangleList.fill(0);
    g_DupPrimitiveCheckCount += 1;
  }
  g_DupPrimitiveCheckCount += 1;
}

void CAreaCollisionCache::ClearCache() {
  x18_leafCaches.clear();
  x1b40_24_leafOverflow = false;
  x1b40_25_cacheOverflow = false;
}

void CAreaCollisionCache::AddOctreeLeafCache(const CMetroidAreaCollider::COctreeLeafCache& leafCache) {
  if (!leafCache.GetNumLeaves())
    return;

  if (leafCache.HasCacheOverflowed())
    x1b40_24_leafOverflow = true;

  if (x18_leafCaches.size() < 3) {
    x18_leafCaches.push_back(leafCache);
  } else {
    x1b40_24_leafOverflow = true;
    x1b40_25_cacheOverflow = true;
  }
}

} // namespace urde
