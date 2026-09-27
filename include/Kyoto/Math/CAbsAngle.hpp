#ifndef _CABSANGLE
#define _CABSANGLE

#include "types.h"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CRelAngle.hpp"

class CRelAngle;

class CAbsAngle {
public:
  float AsDegrees() const { return mAngle * (180.f / M_PIF); }
  float AsRadians() const { return mAngle; }
  // ArcCosine__9CAbsAngleFf weak
  // -> calls ArcCosineR__5CMathFf

  CAbsAngle& operator+=(const CAbsAngle& v) {
    mAngle += v.mAngle;
    return *this;
  }

  CAbsAngle& operator-=(const CAbsAngle& v) {
    mAngle -= v.mAngle;
    return *this;
  }
  CAbsAngle& operator*=(float v) {
    mAngle *= v;
    return *this;
  }
  CAbsAngle& operator/=(float v) {
    mAngle /= v;
    return *this;
  }

  CAbsAngle& operator+=(const CRelAngle& v) {
    mAngle = CMath::ClampRadians(mAngle + v.AsRadians());
    return *this;
  }

  CAbsAngle& operator-=(const CRelAngle& v) {
    mAngle = CMath::ClampRadians(mAngle - v.AsRadians());
    return *this;
  }

  static CAbsAngle FromDegrees(float deg) {
    return CAbsAngle(CMath::ClampRadians(deg * (M_PIF / 180.f)));
  }
  static CAbsAngle FromRadians(float rad) { return CAbsAngle(CMath::ClampRadians(rad)); }

private:
  CAbsAngle(float rad) : mAngle(rad) {}

  float mAngle;
};
CHECK_SIZEOF(CAbsAngle, 0x4)

inline CRelAngle operator-(const CAbsAngle& lhs, const CAbsAngle& rhs) {
  return CRelAngle::FromRadians(lhs.AsRadians() - rhs.AsRadians());
}

static inline float cosine(const CAbsAngle& angle) { return cos(angle.AsRadians()); }

#endif // _CABSANGLE
