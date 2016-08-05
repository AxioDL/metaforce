#include "CAreaOctTree.hpp"

namespace urde
{

CAreaOctTree::CAreaOctTree(const zeus::CAABox& aabb, Node::ETreeType treeType, u8* buf, void* treeBuf,
                           u32 matCount, u32* materials, u8* vertMats, u8* edgeMats, u8* polyMats,
                           u32 edgeCount, CCollisionEdge* edges, u32 polyCount, u16* polyEdges,
                           u32 vertCount, zeus::CVector3f* verts)
: x0_aabb(aabb), x18_treeType(treeType), x1c_buf(buf), x20_treeBuf(treeBuf),
  x24_matCount(matCount), x28_materials(materials), x2c_vertMats(vertMats),
  x30_edgeMats(edgeMats), x34_polyMats(polyMats), x38_edgeCount(edgeCount),
  x40_polyCount(polyCount), x44_polyEdges(polyEdges),
  x48_vertCount(vertCount)
{
    {
        x3c_edges.reserve(edgeCount);
        athena::io::MemoryReader r(edges, edgeCount * 4);
        for (u32 i=0 ; i<vertCount ; ++i)
            x3c_edges.emplace_back(r);
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

    void* treeBuf = cur;
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

    return std::make_unique<CAreaOctTree>(aabb, nodeType, reinterpret_cast<u8*>(buf), treeBuf,
                                          matCount, matBuf, vertMatsBuf, edgeMatsBuf, polyMatsBuf,
                                          edgeCount, edgeBuf, polyCount, polyBuf, vertCount, vertBuf);
}

}
