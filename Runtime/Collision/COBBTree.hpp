#ifndef __URDE_COBBTREE_HPP__
#define __URDE_COBBTREE_HPP__
#include "RetroTypes.hpp"
#include "CCollisionEdge.hpp"
#include "zeus/CVector3f.hpp"
#include "zeus/COBBox.hpp"

namespace urde
{
class COBBTree
{
public:
    struct SIndexData
    {
        std::vector<u32> x0_;
        std::vector<u8> x10_;
        std::vector<u8> x20_;
        std::vector<u8> x30_;
        std::vector<CCollisionEdge> x40_;
        std::vector<u16> x50_;
        std::vector<zeus::CVector3f> x60_;
        SIndexData()=default;
        SIndexData(CInputStream&);
    };

    class CLeafData
    {
        std::vector<u16> x0_surface;
    public:
        CLeafData()=default;
        CLeafData(const std::vector<u16>&);
        CLeafData(CInputStream&);

        const std::vector<u16>& GetSurfaceVector() const;
        size_t GetMemoryUsage() const;
    };

    class CNode
    {
        zeus::COBBox x0_obb;
        bool x3c_isLeaf = false;
        std::unique_ptr<CNode> x40_left;
        std::unique_ptr<CNode> x44_right;
        std::unique_ptr<CLeafData> x48_leaf;
    public:
        CNode() = default;
        CNode(const CNode&)=default;
        CNode(const zeus::CTransform&, const zeus::CVector3f&, const CNode*, const CNode*, const CLeafData*);
        CNode(CInputStream&);

        bool WasHit() const;
        void SetHit(bool) const;
        CNode* GetLeft() const;
        CNode* GetRight() const;
        CLeafData* GetLeafData() const;
        const zeus::COBBox& GetOBB() const;
        size_t GetMemoryUsage() const;
    };

private:

    u32 x0_magic = 0;
    u32 x4_version = 0;
    u32 x8_memsize = 0;
    /* CSimpleAllocator xc_ We're not using this but lets keep track*/
    SIndexData x18_indexData;
    std::unique_ptr<CNode> x88_root;
public:
    COBBTree()=default;
    COBBTree(const COBBTree::SIndexData&, const CNode*);
    COBBTree(CInputStream&);

    zeus::CAABox CalculateLocalAABox() const;
    zeus::CAABox CalculateAABox(const zeus::CTransform&) const;
};
}

#endif // __URDE_COBBTREE_HPP__
