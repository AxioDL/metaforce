#include "CMetroidAreaCollider.hpp"
#include "CMaterialFilter.hpp"
#include "CollisionUtil.hpp"

namespace urde
{

u32 CMetroidAreaCollider::g_TrianglesProcessed = 0;

CBooleanAABoxAreaCache::CBooleanAABoxAreaCache(const zeus::CAABox& aabb, const CMaterialFilter& filter)
: x0_aabb(aabb), x4_filter(filter), x8_center(aabb.center()), x14_halfExtent(aabb.extents())
{}

CMetroidAreaCollider::COctreeLeafCache::COctreeLeafCache(const CAreaOctTree& octTree)
: x0_octTree(octTree)
{
    x908_24_overflow = false;
}

void CMetroidAreaCollider::COctreeLeafCache::AddLeaf(const CAreaOctTree::Node& node)
{
    if (x4_nodeCache.size() == 64)
    {
        x908_24_overflow = true;
        return;
    }

    x4_nodeCache.push_back(node);
}

void CMetroidAreaCollider::BuildOctreeLeafCache(const CAreaOctTree::Node& node, const zeus::CAABox& aabb,
                                                CMetroidAreaCollider::COctreeLeafCache& cache)
{
    for (int i=0 ; i<8 ; ++i)
    {
        u16 flags = (node.GetChildFlags() >> (i * 2)) & 0x3;
        if (flags)
        {
            CAreaOctTree::Node ch = node.GetChild(i);
            if (aabb.intersects(ch.GetBoundingBox()))
            {
                if (flags == 0x2)
                    cache.AddLeaf(ch);
                else
                    BuildOctreeLeafCache(ch, aabb, cache);
            }
        }
    }
}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                             const CMaterialFilter& filter)
{
    return false;
}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean_Internal(const CAreaOctTree::Node& node,
                                                               const CBooleanAABoxAreaCache& cache)
{
    for (int i=0 ; i<8 ; ++i)
    {
        CAreaOctTree::Node::ETreeType type = node.GetChildType(i);
        if (type != CAreaOctTree::Node::ETreeType::Invalid)
        {
            CAreaOctTree::Node ch = node.GetChild(i);
            if (cache.x0_aabb.intersects(ch.GetBoundingBox()))
            {
                if (type == CAreaOctTree::Node::ETreeType::Leaf)
                {
                    CAreaOctTree::TriListReference list = ch.GetTriangleArray();
                    for (int j=0 ; j<list.GetSize() ; ++j)
                    {
                        ++g_TrianglesProcessed;
                        CCollisionSurface surf = ch.GetOwner().GetMasterListTriangle(list.GetAt(j));
                        if (cache.x4_filter.Passes(CMaterialList(surf.GetSurfaceFlags())))
                        {
                            if (CollisionUtil::TriBoxOverlap(cache.x8_center, cache.x14_halfExtent,
                                                             surf.GetVert(0), surf.GetVert(1), surf.GetVert(2)))
                                return true;
                        }
                    }
                }
                else
                {
                    if (AABoxCollisionCheckBoolean_Internal(ch, cache))
                        return true;
                }
            }
        }
    }
    return false;
}

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                                      const CMaterialFilter& filter)
{
    CBooleanAABoxAreaCache cache(aabb, filter);
    return AABoxCollisionCheckBoolean_Internal(octTree.GetRootNode(), cache);
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                              const zeus::CSphere& sphere, const CMaterialFilter& filter)
{
    return false;
}

bool CMetroidAreaCollider::SphereCollisionCheckBoolean(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                                       const zeus::CSphere& sphere, const CMaterialFilter& filter)
{
    return false;
}

bool CMetroidAreaCollider::AABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                      const CMaterialFilter& filter, const CMaterialList& matList,
                                                      CCollisionInfoList& list)
{
    return false;
}

bool CMetroidAreaCollider::AABoxCollisionCheck(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                               const CMaterialFilter& filter, const CMaterialList& matList,
                                               CCollisionInfoList& list)
{
    return false;
}

bool CMetroidAreaCollider::SphereCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                       const zeus::CSphere& sphere, const CMaterialList& matList,
                                                       const CMaterialFilter& filter, CCollisionInfoList& list)
{
    return false;
}

bool CMetroidAreaCollider::SphereCollisionCheck(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                                const zeus::CSphere& sphere, const CMaterialList& matList,
                                                const CMaterialFilter& filter, CCollisionInfoList& list)
{
    return false;
}

bool CMetroidAreaCollider::MovingAABoxCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                            const CMaterialFilter& filter, const CMaterialList& matList,
                                                            const zeus::CVector3f& vec, float mag, CCollisionInfo& infoOut,
                                                            double& dOut)
{
    return false;
}

bool CMetroidAreaCollider::MovingSphereCollisionCheck_Cached(const COctreeLeafCache& leafCache, const zeus::CAABox& aabb,
                                                             const zeus::CSphere& sphere,
                                                             const CMaterialFilter& filter, const CMaterialList& matList,
                                                             const zeus::CVector3f& vec, float mag, CCollisionInfo& infoOut,
                                                             double& dOut)
{
    return false;
}

void CAreaCollisionCache::ClearCache()
{
    x18_leafCaches.clear();
    x1b40_24_leafOverflow = false;
    x1b40_25_cacheOverflow = false;
}

void CAreaCollisionCache::AddOctreeLeafCache(const CMetroidAreaCollider::COctreeLeafCache& leafCache)
{
    if (!leafCache.GetNumLeaves())
        return;

    if (leafCache.HasCacheOverflowed())
        x1b40_24_leafOverflow = true;

    if (x18_leafCaches.size() < 3)
    {
        x18_leafCaches.push_back(leafCache);
    }
    else
    {
        x1b40_24_leafOverflow = true;
        x1b40_25_cacheOverflow = true;
    }
}

}
