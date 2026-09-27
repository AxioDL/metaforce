#ifndef _CAREAOCTTREE
#define _CAREAOCTTREE

#include "types.h"
#include "Kyoto/Basics/CBasics.hpp"

#include "WorldFormat/CCollisionSurface.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "Kyoto/Math/CPlane.hpp"

#include "rstl/optional_object.hpp"

#include "WorldFormat/CCollisionEdge.hpp"

#if TARGET_LITTLE_ENDIAN
#include "rstl/vector.hpp"
#endif

class CLine;
class CMaterialFilter;

class CAreaOctTree {
public:
  struct SRayResult {
    CPlane mPlane;
    rstl::optional_object< CCollisionSurface > mSurface;
    float mT;

    SRayResult() : mPlane(0.f, CUnitVector3f(1.f, 0.f, 0.f)), mT(0.f) {}
  };

  class TriListReference {
  public:
    explicit TriListReference(const void* ptr) : m_ptr(reinterpret_cast< const ushort* >(ptr)) {}
    explicit TriListReference(const ushort* ptr) : m_ptr(ptr) {}
    const ushort GetAt(int idx) const {
#if TARGET_LITTLE_ENDIAN
      return CBasics::SwapBytes(m_ptr[idx + kTriangleDataOffset]);
#else
      return m_ptr[idx + kTriangleDataOffset];
#endif
    }
    const ushort GetSize() const { return CBasics::SwapBytes(m_ptr[kTriangleCountOffset]); }

  private:
    // Leaf records store bounds, then a ushort count followed by the triangle indices.
    enum {
      kTriangleCountOffset = sizeof(CAABox) / sizeof(ushort),
      kTriangleDataOffset = kTriangleCountOffset + 1,
    };

    const ushort* m_ptr;
  };

  class Node {
  public:
    enum ETreeType { kTT_Invalid, kTT_Branch, kTT_Leaf };

    Node(const void* ptr, const CAABox& aabb, const CAreaOctTree& owner, ETreeType type)
    : mAabb(aabb)
    , mPtr(reinterpret_cast< const uchar* >(ptr))
    , mOwner(owner)
    , mNodeType(type) {}

    bool LineTest(const CLine& line, const CMaterialFilter& filter, float length) const;
    void LineTestEx(const CLine& line, const CMaterialFilter& filter, SRayResult& res,
                    float length) const;

    const CAreaOctTree& GetOwner() const { return mOwner; }
    const CAABox& GetBoundingBox() const { return mAabb; }
    ushort GetChildFlags() const {
      return CBasics::SwapBytes(*reinterpret_cast< const ushort* >(mPtr));
    }
    Node GetChild(int idx) const;
    TriListReference GetTriangleArray() const;
    ETreeType GetChildType(int idx) const {
      ushort flags = CBasics::SwapBytes(*reinterpret_cast< const ushort* >(mPtr));
      return ETreeType((flags >> (2 * idx)) & 0x3);
    }
    ETreeType GetTreeType() const { return mNodeType; }

  private:
    CAABox mAabb;
    const uchar* mPtr;
    const CAreaOctTree& mOwner;
    ETreeType mNodeType;

    bool LineTestInternal(const CLine& line, const CMaterialFilter& filter, float lT, float hT,
                          float maxT, const CVector3f& vec) const;
    void LineTestExInternal(const CLine& line, const CMaterialFilter& filter, SRayResult& res,
                            float lT, float hT, float maxT, const CVector3f& dirRecip) const;
  };

  CAreaOctTree(const CAABox& bounds, Node::ETreeType treeType, uchar* buf, void* treeBuf,
               uint materialCount, uint* materials, uchar* vertexMaterials, uchar* edgeMaterials,
               uchar* triMaterials, uint edgeCount, CCollisionEdge* edges, uint triCount,
               ushort* triangles, uint vertexCount, CVector3f* vertices);
  static void MakeFromMemory(void* buf, uint bufLen, CAreaOctTree** treeOut, bool* valid);
  CCollisionSurface GetMasterListTriangle(ushort idx) const;
  Node GetRootNode() const { return Node(mTreeBuf, mAabb, *this, mTreeType); }
  const void* GetTreeMemory() const { return mTreeBuf; }
  const CAABox& GetBoundingBox() const { return mAabb; }
  Node::ETreeType GetTreeType() const { return mTreeType; }

  const CVector3f& GetVert(int idx) const { return mVerts[idx]; }
  const CCollisionEdge& GetEdge(int idx) const { return mEdges[idx]; }
  uint GetVertMaterial(int idx) const { return mMaterials[mVertMats[idx]]; }
  uint GetEdgeMaterial(int idx) const { return mMaterials[mEdgeMats[idx]]; }
  uint GetTriangleMaterial(int idx) const { return mMaterials[mPolyMats[idx]]; }
  void GetTriangleVertexIndices(ushort idx, ushort indicesOut[3]) const;
  const ushort* GetTriangleEdgeIndices(ushort idx) const;

private:
  CAABox mAabb;
  Node::ETreeType mTreeType;
  const uchar* mBuf;
  const void* mTreeBuf;
  uint mMatCount;
  const uint* mMaterials;
  const uchar* mVertMats;
  const uchar* mEdgeMats;
  const uchar* mPolyMats;
  uint mEdgeCount;
  const CCollisionEdge* mEdges;
  uint mPolyCount;
  const ushort* mPolyEdges;
  uint mVertCount;
  const CVector3f* mVerts;

#if TARGET_LITTLE_ENDIAN
  rstl::vector< uint > mNativeMaterials;
  rstl::vector< uchar > mNativeVertexMaterials;
  rstl::vector< uchar > mNativeEdgeMaterials;
  rstl::vector< uchar > mNativeTriangleMaterials;
  rstl::vector< CCollisionEdge > mNativeEdges;
  rstl::vector< ushort > mNativeTriangleEdges;
  rstl::vector< CVector3f > mNativeVertices;
#endif
};
#if TARGET_BIG_ENDIAN
CHECK_SIZEOF(CAreaOctTree, 0x50)
#endif
NESTED_CHECK_SIZEOF(CAreaOctTree, Node, 0x24)
NESTED_CHECK_SIZEOF(CAreaOctTree, TriListReference, 0x4)
NESTED_CHECK_SIZEOF(CAreaOctTree, SRayResult, 0x40)

#endif // _CAREAOCTTREE
