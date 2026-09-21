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
    char* x0_buffer;
    uint x4_size;
    uint x8_offset;

  public:
    CSimpleAllocator(uint size);
    ~CSimpleAllocator();
    void* Alloc(size_t size);
    uint GetPoolMemSize() const { return x4_size; }
  };

  class CLeafData {
    rstl::vector< ushort > x0_surface;

  public:
    CLeafData() {}
    CLeafData(const rstl::vector< ushort >& surface);
    CLeafData(CInputStream& in);

    const rstl::vector< ushort >& GetSurfaceVector() const { return x0_surface; }
    uint GetMemoryUsage() const;
  };

  class CNode {
    COBBox x0_obb;
    bool x3c_isLeaf;
    const CNode* x40_left;
    const CNode* x44_right;
    const CLeafData* x48_leaf;
    mutable bool x4c_hit;

    static CSimpleAllocator* spAllocator;

  public:
    CNode(const CTransform4f& xf, const CVector3f& extents, const CNode* left, const CNode* right,
          const CLeafData* leaf);
    CNode(CInputStream& in);
    ~CNode();

    const COBBox& GetOBB() const { return x0_obb; }
    bool IsLeaf() const { return x3c_isLeaf; }
    const CNode& GetLeftNode() const { return *x40_left; }
    const CNode& GetRightNode() const { return *x44_right; }
    const CLeafData& GetLeafData() const { return *x48_leaf; }
    void SetHit(bool h) const { x4c_hit = h; }
    bool WasHit() const { return x4c_hit; }

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
    rstl::vector< uint > x0_materials;
    rstl::vector< uchar > x10_vertMaterials;
    rstl::vector< uchar > x20_edgeMaterials;
    rstl::vector< uchar > x30_surfaceMaterials;
    rstl::vector< CCollisionEdge > x40_edges;
    rstl::vector< ushort > x50_surfaceIndices;
    rstl::vector< CVector3f > x60_vertices;

    SIndexData() {}
    SIndexData(CInputStream& in);
  };

  COBBTree(CInputStream& in);
  COBBTree(const SIndexData& indexData, const CNode* root);
  ~COBBTree();

  const CNode& GetRoot() const { return *x88_root; }

  CCollisionSurface GetSurface(ushort idx) const;
  CCollisionSurface GetTransformedSurface(const ushort idx, const CTransform4f& xf) const;
  const ushort* GetTriangleEdgeIndices(ushort idx) const;
  void GetTriangleVertexIndices(ushort idx, ushort* out) const;

  const CCollisionEdge& GetEdge(int idx) const { return x18_indexData.x40_edges[idx]; }
  const CVector3f& GetVert(int idx) const { return x18_indexData.x60_vertices[idx]; }
  uint GetVertMaterial(int idx) const {
    return x18_indexData.x0_materials[x18_indexData.x10_vertMaterials[idx]];
  }
  uint GetEdgeMaterial(int idx) const {
    return x18_indexData.x0_materials[x18_indexData.x20_edgeMaterials[idx]];
  }

  int NumSurfaceMaterials() const { return x18_indexData.x30_surfaceMaterials.size(); }
  CAABox CalculateLocalAABox() const;

  static rstl::auto_ptr< COBBTree > BuildOrientedBoundingBoxTree(const CVector3f& extent,
                                                                 const CVector3f& center);

private:
  uint x0_magic;
  uint x4_version;
  uint x8_memsize;
  CSimpleAllocator xc_allocator;
  SIndexData x18_indexData;
  const CNode* x88_root;
};

CHECK_SIZEOF(COBBTree, 0x8c)
NESTED_CHECK_SIZEOF(COBBTree, CSimpleAllocator, 0xc)
NESTED_CHECK_SIZEOF(COBBTree, CLeafData, 0x10)
NESTED_CHECK_SIZEOF(COBBTree, CNode, 0x50)
NESTED_CHECK_SIZEOF(COBBTree, SIndexData, 0x70)

#endif // _COBBTREE
