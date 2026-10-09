#ifndef _CVECTOR2F
#define _CVECTOR2F

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"
#include "types.h"

class CVector2f {
  static const CVector2f skZeroVector;

public:
  CVector2f() {}
  CVector2f(float x, float y);
  CVector2f(CInputStream& in) : mX(in.Get< float >()), mY(in.Get< float >()) {}

  float GetX() const { return mX; }
  float GetY() const { return mY; }

  CVector2f& operator+=(const CVector2f& rhs);
  CVector2f& operator-=(const CVector2f& rhs);
  CVector2f& operator*=(float rhs);
  CVector2f& operator/=(float rhs);

  CVector2f& Normalize();

  float Magnitude() const;
  bool IsMagnitudeSafe() const;
  float MagSquared() const;
  CVector2f AsNormalized() const;

  float& operator[](int idx) { return *(&mX + idx); }

  const float& operator[](int idx) const { return reinterpret_cast< const float* >(this)[idx]; }

  static float GetAngleDiff(const CVector2f& a, const CVector2f& b);
  static float Dot(const CVector2f& a, const CVector2f& b);

  static const CVector2f& Zero() { return skZeroVector; }

  static CVector2f Lerp(const CVector2f& a, const CVector2f& b, const float t) {
    const float negT = 1.f - t;
#if VERSION >= VERSION_R3IJ_00
    // Wii rounds each product before adding the two contributions.
    return CVector2f(static_cast< float >(a.mX * negT) + static_cast< float >(b.mX * t),
                     static_cast< float >(a.mY * negT) + static_cast< float >(b.mY * t));
#else
    return CVector2f(a.mX * negT + b.mX * t, a.mY * negT + b.mY * t);
#endif
  }

  void PutTo(COutputStream& out) const {
    out.Put(GetX());
    out.Put(GetY());
  }

private:
  float mX;
  float mY;
};

CVector2f operator+(const CVector2f& lhs, const CVector2f& rhs);
CVector2f operator-(const CVector2f& lhs, const CVector2f& rhs);
bool operator==(const CVector2f& lhs, const CVector2f& rhs);
CVector2f operator*(const CVector2f& lhs, const float& rhs);
CVector2f operator*(const float& lhs, const CVector2f& rhs);
CVector2f operator/(const CVector2f& lhs, const float& rhs);

#endif // _CVECTOR2F
