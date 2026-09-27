#ifndef _CAREARENDEROCTTREE
#define _CAREARENDEROCTTREE

#include "types.h"
#include "Kyoto/Basics/CBasics.hpp"

#include "Kyoto/Math/CAABox.hpp"
#include "rstl/auto_ptr.hpp"
#include "rstl/vector.hpp"

struct CAreaRenderOctTree {
  struct Node {
    ushort mBitmapIdx;
    uchar mReserved;
    uchar mFlags;
    ushort mChildren[1];

    int GetChildCount() const;
    CAABox GetNodeBounds(const CAABox& curAABB, int idx) const;
    void RecursiveBuildOverlaps(uint* out, const CAreaRenderOctTree& parent, const CAABox& curAABB,
                                const CAABox& testAABB) const;
  };

  rstl::auto_ptr< const uchar > mBuf;
  uint mBitmapCount;
  uint mMeshCount;
  uint mNodeCount;
  uint mBitmapWordCount;
  CAABox mAabb;
  const uint* mBitmaps;
  const uint* mIndirectionTable;
  const uchar* mEntries;

  explicit CAreaRenderOctTree(const rstl::auto_ptr< const uchar >& buf);

  const Node* GetNode(int idx) const {
    return reinterpret_cast< const Node* >(mEntries + CBasics::SwapBytes(mIndirectionTable[idx]));
  }

  void FindOverlappingModels(rstl::vector< uint >& out, const CAABox& testAABB) const;
  void FindOverlappingModels(uint* out, const CAABox& testAABB) const;
  static bool TestBit(const uint* words, int bitIdx);
};
CHECK_SIZEOF(CAreaRenderOctTree, 0x3c);
NESTED_CHECK_SIZEOF(CAreaRenderOctTree, Node, 0x6);

#endif
