#include "CAreaOctTree.hpp"

namespace urde
{

CAreaOctTree::Node CAreaOctTree::Node::GetChild(int idx) const
{
    u16 flags = *reinterpret_cast<const u16*>(m_ptr);
    const u32* offsets = reinterpret_cast<const u32*>(m_ptr + 4);
    ETreeType type = ETreeType((flags >> (2 * idx)) & 0x3);

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
        const float* aabb = reinterpret_cast<const float*>(m_ptr + offsets[idx] + 36);
        zeus::CAABox aabbObj(aabb[0], aabb[1], aabb[2], aabb[3], aabb[4], aabb[5]);
        return Node(aabb, aabbObj, m_owner, ETreeType::Leaf);
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
        u32* offsets = reinterpret_cast<u32*>(ptr + 4);

        for (int i=0 ; i<8 ; ++i)
        {
            Node::ETreeType ctype = Node::ETreeType((*typeBits >> (2 * i)) & 0x3);
            offsets[i] = hecl::SBig(offsets[i]);
            SwapTreeNode(ptr + offsets[i] + 36, ctype);
        }
    }
    else if (type == Node::ETreeType::Leaf)
    {
        float* aabb = reinterpret_cast<float*>(ptr);
        aabb[0] = hecl::SBig(aabb[0]);
        aabb[1] = hecl::SBig(aabb[1]);
        aabb[2] = hecl::SBig(aabb[2]);
        aabb[3] = hecl::SBig(aabb[3]);
        aabb[4] = hecl::SBig(aabb[4]);
        aabb[5] = hecl::SBig(aabb[5]);

        u16* countIdxs = reinterpret_cast<u16*>(ptr + 24);
        *countIdxs = hecl::SBig(*countIdxs);
        for (u16 i=0 ; i<*countIdxs ; ++i)
            countIdxs[i+1] = hecl::SBig(countIdxs[i+1]);
    }
}

CAreaOctTree::CAreaOctTree(const zeus::CAABox& aabb, Node::ETreeType treeType, const u8* buf, const u8* treeBuf,
                           u32 matCount, const u32* materials, const u8* vertMats, const u8* edgeMats, const u8* polyMats,
                           u32 edgeCount, const CCollisionEdge* edges, u32 polyCount, const u16* polyEdges,
                           u32 vertCount, const float* verts)
: x0_aabb(aabb), x18_treeType(treeType), x1c_buf(buf), x20_treeBuf(treeBuf),
  x24_matCount(matCount), x28_materials(materials), x2c_vertMats(vertMats),
  x30_edgeMats(edgeMats), x34_polyMats(polyMats), x38_edgeCount(edgeCount),
  x3c_edges(edges), x40_polyCount(polyCount), x44_polyEdges(polyEdges),
  x48_vertCount(vertCount), x4c_verts(verts)
{
    SwapTreeNode(const_cast<u8*>(x20_treeBuf), treeType);

    for (u32 i=0 ; i<matCount ; ++i)
        const_cast<u32*>(x28_materials)[i] = hecl::SBig(x28_materials[i]);

    for (u32 i=0 ; i<edgeCount ; ++i)
        const_cast<CCollisionEdge*>(x3c_edges)[i].swapBig();

    for (u32 i=0 ; i<polyCount ; ++i)
        const_cast<u16*>(x44_polyEdges)[i] = hecl::SBig(x44_polyEdges[i]);

    for (u32 i=0 ; i<vertCount*3 ; ++i)
        const_cast<float*>(x4c_verts)[i] = hecl::SBig(x4c_verts[i]);
}

std::unique_ptr<CAreaOctTree> CAreaOctTree::MakeFromMemory(const u8* buf, unsigned int size)
{
    athena::io::MemoryReader r(buf + 8, size - 8);
    r.readUint32Big();
    r.readUint32Big();
    zeus::CAABox aabb;
    aabb.readBoundingBoxBig(r);
    Node::ETreeType nodeType = Node::ETreeType(r.readUint32Big());
    u32 treeSize = r.readUint32Big();
    const u8* cur = reinterpret_cast<const u8*>(buf) + 8 + r.position();

    const u8* treeBuf = cur;
    cur += treeSize;

    u32 matCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
    cur += 4;
    const u32* matBuf = reinterpret_cast<const u32*>(cur);
    cur += 4 * matCount;

    u32 vertMatsCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
    cur += 4;
    const u8* vertMatsBuf = cur;
    cur += vertMatsCount;

    u32 edgeMatsCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
    cur += 4;
    const u8* edgeMatsBuf = cur;
    cur += edgeMatsCount;

    u32 polyMatsCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
    cur += 4;
    const u8* polyMatsBuf = cur;
    cur += polyMatsCount;

    u32 edgeCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
    cur += 4;
    const CCollisionEdge* edgeBuf = reinterpret_cast<const CCollisionEdge*>(cur);
    cur += edgeCount * sizeof(edgeCount);

    u32 polyCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
    cur += 4;
    const u16* polyBuf = reinterpret_cast<const u16*>(cur);
    cur += polyCount * 2;

    u32 vertCount = hecl::SBig(*reinterpret_cast<const u32*>(cur));
    cur += 4;
    const float* vertBuf = reinterpret_cast<const float*>(cur);

    return std::make_unique<CAreaOctTree>(aabb, nodeType, reinterpret_cast<const u8*>(buf + 8), treeBuf,
                                          matCount, matBuf, vertMatsBuf, edgeMatsBuf, polyMatsBuf,
                                          edgeCount, edgeBuf, polyCount, polyBuf, vertCount, vertBuf);
}

}
