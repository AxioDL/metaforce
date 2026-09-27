#include "WorldFormat/CAreaOctTree.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#if TARGET_LITTLE_ENDIAN
#include "rstl/auto_ptr.hpp"
#endif

static CAABox BoxFromIndex(int index, const CVector3f& a, const CVector3f& b, const CVector3f& c) {
  switch (index) {
  case 0:
    return CAABox(a, b);
  case 1:
    return CAABox(CVector3f(b.GetX(), a.GetY(), a.GetZ()), CVector3f(c.GetX(), b.GetY(), b.GetZ()));
  case 2:
    return CAABox(CVector3f(a.GetX(), b.GetY(), a.GetZ()), CVector3f(b.GetX(), c.GetY(), b.GetZ()));
  case 3:
    return CAABox(CVector3f(b.GetX(), b.GetY(), a.GetZ()), CVector3f(c.GetX(), c.GetY(), b.GetZ()));
  case 4:
    return CAABox(CVector3f(a.GetX(), a.GetY(), b.GetZ()), CVector3f(b.GetX(), b.GetY(), c.GetZ()));
  case 5:
    return CAABox(CVector3f(b.GetX(), a.GetY(), b.GetZ()), CVector3f(c.GetX(), b.GetY(), c.GetZ()));
  case 6:
    return CAABox(CVector3f(a.GetX(), b.GetY(), b.GetZ()), CVector3f(b.GetX(), c.GetY(), c.GetZ()));
  case 7:
    return CAABox(b, c);
  default:
    return CAABox(a, c);
  }
}

CAreaOctTree::Node CAreaOctTree::Node::GetChild(int index) const {
  ETreeType type = GetChildType(index);
  const uint* offsets = reinterpret_cast< const uint* >(mPtr + sizeof(uint));
  const void* node = mPtr + 9 * sizeof(uint) + CBasics::SwapBytes(offsets[index]);
  if (type == kTT_Leaf) {
#if TARGET_LITTLE_ENDIAN
    CMemoryInStream in(node, sizeof(CAABox));
    CAABox bounds(in);
#else
    CAABox bounds = *reinterpret_cast< const CAABox* >(node);
#endif
    return Node(node, bounds, GetOwner(), type);
  }
  const CVector3f center = 0.5f * (mAabb.GetMinPoint() + mAabb.GetMaxPoint());
  CAABox bounds = BoxFromIndex(index, mAabb.GetMinPoint(), center, mAabb.GetMaxPoint());
  return Node(node, bounds, GetOwner(), type);
}

CAreaOctTree::TriListReference CAreaOctTree::Node::GetTriangleArray() const {
#if NONMATCHING
  static const ushort skDeadArray[sizeof(CAABox) / sizeof(ushort) + 1] = {0};
#else
  static const ushort skDeadArray[2] = {0, 0};
#endif
  if (GetTreeType() != kTT_Leaf) {
    return TriListReference(skDeadArray);
  }

  return TriListReference(mPtr);
}

CAreaOctTree::CAreaOctTree(const CAABox& bounds, Node::ETreeType treeType, uchar* buf,
                           void* treeBuf, uint materialCount, uint* materials,
                           uchar* vertexMaterials, uchar* edgeMaterials, uchar* polyMaterials,
                           uint edgeCount, CCollisionEdge* edges, uint polyCount, ushort* polyEdges,
                           uint vertexCount, CVector3f* vertices)
: mAabb(bounds)
, mTreeType(treeType)
, mBuf(buf)
, mTreeBuf(treeBuf)
, mMatCount(materialCount)
, mMaterials(materials)
, mVertMats(vertexMaterials)
, mEdgeMats(edgeMaterials)
, mPolyMats(polyMaterials)
, mEdgeCount(edgeCount)
, mEdges(edges)
, mPolyCount(polyCount)
, mPolyEdges(polyEdges)
, mVertCount(vertexCount)
, mVerts(vertices) {}

#if TARGET_LITTLE_ENDIAN
template < typename T >
static void ReadCollisionArray(rstl::vector< T >& out, CInputStream& in) {
  const uint count = in.ReadLong();
  out.reserve(count);
  for (uint i = 0; i < count; ++i) {
    out.push_back(in.Get< T >());
  }
}
#endif

void CAreaOctTree::MakeFromMemory(void* buf, const uint bufLen, CAreaOctTree** treeOut,
                                  bool* valid) {
  CMemoryInStream in(buf, bufLen, CMemoryInStream::kOS_NotOwned);
  in.ReadLong();
  in.ReadLong();
  CAABox bounds(in);
  Node::ETreeType treeType = static_cast< Node::ETreeType >(in.ReadLong());
  uint treeSize = in.ReadLong();

  uchar* treeBuf = static_cast< uchar* >(buf) + in.GetReadPosition();
#if TARGET_LITTLE_ENDIAN
  // The accessors expose native references and arrays. Decode these separately;
  // the packed resource does not guarantee alignment for every array element.
  CMemoryInStream data(treeBuf + treeSize, bufLen - in.GetReadPosition() - treeSize);
  rstl::auto_ptr< CAreaOctTree > tree(rs_new CAreaOctTree(
      bounds, treeType, static_cast< uchar* >(buf), treeBuf, 0, nullptr, nullptr, nullptr,
      nullptr, 0, nullptr, 0, nullptr, 0, nullptr));
  ReadCollisionArray(tree->mNativeMaterials, data);
  ReadCollisionArray(tree->mNativeVertexMaterials, data);
  ReadCollisionArray(tree->mNativeEdgeMaterials, data);
  ReadCollisionArray(tree->mNativeTriangleMaterials, data);
  ReadCollisionArray(tree->mNativeEdges, data);
  ReadCollisionArray(tree->mNativeTriangleEdges, data);
  ReadCollisionArray(tree->mNativeVertices, data);

  tree->mMatCount = tree->mNativeMaterials.size();
  tree->mMaterials = tree->mNativeMaterials.data();
  tree->mVertMats = tree->mNativeVertexMaterials.data();
  tree->mEdgeMats = tree->mNativeEdgeMaterials.data();
  tree->mPolyMats = tree->mNativeTriangleMaterials.data();
  tree->mEdgeCount = tree->mNativeEdges.size();
  tree->mEdges = tree->mNativeEdges.data();
  tree->mPolyCount = tree->mNativeTriangleEdges.size() / 3;
  tree->mPolyEdges = tree->mNativeTriangleEdges.data();
  tree->mVertCount = tree->mNativeVertices.size();
  tree->mVerts = tree->mNativeVertices.data();
  *treeOut = tree.release();
#else
  uint* materialHeader = reinterpret_cast< uint* >(treeBuf + treeSize);
  uint matCount = *materialHeader;
  uint* materials = materialHeader + 1;

  uint* vertexMaterialHeader = materials + matCount;
  uchar* vertexMaterials = reinterpret_cast< uchar* >(vertexMaterialHeader + 1);
  uint* edgeMaterialHeader = reinterpret_cast< uint* >(vertexMaterials + *vertexMaterialHeader);
  uchar* edgeMaterials = reinterpret_cast< uchar* >(edgeMaterialHeader + 1);
  uint* polyMaterialHeader = reinterpret_cast< uint* >(edgeMaterials + *edgeMaterialHeader);
  uchar* polyMaterials = reinterpret_cast< uchar* >(polyMaterialHeader + 1);

  uint* edgeHeader = reinterpret_cast< uint* >(polyMaterials + *polyMaterialHeader);
  uint edgeCount = *edgeHeader;
  CCollisionEdge* edges = reinterpret_cast< CCollisionEdge* >(edgeHeader + 1);
  uint* polyHeader = reinterpret_cast< uint* >(edges + edgeCount);
  uint polyCount = *polyHeader / 3;
  ushort* polyEdges = reinterpret_cast< ushort* >(polyHeader + 1);
  uint* vertexHeader = reinterpret_cast< uint* >(polyEdges + polyCount * 3);
  uint vertexCount = *vertexHeader;
  CVector3f* vertices = reinterpret_cast< CVector3f* >(vertexHeader + 1);

  *treeOut = rs_new CAreaOctTree(bounds, treeType, static_cast< uchar* >(buf), treeBuf, matCount,
                                 materials, vertexMaterials, edgeMaterials, polyMaterials,
                                 edgeCount, edges, polyCount, polyEdges, vertexCount, vertices);
#endif
  *valid = true;
}

CCollisionSurface CAreaOctTree::GetMasterListTriangle(ushort idx) const {
  int start = idx * 3;
  ushort edgeIndex0 = mPolyEdges[start];
  ushort edgeIndex1 = mPolyEdges[start + 1];
  const CCollisionEdge& edge0 = mEdges[edgeIndex0];
  const CCollisionEdge& edge1 = mEdges[edgeIndex1];
  ushort vert2 = edge1.GetVertIndex1() != edge0.GetVertIndex1() &&
                         edge1.GetVertIndex1() != edge0.GetVertIndex2()
                     ? edge1.GetVertIndex1()
                     : edge1.GetVertIndex2();
  uint material = mMaterials[mPolyMats[idx]];
  if ((material & 0x2000000) != 0) {
    const CVector3f& thirdVertex = mVerts[vert2];
    return CCollisionSurface(mVerts[edge0.GetVertIndex2()], mVerts[edge0.GetVertIndex1()],
                             thirdVertex, material);
  }
  const CVector3f& thirdVertex = mVerts[vert2];
  return CCollisionSurface(mVerts[edge0.GetVertIndex1()], mVerts[edge0.GetVertIndex2()],
                           thirdVertex, material);
}

void CAreaOctTree::GetTriangleVertexIndices(ushort idx, ushort indicesOut[3]) const {
  int start = idx * 3;
  ushort edgeIndex0 = mPolyEdges[start];
  ushort edgeIndex1 = mPolyEdges[start + 1];
  const CCollisionEdge& edge0 = mEdges[edgeIndex0];
  const CCollisionEdge& edge1 = mEdges[edgeIndex1];
  indicesOut[2] = edge1.GetVertIndex1() != edge0.GetVertIndex1() &&
                          edge1.GetVertIndex1() != edge0.GetVertIndex2()
                      ? edge1.GetVertIndex1()
                      : edge1.GetVertIndex2();
  uint material = mMaterials[mPolyMats[idx]];
  if ((material & 0x2000000) != 0) {
    indicesOut[0] = edge0.GetVertIndex2();
    indicesOut[1] = edge0.GetVertIndex1();
  } else {
    indicesOut[0] = edge0.GetVertIndex1();
    indicesOut[1] = edge0.GetVertIndex2();
  }
}

const ushort* CAreaOctTree::GetTriangleEdgeIndices(ushort idx) const {
  return &mPolyEdges[idx * 3];
}
