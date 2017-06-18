#include "CMetroidAreaCollider.hpp"

namespace urde
{

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

bool CMetroidAreaCollider::AABoxCollisionCheckBoolean(const CAreaOctTree& octTree, const zeus::CAABox& aabb,
                                                      const CMaterialFilter& filter)
{
    return false;
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
