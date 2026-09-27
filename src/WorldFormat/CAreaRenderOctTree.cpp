#include "WorldFormat/CAreaRenderOctTree.hpp"
#include "Kyoto/Basics/CBasics.hpp"

static const int skChildCounts[] = {0, 2, 2, 4, 2, 4, 4, 8};
// Retail indexes this table with flags * 3, including its unusual axis pairs.
static const int skAxes[] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, 0, 1, 1, -1, -1, 0, 2, 1, 2, 0, 1};

CAreaRenderOctTree::CAreaRenderOctTree(const rstl::auto_ptr< const uchar >& buf)
: mBuf(buf)
, mBitmapCount(CBasics::SwapBytes(*reinterpret_cast< const uint* >(buf.get() + 8)))
, mMeshCount(CBasics::SwapBytes(*reinterpret_cast< const uint* >(buf.get() + 12)))
, mNodeCount(CBasics::SwapBytes(*reinterpret_cast< const uint* >(buf.get() + 16)))
, mBitmapWordCount((mMeshCount + 31) / 32)
#if TARGET_LITTLE_ENDIAN
, mAabb(CBasics::SwapBytes(reinterpret_cast< const float* >(buf.get() + 20)[0]),
           CBasics::SwapBytes(reinterpret_cast< const float* >(buf.get() + 20)[1]),
           CBasics::SwapBytes(reinterpret_cast< const float* >(buf.get() + 20)[2]),
           CBasics::SwapBytes(reinterpret_cast< const float* >(buf.get() + 20)[3]),
           CBasics::SwapBytes(reinterpret_cast< const float* >(buf.get() + 20)[4]),
           CBasics::SwapBytes(reinterpret_cast< const float* >(buf.get() + 20)[5]))
#else
, mAabb(*reinterpret_cast< const CAABox* >(buf.get() + 20))
#endif
, mBitmaps(reinterpret_cast< const uint* >(buf.get() + 64))
, mIndirectionTable(mBitmaps + mBitmapCount * mBitmapWordCount)
, mEntries(reinterpret_cast< const uchar* >(mIndirectionTable + mNodeCount)) {}

int CAreaRenderOctTree::Node::GetChildCount() const { return skChildCounts[mFlags]; }

CAABox CAreaRenderOctTree::Node::GetNodeBounds(const CAABox& curAABB, int idx) const {
  CVector3f min = curAABB.GetMinPoint();
  CVector3f max = curAABB.GetMaxPoint();
  const uint flags = mFlags;
  switch (flags) {
  case 0:
  default:
    break;
  case 1: {
    float center = 0.5f * (max.GetX() + min.GetX());
    if (idx == 0) {
      max.SetX(center);
    } else {
      min.SetX(center);
    }
    break;
  }
  case 2: {
    float center = 0.5f * (max.GetY() + min.GetY());
    if (idx == 0) {
      max.SetY(center);
    } else {
      min.SetY(center);
    }
    break;
  }
  case 4: {
    float center = 0.5f * (max.GetZ() + min.GetZ());
    if (idx == 0) {
      max.SetZ(center);
    } else {
      min.SetZ(center);
    }
    break;
  }
  case 3:
  case 5:
  case 6: {
    CVector3f center = curAABB.GetCenterPoint();
    int axis = flags * 3;
    int a = skAxes[axis];
    int b = skAxes[axis + 1];
    switch (idx) {
    case 0:
      max[a] = center[a];
      max[b] = center[b];
      break;
    case 1:
      min[a] = center[a];
      max[b] = center[b];
      break;
    case 2:
      min[b] = center[b];
      max[a] = center[a];
      break;
    case 3:
      min[a] = center[a];
      min[b] = center[b];
      break;
    }
    break;
  }
  case 7: {
    CVector3f center = curAABB.GetCenterPoint();
    for (int i = 0; i < 3; ++i) {
      if (idx & (1 << i)) {
        min[i] = center[i];
      } else {
        max[i] = center[i];
      }
    }
    break;
  }
  }

  return CAABox(min, max);
}

void CAreaRenderOctTree::FindOverlappingModels(rstl::vector< uint >& out,
                                               const CAABox& testAABB) const {
  out.resize(mBitmapWordCount, 0);
  reinterpret_cast< const Node* >(mEntries + CBasics::SwapBytes(mIndirectionTable[0]))
      ->RecursiveBuildOverlaps(out.data(), *this, mAabb, testAABB);
}

void CAreaRenderOctTree::FindOverlappingModels(uint* out, const CAABox& testAABB) const {
  reinterpret_cast< const Node* >(mEntries + CBasics::SwapBytes(mIndirectionTable[0]))
      ->RecursiveBuildOverlaps(out, *this, mAabb, testAABB);
}

void CAreaRenderOctTree::Node::RecursiveBuildOverlaps(uint* out, const CAreaRenderOctTree& parent,
                                                      const CAABox& curAABB,
                                                      const CAABox& testAABB) const {
  if (testAABB.DoBoundsOverlap(curAABB)) {
    if (mFlags == 0 || curAABB.Inside(testAABB)) {
      ushort bitmapIdx = mBitmapIdx;
      bitmapIdx = CBasics::SwapBytes(bitmapIdx);
      const uint* bitmap = &parent.mBitmaps[bitmapIdx * parent.mBitmapWordCount];
      for (uint i = 0; i < parent.mBitmapWordCount; ++i) {
        out[i] |= CBasics::SwapBytes(bitmap[i]);
      }
    } else {
      int childCount = GetChildCount();
      for (int i = 0; i < childCount; ++i) {
        const Node* child = parent.GetNode(CBasics::SwapBytes(mChildren[i]));
        child->RecursiveBuildOverlaps(out, parent, GetNodeBounds(curAABB, i), testAABB);
      }
    }
  }
}

bool CAreaRenderOctTree::TestBit(const uint* words, int bitIdx) {
  return (words[bitIdx >> 5] & (1 << (bitIdx & 31))) != 0;
}
