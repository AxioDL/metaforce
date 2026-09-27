#ifndef _COBBTREE
#define _COBBTREE

#include "types.h"

#include "Collision/COBBox.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "WorldFormat/CCollisionEdge.hpp"

#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

class CAABox;
class CCollisionSurface;
class CInputStream;
class CTransform4f;

class COBBTree {
public:
  class CSimpleAllocator {
    char* mBuffer;
    uint mSize;
    uint mOffset;

  public:
    CSimpleAllocator(uint size);
    ~CSimpleAllocator();
    void* Alloc(size_t size);
    uint GetPoolMemSize() const { return mSize; }
  };

  class CLeafData {
    rstl::vector< ushort > mSurface;

  public:
    CLeafData() {}
    CLeafData(const rstl::vector< ushort >& surface);
    CLeafData(CInputStream& in);

    const rstl::vector< ushort >& GetSurfaceVector() const { return mSurface; }
    uint GetMemoryUsage() const;
  };

  class CNode {
    COBBox mObb;
    bool mIsLeaf;
    const CNode* mLeft;
    const CNode* mRight;
    const CLeafData* mLeaf;
    mutable bool mHit;

    static CSimpleAllocator* spAllocator;

  public:
    CNode(const CTransform4f& xf, const CVector3f& extents, const CNode* left, const CNode* right,
          const CLeafData* leaf);
    CNode(CInputStream& in);
    ~CNode();

    const COBBox& GetOBB() const { return mObb; }
    bool IsLeaf() const { return mIsLeaf; }
    const CNode& GetLeftNode() const { return *mLeft; }
    const CNode& GetRightNode() const { return *mRight; }
    const CLeafData& GetLeafData() const { return *mLeaf; }
    void SetHit(bool h) const { mHit = h; }
    bool WasHit() const { return mHit; }

    uint GetMemoryUsage() const;

    static void SetAllocator(CSimpleAllocator* alloc);
    void* operator new(size_t size, const char* file, int line);
#if defined(TARGET_PC)
    void* operator new(size_t size) { return operator new(size, "COBBTree", 0); }
#endif
    void operator delete(void* ptr, size_t size);
    void operator delete(void* ptr, const char* file, int line);
  };

  struct SIndexData {
    rstl::vector< uint > mMaterials;
    rstl::vector< uchar > mVertMaterials;
    rstl::vector< uchar > mEdgeMaterials;
    rstl::vector< uchar > mSurfaceMaterials;
    rstl::vector< CCollisionEdge > mEdges;
    rstl::vector< ushort > mSurfaceIndices;
    rstl::vector< CVector3f > mVertices;

    SIndexData() {}
    SIndexData(CInputStream& in);
  };

  COBBTree(CInputStream& in);
  COBBTree(const SIndexData& indexData, const CNode* root);
  ~COBBTree();

  const CNode& GetRoot() const { return *mRoot; }

  CCollisionSurface GetSurface(ushort idx) const;
  CCollisionSurface GetTransformedSurface(const ushort idx, const CTransform4f& xf) const;
  const ushort* GetTriangleEdgeIndices(ushort idx) const;
  void GetTriangleVertexIndices(ushort idx, ushort* out) const;

  const CCollisionEdge& GetEdge(int idx) const { return mIndexData.mEdges[idx]; }
  const CVector3f& GetVert(int idx) const { return mIndexData.mVertices[idx]; }
  uint GetVertMaterial(int idx) const {
    return mIndexData.mMaterials[mIndexData.mVertMaterials[idx]];
  }
  uint GetEdgeMaterial(int idx) const {
    return mIndexData.mMaterials[mIndexData.mEdgeMaterials[idx]];
  }

  int NumSurfaceMaterials() const { return mIndexData.mSurfaceMaterials.size(); }
  CAABox CalculateLocalAABox() const;

  static rstl::auto_ptr< COBBTree > BuildOrientedBoundingBoxTree(const CVector3f& extent,
                                                                 const CVector3f& center);

private:
  uint mMagic;
  uint mVersion;
  uint mMemsize;
  CSimpleAllocator mAllocator;
  SIndexData mIndexData;
  const CNode* mRoot;
};

CHECK_SIZEOF(COBBTree, 0x8c)
NESTED_CHECK_SIZEOF(COBBTree, CSimpleAllocator, 0xc)
NESTED_CHECK_SIZEOF(COBBTree, CLeafData, 0x10)
NESTED_CHECK_SIZEOF(COBBTree, CNode, 0x50)
NESTED_CHECK_SIZEOF(COBBTree, SIndexData, 0x70)

#endif // _COBBTREE
