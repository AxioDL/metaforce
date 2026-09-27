#ifndef _CTRI
#define _CTRI

#include "types.h"

#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CTri {
public:
  CTri(const CVector3f& a, const CVector3f& b, const CVector3f& c)
  : mPlane(a, b, c), mA(a), mB(b), mC(c) {}

  const CPlane& GetPlane() const { return mPlane; }
  const CVector3f& GetPointA() const { return mA; }
  const CVector3f& GetPointB() const { return mB; }
  const CVector3f& GetPointC() const { return mC; }

  // TODO: Verify, this should be right though
  CPlane GetState() const { return CPlane(mA, mB, mC); }

private:
  CPlane mPlane;
  CVector3f mA;
  CVector3f mB;
  CVector3f mC;
};
CHECK_SIZEOF(CTri, 0x34)

#endif // _CTRI
