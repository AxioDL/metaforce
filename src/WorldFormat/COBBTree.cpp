#include "WorldFormat/COBBTree.hpp"
#include "WorldFormat/CCollisionSurface.hpp"

#include "Collision/CMaterialList.hpp"
#include "Kyoto/Streams/CInputStream.hpp"
#include "rstl/algorithm.hpp"

COBBTree::CSimpleAllocator* COBBTree::CNode::spAllocator = nullptr;

COBBTree::SIndexData::SIndexData(CInputStream& in)
: mMaterials(in)
, mVertMaterials(in)
, mEdgeMaterials(in)
, mSurfaceMaterials(in)
, mEdges(in)
, mSurfaceIndices(in)
, mVertices(in) {}

COBBTree::COBBTree(const SIndexData& indexData, const CNode* root)
: mMemsize(root->GetMemoryUsage())
, mAllocator(0)
, mIndexData(indexData)
, mRoot(root) {
  CNode::SetAllocator(nullptr);
}

uint verify_deaf_babe(CInputStream& in) { return in.Get< uint >(); }
uint verify_version(CInputStream& in) { return in.Get< uint >(); }

COBBTree::COBBTree(CInputStream& in)
: mMagic(verify_deaf_babe(in))
, mVersion(verify_version(in))
, mMemsize(in.Get< uint >())
, mAllocator(mMemsize)
, mIndexData(in)
, mRoot(nullptr) {
  CNode::SetAllocator(&mAllocator);

  mRoot = rs_new CNode(in);
}

COBBTree::~COBBTree() {
  if (mAllocator.GetPoolMemSize() != 0) {
    CNode::SetAllocator(&mAllocator);
  } else {
    CNode::SetAllocator(nullptr);
  }
  delete mRoot;
}

CAABox COBBTree::CalculateLocalAABox() const {
  if (mRoot != nullptr) {
    return mRoot->GetOBB().CalculateAABox(CTransform4f::Identity());
  }

  return CAABox(0.f, 0.f, 0.f, 0.f, 0.f, 0.f);
}

CCollisionSurface COBBTree::GetSurface(const ushort index) const {
  const int surfIdx = index * 3;
  const CCollisionEdge& e0 = mIndexData.mEdges[mIndexData.mSurfaceIndices[surfIdx]];
  const CCollisionEdge& e1 = mIndexData.mEdges[mIndexData.mSurfaceIndices[surfIdx + 1]];
  const ushort v2 =
      (e1.GetVertIndex1() != e0.GetVertIndex1() && e1.GetVertIndex1() != e0.GetVertIndex2())
          ? e1.GetVertIndex1()
          : e1.GetVertIndex2();
  const uint material = mIndexData.mMaterials[mIndexData.mSurfaceMaterials[index]];
  if ((material & 0x2000000) != 0) {
    const CVector3f& thirdVertex = mIndexData.mVertices[v2];
    return CCollisionSurface(mIndexData.mVertices[e0.GetVertIndex2()],
                             mIndexData.mVertices[e0.GetVertIndex1()], thirdVertex, material);
  }
  const CVector3f& thirdVertex = mIndexData.mVertices[v2];
  return CCollisionSurface(mIndexData.mVertices[e0.GetVertIndex1()],
                           mIndexData.mVertices[e0.GetVertIndex2()], thirdVertex, material);
}

CCollisionSurface COBBTree::GetTransformedSurface(const ushort index,
                                                  const CTransform4f& xf) const {
  const int surfIdx = index * 3;
  const CCollisionEdge& e0 = mIndexData.mEdges[mIndexData.mSurfaceIndices[surfIdx]];
  const CCollisionEdge& e1 = mIndexData.mEdges[mIndexData.mSurfaceIndices[surfIdx + 1]];
  const ushort v2 =
      (e1.GetVertIndex1() != e0.GetVertIndex1() && e1.GetVertIndex1() != e0.GetVertIndex2())
          ? e1.GetVertIndex1()
          : e1.GetVertIndex2();
  const uint material = mIndexData.mMaterials[mIndexData.mSurfaceMaterials[index]];
  if ((material & 0x2000000) != 0) {
    const CVector3f& thirdVertex = mIndexData.mVertices[v2];
    return CCollisionSurface(xf * mIndexData.mVertices[e0.GetVertIndex2()],
                             xf * mIndexData.mVertices[e0.GetVertIndex1()], xf * thirdVertex,
                             material);
  }
  const CVector3f& thirdVertex = mIndexData.mVertices[v2];
  return CCollisionSurface(xf * mIndexData.mVertices[e0.GetVertIndex1()],
                           xf * mIndexData.mVertices[e0.GetVertIndex2()], xf * thirdVertex,
                           material);
}

rstl::auto_ptr< COBBTree > COBBTree::BuildOrientedBoundingBoxTree(const CVector3f& extent,
                                                                  const CVector3f& center) {
  const CVector3f halfExtent = extent * 0.5f;
  const CVector3f negHalfExtent = extent * -0.5f;
  const CAABox aabb(negHalfExtent + center, halfExtent + center);
  SIndexData indexData;
  indexData.mMaterials.reserve(3);
  indexData.mMaterials.push_back((1u << kMT_Wall) | (1u << kMT_NoPlatformCollision) |
                                   (1u << kMT_Solid));
  indexData.mMaterials.push_back((1u << kMT_Wall) | (1u << kMT_RedundantEdgeOrFlippedTri) |
                                   (1u << kMT_NoPlatformCollision) | (1u << kMT_Solid));
  indexData.mMaterials.push_back((1u << kMT_Wall) | (1u << kMT_NoEdgeCollision) |
                                   (1u << kMT_NoPlatformCollision) | (1u << kMT_Solid));
  indexData.mVertMaterials = rstl::vector< uchar >(8, static_cast< uchar >(0));
  static const uchar kEdgeMaterials[] = {
      2, 0, 0, 0, 0, 2, 0, 0, 0, 0, 2, 0, 0, 2, 0, 0, 2, 2,
  };
  indexData.mEdgeMaterials.reserve(ARRAY_SIZE(kEdgeMaterials));
  for (size_t i = 0; i < ARRAY_SIZE(kEdgeMaterials); ++i) {
    indexData.mEdgeMaterials.push_back(kEdgeMaterials[i]);
  }

  static const uchar kTriangleMaterials[] = {
      0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1,
  };
  indexData.mSurfaceMaterials.reserve(ARRAY_SIZE(kTriangleMaterials));
  for (size_t i = 0; i < ARRAY_SIZE(kTriangleMaterials); ++i) {
    indexData.mSurfaceMaterials.push_back(kTriangleMaterials[i]);
  }

  static const ushort kEdges[] = {
      4, 1, 1, 5, 5, 4, 4, 0, 0, 1, 7, 2, 2, 6, 6, 7, 7, 3,
      3, 2, 6, 0, 4, 6, 2, 0, 5, 3, 7, 5, 1, 3, 6, 5, 0, 3,
  };
  indexData.mEdges.reserve(ARRAY_SIZE(kEdges) / 2);

  for (size_t i = 0; i < ARRAY_SIZE(kEdges); i += 2) {
    indexData.mEdges.push_back(CCollisionEdge(kEdges[i], kEdges[i + 1]));
  }

  static const ushort kTriangleEdgeIndices[] = {
      0,  1, 2,  0,  3, 4,  5,  6,  7, 5,  8,  9, 10, 3,  11, 10, 6,  12,
      13, 8, 14, 13, 1, 15, 16, 14, 7, 16, 11, 2, 17, 15, 4,  17, 12, 9,
  };

  indexData.mSurfaceIndices.reserve(ARRAY_SIZE(kTriangleEdgeIndices));
  for (size_t i = 0; i < ARRAY_SIZE(kTriangleEdgeIndices); i++) {
    indexData.mSurfaceIndices.push_back(kTriangleEdgeIndices[i]);
  }

  indexData.mVertices.reserve(8);
  for (int i = 0; i < 8; ++i) {
    indexData.mVertices.push_back(aabb.GetPoint(i));
  }

  rstl::vector< ushort > surface;
  surface.reserve(12);
  for (ushort i = 0; i < 12; ++i) {
    surface.push_back(i);
  }
  CNode::SetAllocator(nullptr);

  CLeafData* leafData = rs_new CLeafData(surface);
  CNode* root =
      rs_new CNode(CTransform4f::Translate(center), halfExtent, nullptr, nullptr, leafData);

  return rs_new COBBTree(indexData, root);
}

void COBBTree::GetTriangleVertexIndices(const ushort index, ushort out[3]) const {
  const int surfIdx = index * 3;
  const CCollisionEdge& e0 = mIndexData.mEdges[mIndexData.mSurfaceIndices[surfIdx]];
  const CCollisionEdge& e1 = mIndexData.mEdges[mIndexData.mSurfaceIndices[surfIdx + 1]];

  out[2] = (e1.GetVertIndex1() != e0.GetVertIndex1() && e1.GetVertIndex1() != e0.GetVertIndex2())
               ? e1.GetVertIndex1()
               : e1.GetVertIndex2();

  const uint material = mIndexData.mMaterials[mIndexData.mSurfaceMaterials[index]];
  if ((material & 0x2000000) != 0) {
    out[0] = e0.GetVertIndex2();
    out[1] = e0.GetVertIndex1();
  } else {
    out[0] = e0.GetVertIndex1();
    out[1] = e0.GetVertIndex2();
  }
}

const ushort* COBBTree::GetTriangleEdgeIndices(const ushort index) const {
  return &mIndexData.mSurfaceIndices[index * 3];
}

COBBTree::CNode::CNode(const CTransform4f& xf, const CVector3f& point, const CNode* left,
                       const CNode* right, const CLeafData* leaf)
: mObb(xf, point)
, mIsLeaf(leaf != nullptr)
, mLeft(left)
, mRight(right)
, mLeaf(leaf)
#if NONMATCHING
, mHit(false)
#endif
{}

COBBTree::CNode::CNode(CInputStream& in)
: mObb(in)
, mIsLeaf(in.Get< bool >())
, mLeft(mIsLeaf ? nullptr : rs_new CNode(in))
, mRight(mIsLeaf ? nullptr : rs_new CNode(in))
, mLeaf(mIsLeaf ? rs_new CLeafData(in) : nullptr)
#if NONMATCHING
, mHit(false)
#endif
{}

COBBTree::CNode::~CNode() {
  delete mLeft;
  delete mRight;
  delete mLeaf;
}

uint COBBTree::CNode::GetMemoryUsage() const {
  uint ret = sizeof(CNode);
  if (mIsLeaf) {
    ret += mLeaf->GetMemoryUsage();
  } else {
    ret += mLeft->GetMemoryUsage();
    ret += mRight->GetMemoryUsage();
  }

  if (ret & 3) {
    ret += 4 - (ret & 3);
  }

  return ret;
}

void COBBTree::CNode::SetAllocator(CSimpleAllocator* allocator) { spAllocator = allocator; }

void* COBBTree::CNode::operator new(size_t size, const char* file, int line) {
  if (spAllocator == nullptr) {
    return rs_new char[size];
  }
  return spAllocator->Alloc(size);
}

void COBBTree::CNode::operator delete(void* ptr, size_t size) {
  if (spAllocator == nullptr && ptr != nullptr) {
    delete[] static_cast< char* >(ptr);
  }
}

COBBTree::CLeafData::CLeafData(const rstl::vector< ushort >& surface) : mSurface(surface) {}
COBBTree::CLeafData::CLeafData(CInputStream& in) : mSurface(in) {}

uint COBBTree::CLeafData::GetMemoryUsage() const {
  uint ret = sizeof(CLeafData) + mSurface.size() * sizeof(ushort);

  if (ret & 3) {
    ret += 4 - (ret & 3);
  }

  return ret;
}

COBBTree::CSimpleAllocator::CSimpleAllocator(uint size)
: mBuffer(rs_new char[size])
, mSize(size)
, mOffset(0) {}

COBBTree::CSimpleAllocator::~CSimpleAllocator() {
  if (mBuffer) {
    delete[] mBuffer;
  }
}

void* COBBTree::CSimpleAllocator::Alloc(const size_t size) {
  void* ret = mBuffer + mOffset;
  mOffset += size;
  if (mOffset & 3) {
    mOffset += 4 - (mOffset & 3);
  }

  return ret;
}
