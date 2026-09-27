#ifndef _CMETROIDAREACOLLIDER
#define _CMETROIDAREACOLLIDER

#include "types.h"

#include "WorldFormat/CAreaOctTree.hpp"

#include "Collision/CCollisionInfoList.hpp"
#include "Collision/CMaterialFilter.hpp"
#include "Collision/CMaterialList.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CLineSeg.hpp"
#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CSphere.hpp"
#include "Kyoto/Math/CVector3d.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/reserved_vector.hpp"

class CAABoxAreaCache {
public:
  friend class CMetroidAreaCollider;

  CAABoxAreaCache(const CAABox& aabb, const CPlane* pl, const CMaterialFilter& filter,
                  const CMaterialList& material, CCollisionInfoList& collisionList);

private:
  const CAABox& mAabb;
  const CPlane* mPlanes;
  const CMaterialFilter& mFilter;
  const CMaterialList& mMaterial;
  CCollisionInfoList& mCollisionList;
  CVector3f mCenter;
  CVector3f mHalfExtent;
};

CHECK_SIZEOF(CAABoxAreaCache, 0x2c)

class CBooleanAABoxAreaCache {
public:
  friend class CMetroidAreaCollider;

  CBooleanAABoxAreaCache(const CAABox& aabb, const CMaterialFilter& filter);

private:
  const CAABox& mAabb;
  const CMaterialFilter& mFilter;
  CVector3f mCenter;
  CVector3f mHalfExtent;
};

CHECK_SIZEOF(CBooleanAABoxAreaCache, 0x20)

class CSphereAreaCache {
public:
  friend class CMetroidAreaCollider;

  CSphereAreaCache(const CAABox& aabb, const CSphere& sphere, const CMaterialFilter& filter,
                   const CMaterialList& material, CCollisionInfoList& collisionList)
  : mAabb(aabb)
  , mSphere(sphere)
  , mFilter(filter)
  , mMaterial(material)
  , mCollisionList(collisionList) {}

private:
  const CAABox& mAabb;
  const CSphere& mSphere;
  const CMaterialFilter& mFilter;
  const CMaterialList& mMaterial;
  CCollisionInfoList& mCollisionList;
};

CHECK_SIZEOF(CSphereAreaCache, 0x14)

class CBooleanSphereAreaCache {
public:
  friend class CMetroidAreaCollider;

  CBooleanSphereAreaCache(const CAABox& aabb, const CSphere& sphere, const CMaterialFilter& filter)
  : mAabb(aabb), mSphere(sphere), mFilter(filter) {}

private:
  const CAABox& mAabb;
  const CSphere& mSphere;
  const CMaterialFilter& mFilter;
};

CHECK_SIZEOF(CBooleanSphereAreaCache, 0xc)

class CMetroidAreaCollider {
public:
  struct SBoxEdge {
    CLineSeg mSeg;
    CVector3d mStart;
    CVector3d mEnd;
    CVector3d mDelta;
    CVector3d mCoDir;
    double mDirCoDirDot;
    SBoxEdge(const CAABox& aabb, int idx, const CVector3f& dir);
  };

  class CMovingAABoxComponents {
  public:
    CMovingAABoxComponents(const CAABox& aabb, const CVector3f& dir);

  private:
    friend class CMetroidAreaCollider;
    friend class CCollidableOBBTree;
    rstl::reserved_vector< SBoxEdge, 12 > mEdges;
    rstl::reserved_vector< uint, 8 > mVertIdxs;
    CAABox mAabb;
  };

  class COctreeLeafCache {
  public:
    COctreeLeafCache(const CAreaOctTree& octTree);
    void AddLeaf(const CAreaOctTree::Node& node);
    const CAreaOctTree::Node& GetLeaf(int i) const { return mNodeCache[i]; }
    int GetNumLeaves() const { return mNodeCache.size(); }
    bool HasCacheOverflowed() const { return mOverflow; }
    const CAreaOctTree& GetOctTree() const { return mOctTree; }
    rstl::reserved_vector< CAreaOctTree::Node, 64 >::const_iterator begin() const {
      return mNodeCache.begin();
    }
    rstl::reserved_vector< CAreaOctTree::Node, 64 >::const_iterator end() const {
      return mNodeCache.end();
    }

  private:
    friend class CMetroidAreaCollider;
    const CAreaOctTree& mOctTree;
    rstl::reserved_vector< CAreaOctTree::Node, 64 > mNodeCache;
    bool mOverflow : 1;
  };

  static bool ConvexPolyCollision(const CPlane* planes, const CVector3f* verts, CAABox& aabb);
  static void BuildOctreeLeafCache(const CAreaOctTree::Node& node, const CAABox& aabb,
                                   COctreeLeafCache& leafCache);
  static bool AABoxCollisionCheckBoolean(const CAreaOctTree& octTree, const CAABox& aabb,
                                         const CMaterialFilter& filter);
  static bool SphereCollisionCheckBoolean(const CAreaOctTree& octTree, const CAABox& aabb,
                                          const CSphere& sphere, const CMaterialFilter& filter);
  static bool AABoxCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache,
                                                const CAABox& aabb, const CMaterialFilter& filter);
  static bool SphereCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache,
                                                 const CAABox& aabb, const CSphere& sphere,
                                                 const CMaterialFilter& filter);
  static bool AABoxCollisionCheck(const CAreaOctTree& octTree, const CAABox& aabb,
                                  const CMaterialFilter& filter, const CMaterialList& matList,
                                  CCollisionInfoList& list);
  static bool SphereCollisionCheck(const CAreaOctTree& octTree, const CAABox& aabb,
                                   const CSphere& sphere, const CMaterialList& matList,
                                   const CMaterialFilter& filter, CCollisionInfoList& list);
  static bool AABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache, const CAABox& aabb,
                                         const CMaterialFilter& filter,
                                         const CMaterialList& matList, CCollisionInfoList& list);
  static bool SphereCollisionCheck_Cached(const COctreeLeafCache& leafCache, const CAABox& aabb,
                                          const CSphere& sphere, const CMaterialList& matList,
                                          const CMaterialFilter& filter, CCollisionInfoList& list);
  static bool MovingAABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache,
                                               const CAABox& aabb, const CMaterialFilter& filter,
                                               const CMaterialList& matList, CVector3f dir, float d,
                                               CCollisionInfo& infoOut, double& dOut);
  static bool MovingSphereCollisionCheck_Cached(const COctreeLeafCache& leafCache,
                                                const CAABox& aabb, const CSphere& sphere,
                                                const CMaterialFilter& filter,
                                                const CMaterialList& matList, CVector3f dir,
                                                float d, CCollisionInfo& infoOut, double& dOut);

  static void ResetInternalCounters();
  static ushort GetDupPrimitiveCheckCount() { return sDupPrimitiveCheckCount; }
  static ushort& DupVertexListValue(uint idx) { return sDupVertexList[idx]; }
  static ushort& DupEdgeListValue(uint idx) { return sDupEdgeList[idx]; }
  static ushort& DupTriangleListValue(uint idx) { return sDupTriangleList[idx]; }

private:
  friend class CCollidableOBBTree;
  static ushort sDupPrimitiveCheckCount;
  static ushort sDupVertexList[0x2800];
  static ushort sDupEdgeList[0x6000];
  static ushort sDupTriangleList[0x4000];
  static bool AABoxCollisionCheck_Internal(const CAreaOctTree::Node&, CAABoxAreaCache&);
  static bool AABoxCollisionCheckBoolean_Internal(const CAreaOctTree::Node&,
                                                  const CBooleanAABoxAreaCache&);
  static bool SphereCollisionCheck_Internal(const CAreaOctTree::Node&, CSphereAreaCache&);
  static bool SphereCollisionCheckBoolean_Internal(const CAreaOctTree::Node&,
                                                   const CBooleanSphereAreaCache&);
  static bool MovingAABoxCollisionCheck_BoxVertexTri(const CCollisionSurface&, const CAABox&,
                                                     const rstl::reserved_vector< uint, 8 >&,
                                                     CVector3f, double&, CVector3f&, CVector3f&);
  static bool MovingAABoxCollisionCheck_TriVertexBox(const CVector3f&, const CAABox&, CVector3f,
                                                     double&, CVector3f&, CVector3f&);
  static bool MovingAABoxCollisionCheck_Edge(const CVector3f&, const CVector3f&,
                                             const rstl::reserved_vector< SBoxEdge, 12 >&,
                                             CVector3f, double&, CVector3f&, CVector3f&);
};

NESTED_CHECK_SIZEOF(CMetroidAreaCollider, SBoxEdge, 0x90)
NESTED_CHECK_SIZEOF(CMetroidAreaCollider, CMovingAABoxComponents, 0x700)
NESTED_CHECK_SIZEOF(CMetroidAreaCollider, COctreeLeafCache, 0x90c)

class CAreaCollisionCache {
public:
  CAreaCollisionCache(const CAABox& aabb);

  void ClearCache();
  const CAABox& GetCacheBounds() const { return mAabb; }
  void SetCacheBounds(const CAABox& aabb);
  void AddOctreeLeafCache(const CMetroidAreaCollider::COctreeLeafCache& leafCache);
  uint GetNumCaches() const { return mLeafCaches.size(); }
  const CMetroidAreaCollider::COctreeLeafCache& GetOctreeLeafCache(int idx) {
    return mLeafCaches[idx];
  }
  const CMetroidAreaCollider::COctreeLeafCache& GetOctreeLeafCache(int idx) const {
    return mLeafCaches[idx];
  }
  bool HasCacheOverflowed() const { return mLeafOverflow; }
  rstl::reserved_vector< CMetroidAreaCollider::COctreeLeafCache, 3 >::const_iterator begin() const {
    return mLeafCaches.begin();
  }
  rstl::reserved_vector< CMetroidAreaCollider::COctreeLeafCache, 3 >::const_iterator end() const {
    return mLeafCaches.end();
  }

private:
  CAABox mAabb;
  rstl::reserved_vector< CMetroidAreaCollider::COctreeLeafCache, 3 > mLeafCaches;
  bool mLeafOverflow : 1;
  bool mCacheOverflow : 1;
};
CHECK_SIZEOF(CAreaCollisionCache, 0x1b44)

#endif // _CMETROIDAREACOLLIDER
