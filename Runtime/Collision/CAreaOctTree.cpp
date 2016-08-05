#include "CAreaOctTree.hpp"

namespace urde
{

CAreaOctTree::Node CAreaOctTree::Node::GetChild(int idx) const
{
    u16 flags = *reinterpret_cast<const u16*>(m_ptr);
    const u32* offsets = reinterpret_cast<const u32*>(m_ptr + 4);
    ETreeType type = ETreeType((flags << (2 * idx)) & 0x3);

    if (type == ETreeType::Branch)
    {
        zeus::CAABox pos, neg, res;
        m_aabb.splitZ(pos, neg);
        if (idx & 4)
        {
            pos.splitY(pos, neg);
            if (idx & 2)
            {
                pos.splitX(pos, neg);
                if (idx & 1)
                    res = pos;
                else
                    res = neg;
            }
            else
            {
                neg.splitX(pos, neg);
                if (idx & 1)
                    res = pos;
                else
                    res = neg;
            }
        }
        else
        {
            neg.splitY(pos, neg);
            if (idx & 2)
            {
                pos.splitX(pos, neg);
                if (idx & 1)
                    res = pos;
                else
                    res = neg;
            }
            else
            {
                neg.splitX(pos, neg);
                if (idx & 1)
                    res = pos;
                else
                    res = neg;
            }
        }

        return Node(m_ptr + offsets[idx] + 36, res, m_owner, ETreeType::Branch);
    }
    else if (type == ETreeType::Leaf)
    {
        const zeus::CAABox* aabb = reinterpret_cast<const zeus::CAABox*>(m_ptr + offsets[idx] + 36);
        return Node(aabb, *aabb, m_owner, ETreeType::Leaf);
    }
    else
    {
        return Node(nullptr, zeus::CAABox::skNullBox, m_owner, ETreeType::Invalid);
    }
}

void CAreaOctTree::SwapTreeNode(u8* ptr, Node::ETreeType type)
{
    if (type == Node::ETreeType::Branch)
    {
        u16* typeBits = reinterpret_cast<u16*>(ptr);
        *typeBits = hecl::SBig(*typeBits);

        for (int i=0 ; i<8 ; ++i)
        {
            Node::ETreeType ctype = Node::ETreeType((*typeBits << (2 * i)) & 0x3);
            u32* offsets = reinterpret_cast<u32*>(ptr + 4);
            offsets[i] = hecl::SBig(offsets[i]);
            SwapTreeNode(ptr + offsets[i] + 36, ctype);
        }
    }
    else if (type == Node::ETreeType::Leaf)
    {
        zeus::CAABox* aabb = reinterpret_cast<zeus::CAABox*>(ptr);
        aabb->min[0] = hecl::SBig(aabb->min[0]);
        aabb->min[1] = hecl::SBig(aabb->min[1]);
        aabb->min[2] = hecl::SBig(aabb->min[2]);
        aabb->max[0] = hecl::SBig(aabb->max[0]);
        aabb->max[1] = hecl::SBig(aabb->max[1]);
        aabb->max[2] = hecl::SBig(aabb->max[2]);

        u16* countIdxs = reinterpret_cast<u16*>(ptr + 24);
        *countIdxs = hecl::SBig(*countIdxs);
        for (u16 i=0 ; i<*countIdxs ; ++i)
            countIdxs[i+1] = hecl::SBig(countIdxs[i+1]);
    }
}

CAreaOctTree::CAreaOctTree(const zeus::CAABox& aabb, Node::ETreeType treeType, u8* buf, std::unique_ptr<u8[]>&& treeBuf,
                           u32 matCount, u32* materials, u8* vertMats, u8* edgeMats, u8* polyMats,
                           u32 edgeCount, CCollisionEdge* edges, u32 polyCount, u16* polyEdges,
                           u32 vertCount, zeus::CVector3f* verts)
: x0_aabb(aabb), x18_treeType(treeType), x1c_buf(buf), x20_treeBuf(std::move(treeBuf)),
  x24_matCount(matCount), x2c_vertMats(vertMats),
  x30_edgeMats(edgeMats), x34_polyMats(polyMats), x38_edgeCount(edgeCount),
  x40_polyCount(polyCount),
  x48_vertCount(vertCount)
{
    SwapTreeNode(x20_treeBuf.get(), treeType);

    {
        x28_materials.reserve(matCount);
        athena::io::MemoryReader r(materials, matCount * 4);
        for (u32 i=0 ; i<matCount ; ++i)
            x28_materials.push_back(r.readUint32Big());
    }

    {
        x3c_edges.reserve(edgeCount);
        athena::io::MemoryReader r(edges, edgeCount * 4);
        for (u32 i=0 ; i<edgeCount ; ++i)
            x3c_edges.emplace_back(r);
    }

    {
        x44_polyEdges.reserve(polyCount);
        athena::io::MemoryReader r(polyEdges, polyCount * 2);
        for (u32 i=0 ; i<polyCount ; ++i)
            x44_polyEdges.push_back(r.readUint16Big());
    }

    {
        x4c_verts.reserve(vertCount);
        athena::io::MemoryReader r(verts, vertCount * 12);
        for (u32 i=0 ; i<vertCount ; ++i)
            x4c_verts.push_back(zeus::CVector3f::ReadBig(r));
    }
}

std::unique_ptr<CAreaOctTree> CAreaOctTree::MakeFromMemory(void* buf, unsigned int size)
{
    athena::io::MemoryReader r(buf, size);
    r.readUint32Big();
    r.readUint32Big();
    zeus::CAABox aabb;
    aabb.readBoundingBoxBig(r);
    Node::ETreeType nodeType = Node::ETreeType(r.readUint32Big());
    u32 treeSize = r.readUint32Big();
    u8* cur = reinterpret_cast<u8*>(buf) + r.position();

    std::unique_ptr<u8[]> treeBuf(new u8[treeSize]);
    memmove(treeBuf.get(), cur, treeSize);
    cur += treeSize;

    u32 matCount = hecl::SBig(*reinterpret_cast<u32*>(cur));
    cur += 4;
    u32* matBuf = reinterpret_cast<u32*>(cur);
    cur += 4 * matCount;

    u32 vertMatsCount = hecl::SBig(*reinterpret_cast<u32*>(cur));
    cur += 4;
    u8* vertMatsBuf = cur;
    cur += vertMatsCount;

    u32 edgeMatsCount = hecl::SBig(*reinterpret_cast<u32*>(cur));
    cur += 4;
    u8* edgeMatsBuf = cur;
    cur += edgeMatsCount;

    u32 polyMatsCount = hecl::SBig(*reinterpret_cast<u32*>(cur));
    cur += 4;
    u8* polyMatsBuf = cur;
    cur += polyMatsCount;

    u32 edgeCount = hecl::SBig(*reinterpret_cast<u32*>(cur));
    cur += 4;
    CCollisionEdge* edgeBuf = reinterpret_cast<CCollisionEdge*>(cur);
    cur += edgeCount * sizeof(edgeCount);

    u32 polyCount = hecl::SBig(*reinterpret_cast<u32*>(cur));
    cur += 4;
    u16* polyBuf = reinterpret_cast<u16*>(cur);
    cur += polyCount * 2;

    u32 vertCount = hecl::SBig(*reinterpret_cast<u32*>(cur));
    cur += 4;
    zeus::CVector3f* vertBuf = reinterpret_cast<zeus::CVector3f*>(cur);
    cur += polyCount * 2;

    return std::make_unique<CAreaOctTree>(aabb, nodeType, reinterpret_cast<u8*>(buf), std::move(treeBuf),
                                          matCount, matBuf, vertMatsBuf, edgeMatsBuf, polyMatsBuf,
                                          edgeCount, edgeBuf, polyCount, polyBuf, vertCount, vertBuf);
}

}
