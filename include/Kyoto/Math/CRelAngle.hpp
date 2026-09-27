#ifndef _CRELANGLE
#define _CRELANGLE

#include "types.h"

#include "Kyoto/Math/CMath.hpp"

class CRelAngle {
public:
  CRelAngle(float rad) : mAngle(rad) {}
  float AsDegrees() const { return mAngle * (180.f / M_PIF); }
  float AsRadians() const { return mAngle; }
  // ArcCosine__9CRelAngleFf weak

  CRelAngle& operator+=(const CRelAngle& v) {
    mAngle += v.mAngle;
    return *this;
  }
  CRelAngle& operator-=(const CRelAngle& v) {
    mAngle -= v.mAngle;
    return *this;
  }
  CRelAngle& operator*=(float v) {
    mAngle *= v;
    return *this;
  }
  CRelAngle& operator/=(float v) {
    mAngle /= v;
    return *this;
  }

  bool operator<(const CRelAngle& rhs) const { return mAngle < rhs.mAngle; }

  static CRelAngle FromDegrees(const float deg) { return CRelAngle(deg * (M_PIF / 180.f)); }
  static CRelAngle FromRadians(const float rad) { return CRelAngle(rad); }

private:
  float mAngle;
};
CHECK_SIZEOF(CRelAngle, 0x4)

inline CRelAngle operator-(const CRelAngle& a, const CRelAngle& b) {
  CRelAngle out(a);
  out -= b;
  return out;
}
inline CRelAngle operator+(const CRelAngle& a, const CRelAngle& b) {
  CRelAngle out(a);
  out += b;
  return out;
}
inline CRelAngle operator/(const CRelAngle& a, float b) {
  CRelAngle out(a);
  out /= b;
  return out;
}
inline CRelAngle operator*(const CRelAngle& a, float b) {
  CRelAngle out(a);
  out *= b;
  return out;
}
inline float sine(const CRelAngle& angle) { return sin(angle.AsRadians()); }
inline float cosine(const CRelAngle& angle) { return cos(angle.AsRadians()); }

#endif // _CRELANGLE
