#ifndef __URDE_CMETROIDAREACOLLIDER_HPP__
#define __URDE_CMETROIDAREACOLLIDER_HPP__

#include "RetroTypes.hpp"
#include "zeus/CAABox.hpp"
#include "CAreaOctTree.hpp"

namespace urde
{
class CCollisionInfoList;
class CCollisionInfo;
class CMaterialList;

class CBooleanAABoxAreaCache
{
    friend class CMetroidAreaCollider;
    const zeus::CAABox& x0_aabb;
    const CMaterialFilter& x4_filter;
    zeus::CVector3f x8_center;
    zeus::CVector3f x14_halfExtent;
public:
    CBooleanAABoxAreaCache(const zeus::CAABox& aabb, const CMaterialFilter& filter);
};

class CMetroidAreaCollider
{
    static u32 g_TrianglesProcessed;
    static bool AABoxCollisionCheckBoolean_Internal(const CAreaOctTree::Node& node,
                                                    const CBooleanAABoxAreaCache& cache);
public:
    class COctreeLeafCache
    {
        const CAreaOctTree& x0_octTree;
        rstl::reserved_vector<CAreaOctTree::Node, 64> x4_nodeCache;
        bool x908_24_overflow : 1;
    public:
        COctreeLeafCache(const CAreaOctTree& octTree);
        void AddLeaf(const CAreaOctTree::Node& node);
        u32 GetNumLeaves() const { return x4_nodeCache.size(); }
        bool HasCacheOverflowed() const { return x908_24_overflow; }
    };
    static void BuildOctreeLeafCache(const CAreaOctTree::Node& root, const zeus::CAABox& aabb,
                                     CMetroidAreaCollider::COctreeLeafCache& cache);
    static bool AABoxCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                  const CMaterialFilter& filter);
    static bool AABoxCollisionCheckBoolean(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                           const CMaterialFilter& filter);
    static bool SphereCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                   const zeus::CSphere& sphere, const CMaterialFilter& filter);
    static bool SphereCollisionCheckBoolean(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                            const zeus::CSphere& sphere, const CMaterialFilter& filter);

    static bool AABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                           const CMaterialFilter& filter, const CMaterialList& matList,
                                           CCollisionInfoList& list);
    static bool AABoxCollisionCheck(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                    const CMaterialFilter& filter, const CMaterialList& matList,
                                    CCollisionInfoList& list);
    static bool SphereCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                            const zeus::CSphere& sphere, const CMaterialList& matList,
                                            const CMaterialFilter& filter, CCollisionInfoList& list);
    static bool SphereCollisionCheck(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                     const zeus::CSphere& sphere, const CMaterialList& matList,
                                     const CMaterialFilter& filter, CCollisionInfoList& list);
    static bool MovingAABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                 const CMaterialFilter& filter, const CMaterialList& matList,
                                                 const zeus::CVector3f& vec, float mag, CCollisionInfo& infoOut,
                                                 double& dOut);
    static bool MovingSphereCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                  const zeus::CSphere& sphere,
                                                  const CMaterialFilter& filter, const CMaterialList& matList,
                                                  const zeus::CVector3f& vec, float mag, CCollisionInfo& infoOut,
                                                  double& dOut);
};

class CAreaCollisionCache
{
    zeus::CAABox x0_aabb;
    rstl::reserved_vector<CMetroidAreaCollider::COctreeLeafCache, 3> x18_leafCaches;
    union
    {
        struct
        {
            bool x1b40_24_leafOverflow : 1;
            bool x1b40_25_cacheOverflow : 1;
        };
        u32 _dummy = 0;
    };
public:
    CAreaCollisionCache(const zeus::CAABox& aabb) : x0_aabb(aabb) {}
    void ClearCache();
    const zeus::CAABox& GetCacheBounds() const { return x0_aabb; }
    void SetCacheBounds(const zeus::CAABox& aabb) { x0_aabb = aabb; }
    void AddOctreeLeafCache(const CMetroidAreaCollider::COctreeLeafCache& leafCache);
    u32 GetNumCaches() const { return x18_leafCaches.size(); }
    const CMetroidAreaCollider::COctreeLeafCache& GetOctreeLeafCache(int idx) { return x18_leafCaches[idx]; }
    bool HasCacheOverflowed() const { return x1b40_24_leafOverflow; }
    rstl::reserved_vector<CMetroidAreaCollider::COctreeLeafCache, 3>::iterator begin() { return x18_leafCaches.begin(); }
    rstl::reserved_vector<CMetroidAreaCollider::COctreeLeafCache, 3>::iterator end() { return x18_leafCaches.end(); }
};

}

#endif // __URDE_CMETROIDAREACOLLIDER_HPP__
