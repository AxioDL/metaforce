#ifndef _CCOLLISIONSURFACE
#define _CCOLLISIONSURFACE

#include "types.h"

#include "Kyoto/Math/CPlane.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CCollisionSurface {
public:
  CCollisionSurface(const CVector3f&, const CVector3f&, const CVector3f&, uint flags);

  CUnitVector3f GetNormal() const;
  CPlane GetPlane() const;
  uint GetSurfaceFlags() const { return mFlags; }
  const CVector3f& GetVert(int i) const { return (&mA)[i]; }
  const CVector3f& GetPoint(int i) const { return (&mA)[i]; }

private:
  CVector3f mA;
  CVector3f mB;
  CVector3f mC;
  uint mFlags;
};
CHECK_SIZEOF(CCollisionSurface, 0x28)

#endif // _CCOLLISIONSURFACE
