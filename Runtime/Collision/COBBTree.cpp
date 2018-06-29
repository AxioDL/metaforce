#include "COBBTree.hpp"
#include "CCollidableOBBTreeGroup.hpp"

namespace urde
{
/* This is exactly what retro did >.< */
u32 verify_deaf_babe(CInputStream& in)
{
    return in.readUint32Big();
}

/* This is exactly what retro did >.< */
u32 verify_version(CInputStream& in)
{
    return in.readUint32Big();
}

COBBTree::COBBTree(const SIndexData& indexData, const CNode* root)
    : x18_indexData(indexData),
      x88_root((CNode*)root)
{
}

COBBTree::COBBTree(CInputStream& in)
    : x0_magic(verify_deaf_babe(in)),
      x4_version(verify_version(in)),
      x8_memsize(in.readUint32()),
      x18_indexData(in),
      x88_root(new CNode(in))
{
}

std::unique_ptr<CCollidableOBBTreeGroupContainer>
COBBTree::BuildOrientedBoundingBoxTree(const zeus::CVector3f& a, const zeus::CVector3f& b)
{
    return std::make_unique<CCollidableOBBTreeGroupContainer>(a, b);
}

CCollisionSurface COBBTree::GetSurface(u16 idx) const
{
    int surfIdx = idx * 3;
    CCollisionEdge e0 = x18_indexData.x40_edges[x18_indexData.x50_surfaceIndices[surfIdx]];
    CCollisionEdge e1 = x18_indexData.x40_edges[x18_indexData.x50_surfaceIndices[surfIdx + 1]];
    u16 vert1 = e0.GetVertIndex1();
    u16 vert2 = e0.GetVertIndex2();
    u16 vert3 = e1.GetVertIndex1();

    if (vert3 == vert1 || vert3 == vert2)
        vert3 = e1.GetVertIndex2();

    u32 mat = x18_indexData.x0_materials[x18_indexData.x30_surfaceMaterials[idx]];

    if ((mat & 0x2000000) != 0)
    {
        return CCollisionSurface(x18_indexData.x60_vertices[vert2], x18_indexData.x60_vertices[vert1],
                                 x18_indexData.x60_vertices[vert3], mat);
    }
    return CCollisionSurface(x18_indexData.x60_vertices[vert1], x18_indexData.x60_vertices[vert2],
                             x18_indexData.x60_vertices[vert3], mat);
}

void COBBTree::GetTriangleVertexIndices(u16 idx, u16 indicesOut[3]) const
{
    const CCollisionEdge& e0 = x18_indexData.x40_edges[x18_indexData.x50_surfaceIndices[idx*3]];
    const CCollisionEdge& e1 = x18_indexData.x40_edges[x18_indexData.x50_surfaceIndices[idx*3+1]];
    indicesOut[2] =
        (e1.GetVertIndex1() != e0.GetVertIndex1() && e1.GetVertIndex1() != e0.GetVertIndex2()) ?
        e1.GetVertIndex1() : e1.GetVertIndex2();

    u32 material = x18_indexData.x0_materials[x18_indexData.x30_surfaceMaterials[idx]];
    if (material & 0x2000000)
    {
        indicesOut[0] = e0.GetVertIndex2();
        indicesOut[1] = e0.GetVertIndex1();
    }
    else
    {
        indicesOut[0] = e0.GetVertIndex1();
        indicesOut[1] = e0.GetVertIndex2();
    }
}

CCollisionSurface COBBTree::GetTransformedSurface(u16 idx, const zeus::CTransform& xf) const
{
    int surfIdx = idx * 3;
    CCollisionEdge e0 = x18_indexData.x40_edges[x18_indexData.x50_surfaceIndices[surfIdx]];
    CCollisionEdge e1 = x18_indexData.x40_edges[x18_indexData.x50_surfaceIndices[surfIdx + 1]];
    u16 vert1 = e0.GetVertIndex1();
    u16 vert2 = e0.GetVertIndex2();
    u16 vert3 = e1.GetVertIndex1();

    if (vert3 == vert1 || vert3 == vert2)
        vert3 = e1.GetVertIndex2();

    u32 mat = x18_indexData.x0_materials[x18_indexData.x30_surfaceMaterials[idx]];

    if ((mat & 0x2000000) != 0)
    {
        return CCollisionSurface(xf * x18_indexData.x60_vertices[vert2], xf * x18_indexData.x60_vertices[vert1],
                                 xf * x18_indexData.x60_vertices[vert3], mat);
    }
    return CCollisionSurface(xf * x18_indexData.x60_vertices[vert1], xf * x18_indexData.x60_vertices[vert2],
                             xf * x18_indexData.x60_vertices[vert3], mat);
}

zeus::CAABox COBBTree::CalculateLocalAABox() const
{
    return CalculateAABox(zeus::CTransform::Identity());
}

zeus::CAABox COBBTree::CalculateAABox(const zeus::CTransform& xf) const
{
    if (x88_root)
        return x88_root->GetOBB().calculateAABox(xf);
    return zeus::CAABox::skInvertedBox;
}

COBBTree::SIndexData::SIndexData(CInputStream& in)
{
    u32 count = in.readUint32Big();
    x0_materials.reserve(count);
    for (u32 i = 0 ; i < count ; i++)
        x0_materials.push_back(in.readUint32Big());

    count = in.readUint32Big();
    for (u32 i = 0 ; i < count ; i++)
        x10_vertMaterials.push_back(in.readUByte());
    count = in.readUint32Big();
    for (u32 i = 0 ; i < count ; i++)
        x20_edgeMaterials.push_back(in.readUByte());
    count = in.readUint32Big();
    for (u32 i = 0 ; i < count ; i++)
        x30_surfaceMaterials.push_back(in.readUByte());

    count = in.readUint32Big();
    for (u32 i = 0 ; i < count ; i++)
        x40_edges.push_back(in);

    count = in.readUint32Big();
    for (u32 i = 0 ; i < count ; i++)
        x50_surfaceIndices.push_back(in.readUint16Big());

    count = in.readUint32Big();
    for (u32 i = 0 ; i < count ; i++)
        x60_vertices.push_back(zeus::CVector3f::ReadBig(in));
}

COBBTree::CNode::CNode(const zeus::CTransform& xf, const zeus::CVector3f& point,
                       const COBBTree::CNode* left, const COBBTree::CNode* right,
                       const COBBTree::CLeafData* leaf)
    : x0_obb(xf, point),
      x3c_isLeaf(leaf != nullptr)
{
    x40_left.reset((CNode*)left);
    x44_right.reset((CNode*)right);
    x48_leaf.reset((CLeafData*)leaf);
}

COBBTree::CNode::CNode(CInputStream& in)
{
    x0_obb = zeus::COBBox::ReadBig(in);
    x3c_isLeaf = in.readBool();
    if (x3c_isLeaf)
        x48_leaf.reset(new CLeafData(in));
    else
    {
        x40_left.reset(new CNode(in));
        x44_right.reset(new CNode(in));
    }
}

size_t COBBTree::CNode::GetMemoryUsage() const
{
    size_t ret = 0;
    if (x3c_isLeaf)
        ret = x48_leaf->GetMemoryUsage() + /*sizeof(CNode)*/ 80;
    else
    {
        if (x40_left)
            ret = x40_left->GetMemoryUsage() + /*sizeof(CNode)*/ 80;
        if (x44_right)
            ret += x44_right->GetMemoryUsage();
    }

    return (ret + 3) & ~3;
}

COBBTree::CLeafData::CLeafData(const std::vector<u16>& surface)
    : x0_surface(surface)
{
}

const std::vector<u16>& COBBTree::CLeafData::GetSurfaceVector() const
{
    return x0_surface;
}

size_t COBBTree::CLeafData::GetMemoryUsage() const
{
    size_t ret = (x0_surface.size() * 2) + /*sizeof(CLeafData)*/ 16;
    return (ret + 3) & ~3;
}

COBBTree::CLeafData::CLeafData(CInputStream& in)
{
    u32 edgeCount = in.readUint32Big();
    for (u32 i = 0 ; i < edgeCount ; i++)
        x0_surface.push_back(in.readUint16Big());
}

}
