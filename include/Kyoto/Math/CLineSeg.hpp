#ifndef _CLINESEG
#define _CLINESEG

#include "types.h"

#include "Kyoto/Math/CLine.hpp"
#include "Kyoto/Math/CUnitVector3f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CLineSeg : public CLine {
public:
  CLineSeg(const CVector3f& start, const CVector3f& end)
  : CLine(start, CUnitVector3f((end - start).AsNormalized())), mEnd(end) {}

  const CVector3f& GetEndPoint() const { return mEnd; }

private:
  CVector3f mEnd;
};
CHECK_SIZEOF(CLineSeg, 0x24)

#endif // _CLINESEG
