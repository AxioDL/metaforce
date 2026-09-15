#include "Kyoto/PVS/CPVSVisSet.hpp"
#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/PVS/CPVSVisOctree.hpp"
#include "Kyoto/Streams/CMemoryInStream.hpp"
#include <string.h>

CPVSVisSet::CPVSVisSet(int numBits, int numLights, const rstl::auto_ptr< const char >& leafPtr)
: x0_state(kVSS_NodeFound), x4_numBits(numBits), x8_numLights(numLights), xc_ptr(leafPtr) {}

CPVSVisSet::CPVSVisSet(EPVSVisSetState state) : x0_state(state), x4_numBits(0), x8_numLights(0) {}

EPVSVisSetState CPVSVisSet::GetVisible(int idx) const {
  if (x0_state != kVSS_NodeFound)
    return x0_state;

  int numFeatures = x4_numBits - x8_numLights;
  if (idx < numFeatures) {
    /* This is a feature lookup */
    u8 flag = xc_ptr.get()[idx / 8];
    return flag & (1 << (idx & 7)) ? kVSS_OutOfBounds : kVSS_EndOfTree;
  }

  /* This is a light lookup */
  int lightTest = idx - numFeatures + idx;
  const char* ptr = &xc_ptr.get()[lightTest / 8];
  lightTest &= 0x7;
  if (lightTest < 0x7) {
    return static_cast< EPVSVisSetState >(((uchar)ptr[0] & (0x3 << lightTest)) >> lightTest);
  }
  return static_cast< EPVSVisSetState >((((uchar)ptr[0] >> 7) & 1) | ((ptr[1] & 0x1) << 1));
}

CPVSVisOctree::CPVSVisOctree(const CAABox& bounds, const int numObjects, const int numLights,
                             const char* octreeData)
: mBounds(bounds)
, mNumObjects(numObjects)
, mNumLights(numLights)
, mOctreeData(const_cast< char* >(octreeData))
, mMin(mBounds.GetMinPoint())
, mMax(mBounds.GetMaxPoint()) {
  mOctreeData.release();
}

CPVSVisOctree CPVSVisOctree::MakePVSVisOctree(const char* data, int len) {
  CMemoryInStream in(data, len);
  CAABox bounds(in);
  int numObjects = in.Get< int >();
  int numLights = in.Get< int >();
  in.Get< int >();

  return CPVSVisOctree(bounds, numObjects, numLights, data + in.GetReadPosition());
}

CPVSVisSet CPVSVisOctree::GetVisSet(const CVector3f& point) const {
  if (!GetBounds().PointInside(point)) {
    return CPVSVisSet(kVSS_OutOfBounds);
  }

  uchar nodeData;
  const char* data = mOctreeData.get();
  mMin = mBounds.GetMinPoint();
  mMax = mBounds.GetMaxPoint();

  int child;
  while ((child = IterateSearch((nodeData = CCast::ToUint8(*data++)), point)) != -1) {
    if (child != 0) {
      if ((nodeData & 0x60) == 0) {
        const int index = child - 1;
#ifdef __MWERKS__
        data += CBasics::SwapBytes(reinterpret_cast< const ushort* >(data)[index]);
#else
        ushort offset;
        memcpy(&offset, data + index * sizeof(offset), sizeof(offset));
        data += CBasics::SwapBytes(offset);
#endif
      } else if (nodeData & 0x20) {
        --child;
        data += CCast::ToUint8(data[child]);
      } else {
        const uchar* offset = reinterpret_cast< const uchar* >(data) + (child - 1) * 3;
        data += (offset[0] << 16) + (offset[1] << 8) + offset[2];
      }
    }

    if ((nodeData & 0x60) == 0) {
      data += (GetNumChildren(nodeData) - 1) * 2;
    } else if (nodeData & 0x20) {
      data += GetNumChildren(nodeData) - 1;
    } else {
      data += (GetNumChildren(nodeData) - 1) * 3;
    }
  }

  switch (nodeData & 0x18) {
  case 24: {
    rstl::auto_ptr< const char > leaf(data);
    leaf.release();
    return CPVSVisSet(GetNumObjects(), GetNumLights(), rstl::auto_ptr< const char >(leaf));
  }
  case 8:
    return CPVSVisSet(kVSS_OutOfBounds);
  case 16:
    return CPVSVisSet(kVSS_EndOfTree);
  default:
    return CPVSVisSet(kVSS_OutOfBounds);
  }
}
