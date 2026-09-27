#ifndef _CCOLLISIONEDGE
#define _CCOLLISIONEDGE

#include "types.h"

#include "Kyoto/Streams/CInputStream.hpp"

class CCollisionEdge {
public:
  CCollisionEdge(const ushort index1, const ushort index2) : mIndex1(index1), mIndex2(index2) {}
  CCollisionEdge(CInputStream& in) {
    mIndex1 = in.Get< ushort >();
    mIndex2 = in.Get< ushort >();
  }

  const ushort GetVertIndex1() const { return mIndex1; }
  const ushort GetVertIndex2() const { return mIndex2; }

private:
  ushort mIndex1;
  ushort mIndex2;
};

CHECK_SIZEOF(CCollisionEdge, 0x4)

#endif // _CCOLLISIONEDGE
