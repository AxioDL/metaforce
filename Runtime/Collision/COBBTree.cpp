#include "Runtime/Collision/COBBTree.hpp"

#include <array>

#include "Runtime/Collision/CCollidableOBBTreeGroup.hpp"

namespace urde {
namespace {
constexpr std::array<u8, 18> DefaultEdgeMaterials{
    2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 2, 2,
};

constexpr std::array<u8, 12> DefaultSurfaceMaterials{
    0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
};

constexpr std::array<CCollisionEdge, 18> DefaultEdges{{
    {4, 1},
    {1, 5},
    {5, 4},
    {4, 0},
    {0, 1},
    {7, 2},
    {2, 6},
    {6, 7},
    {7, 3},
    {3, 2},
    {6, 0},
    {4, 6},
    {2, 0},
    {5, 3},
    {7, 5},
    {1, 3},
    {6, 5},
    {0, 3},
}};

constexpr std::array<u16, 36> DefaultSurfaceIndices{
    0,  1, 2,  0,  3, 4,  5,  6,  7, 5,  8,  9, 10, 3,  11, 10, 6,  12,
    13, 8, 14, 13, 1, 15, 16, 14, 7, 16, 11, 2, 17, 15, 4,  17, 12, 9,
};


/* This is exactly what retro did >.< */
u32 verify_deaf_babe(CInputStream& in) { return in.readUint32Big(); }

/* This is exactly what retro did >.< */
u32 verify_version(CInputStream& in) { return in.readUint32Big(); }
} // Anonymous namespace

COBBTree::COBBTree(CInputStream& in)
: x0_magic(verify_deaf_babe(in))
, x4_version(verify_version(in))
, x8_memsize(in.readUint32())
, x18_indexData(in)
, x88_root(std::make_unique<CNode>(in)) {}

std::unique_ptr<COBBTree> COBBTree::BuildOrientedBoundingBoxTree(const zeus::CVector3f& extent,
                                                                 const zeus::CVector3f& center) {
  zeus::CAABox aabb(extent * -0.5f + center, extent * 0.5f + center);
  std::unique_ptr<COBBTree> ret = std::make_unique<COBBTree>();
  COBBTree::SIndexData& idxData = ret->x18_indexData;
  idxData.x0_materials.reserve(3);
  idxData.x0_materials.push_back(0x40180000);
  idxData.x0_materials.push_back(0x42180000);
  idxData.x0_materials.push_back(0x41180000);
  idxData.x10_vertMaterials = std::vector<u8>(8, u8(0));
  idxData.x20_edgeMaterials = std::vector<u8>(DefaultEdgeMaterials.cbegin(), DefaultEdgeMaterials.cend());
  idxData.x30_surfaceMaterials = std::vector<u8>(DefaultSurfaceMaterials.cbegin(), DefaultSurfaceMaterials.cend());
  idxData.x40_edges = std::vector<CCollisionEdge>(DefaultEdges.cbegin(), DefaultEdges.cend());
  idxData.x50_surfaceIndices = std::vector<u16>(DefaultSurfaceIndices.cbegin(), DefaultSurfaceIndices.cend());
  idxData.x60_vertices.reserve(8);
  for (int i = 0; i < 8; ++i)
    idxData.x60_vertices.push_back(aabb.getPoint(i));
  std::vector<u16> surface;
  surface.reserve(12);
  for (int i = 0; i < 12; ++i)
    surface.push_back(i);
  ret->x88_root = std::make_unique<CNode>(zeus::CTransform::Translate(center), extent * 0.5f,
    std::unique_ptr<CNode>{}, std::unique_ptr<CNode>{}, std::make_unique<CLeafData>(std::move(surface)));
  return ret;
}

CCollisionSurface COBBTree::GetSurface(u16 idx) const {
  int surfIdx = idx * 3;
  CCollisionEdge e0 = x18_indexData.x40_edges[x18_indexData.x50_surfaceIndices[surfIdx]];
  CCollisionEdge e1 = x18_indexData.x40_edges[x18_indexData.x50_surfaceIndices[surfIdx + 1]];
  u16 vert1 = e0.GetVertIndex1();
  u16 vert2 = e0.GetVertIndex2();
  u16 vert3 = e1.GetVertIndex1();

  if (vert3 == vert1 || vert3 == vert2)
    vert3 = e1.GetVertIndex2();

  u32 mat = x18_indexData.x0_materials[x18_indexData.x30_surfaceMaterials[idx]];

  if ((mat & 0x2000000) != 0) {
    return CCollisionSurface(x18_indexData.x60_vertices[vert2], x18_indexData.x60_vertices[vert1],
                             x18_indexData.x60_vertices[vert3], mat);
  }
  return CCollisionSurface(x18_indexData.x60_vertices[vert1], x18_indexData.x60_vertices[vert2],
                           x18_indexData.x60_vertices[vert3], mat);
}

void COBBTree::GetTriangleVertexIndices(u16 idx, u16 indicesOut[3]) const {
  const CCollisionEdge& e0 = x18_indexData.x40_edges[x18_indexData.x50_surfaceIndices[idx * 3]];
  const CCollisionEdge& e1 = x18_indexData.x40_edges[x18_indexData.x50_surfaceIndices[idx * 3 + 1]];
  indicesOut[2] = (e1.GetVertIndex1() != e0.GetVertIndex1() && e1.GetVertIndex1() != e0.GetVertIndex2())
                      ? e1.GetVertIndex1()
                      : e1.GetVertIndex2();

  u32 material = x18_indexData.x0_materials[x18_indexData.x30_surfaceMaterials[idx]];
  if (material & 0x2000000) {
    indicesOut[0] = e0.GetVertIndex2();
    indicesOut[1] = e0.GetVertIndex1();
  } else {
    indicesOut[0] = e0.GetVertIndex1();
    indicesOut[1] = e0.GetVertIndex2();
  }
}

CCollisionSurface COBBTree::GetTransformedSurface(u16 idx, const zeus::CTransform& xf) const {
  int surfIdx = idx * 3;
  CCollisionEdge e0 = x18_indexData.x40_edges[x18_indexData.x50_surfaceIndices[surfIdx]];
  CCollisionEdge e1 = x18_indexData.x40_edges[x18_indexData.x50_surfaceIndices[surfIdx + 1]];
  u16 vert1 = e0.GetVertIndex1();
  u16 vert2 = e0.GetVertIndex2();
  u16 vert3 = e1.GetVertIndex1();

  if (vert3 == vert1 || vert3 == vert2)
    vert3 = e1.GetVertIndex2();

  u32 mat = x18_indexData.x0_materials[x18_indexData.x30_surfaceMaterials[idx]];

  if ((mat & 0x2000000) != 0) {
    return CCollisionSurface(xf * x18_indexData.x60_vertices[vert2], xf * x18_indexData.x60_vertices[vert1],
                             xf * x18_indexData.x60_vertices[vert3], mat);
  }
  return CCollisionSurface(xf * x18_indexData.x60_vertices[vert1], xf * x18_indexData.x60_vertices[vert2],
                           xf * x18_indexData.x60_vertices[vert3], mat);
}

zeus::CAABox COBBTree::CalculateLocalAABox() const { return CalculateAABox(zeus::CTransform()); }

zeus::CAABox COBBTree::CalculateAABox(const zeus::CTransform& xf) const {
  if (x88_root)
    return x88_root->GetOBB().calculateAABox(xf);
  return zeus::CAABox();
}

COBBTree::SIndexData::SIndexData(CInputStream& in) {
  u32 count = in.readUint32Big();
  x0_materials.reserve(count);
  for (u32 i = 0; i < count; i++) {
    x0_materials.emplace_back(in.readUint32Big());
  }

  count = in.readUint32Big();
  for (u32 i = 0; i < count; i++) {
    x10_vertMaterials.emplace_back(in.readUByte());
  }
  count = in.readUint32Big();
  for (u32 i = 0; i < count; i++) {
    x20_edgeMaterials.emplace_back(in.readUByte());
  }
  count = in.readUint32Big();
  for (u32 i = 0; i < count; i++) {
    x30_surfaceMaterials.emplace_back(in.readUByte());
  }

  count = in.readUint32Big();
  for (u32 i = 0; i < count; i++) {
    x40_edges.emplace_back(in);
  }

  count = in.readUint32Big();
  for (u32 i = 0; i < count; i++) {
    x50_surfaceIndices.emplace_back(in.readUint16Big());
  }

  count = in.readUint32Big();
  for (u32 i = 0; i < count; i++) {
    x60_vertices.emplace_back(zeus::CVector3f::ReadBig(in));
  }
}

COBBTree::CNode::CNode(const zeus::CTransform& xf, const zeus::CVector3f& point,
                       std::unique_ptr<CNode>&& left, std::unique_ptr<CNode>&& right,
                       std::unique_ptr<CLeafData>&& leaf)
: x0_obb(xf, point)
, x3c_isLeaf(leaf.operator bool())
, x40_left(std::move(left))
, x44_right(std::move(right))
, x48_leaf(std::move(leaf)) {}

COBBTree::CNode::CNode(CInputStream& in) {
  x0_obb = zeus::COBBox::ReadBig(in);
  x3c_isLeaf = in.readBool();
  if (x3c_isLeaf)
    x48_leaf = std::make_unique<CLeafData>(in);
  else {
    x40_left = std::make_unique<CNode>(in);
    x44_right = std::make_unique<CNode>(in);
  }
}

size_t COBBTree::CNode::GetMemoryUsage() const {
  size_t ret = 0;
  if (x3c_isLeaf)
    ret = x48_leaf->GetMemoryUsage() + /*sizeof(CNode)*/ 80;
  else {
    if (x40_left)
      ret = x40_left->GetMemoryUsage() + /*sizeof(CNode)*/ 80;
    if (x44_right)
      ret += x44_right->GetMemoryUsage();
  }

  return (ret + 3) & ~3;
}

COBBTree::CLeafData::CLeafData(std::vector<u16>&& surface) : x0_surface(std::move(surface)) {}

const std::vector<u16>& COBBTree::CLeafData::GetSurfaceVector() const { return x0_surface; }

size_t COBBTree::CLeafData::GetMemoryUsage() const {
  const size_t ret = (x0_surface.size() * 2) + /*sizeof(CLeafData)*/ 16;
  return (ret + 3) & ~3;
}

COBBTree::CLeafData::CLeafData(CInputStream& in) {
  const u32 edgeCount = in.readUint32Big();
  for (u32 i = 0; i < edgeCount; i++) {
    x0_surface.emplace_back(in.readUint16Big());
  }
}

} // namespace urde
