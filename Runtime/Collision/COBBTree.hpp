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

    class CNodeLeafData
    {
    };

    class CNode
    {
        zeus::COBBox x0_obb;
        bool x3c_ = false;
        std::unique_ptr<CNode> x40_;
        std::unique_ptr<CNode> x44_;
        std::unique_ptr<CNodeLeafData> x48_;
    public:
        CNode() = default;
        CNode(CInputStream&);
    };

private:

    u32 x0_magic = 0;
    u32 x4_version = 0;
    u32 x8_memsize = 0;
    /* CSimpleAllocator xc_ We're not using this but lets keep track*/
    SIndexData x18_indexData;
    std::unique_ptr<CNode> x88_root;
public:


    COBBTree(const COBBTree::SIndexData&, const CNode*);
    COBBTree(CInputStream&);
};
}

#endif // __URDE_COBBTREE_HPP__
