#ifndef _CMATRIX3F
#define _CMATRIX3F

#include "types.h"

#include "Kyoto/Math/CTransform4f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

class CInputStream;
class CMatrix3f {
  static const CMatrix3f sIdentity;

public:
  CMatrix3f(const float _m00, const float _m01, const float _m02, const float _m10,
            const float _m11, const float _m12, const float _m20, const float _m21,
            const float _m22)
  : m00(_m00)
  , m01(_m01)
  , m02(_m02)
  , m10(_m10)
  , m11(_m11)
  , m12(_m12)
  , m20(_m20)
  , m21(_m21)
  , m22(_m22) {}
  CMatrix3f(const CVector3f& _m0, const CVector3f& _m1, const CVector3f& _m2);
  CMatrix3f(const CMatrix3f& left, float leftScale, const CMatrix3f& right, float rightScale);
  CMatrix3f(CInputStream& in);
  CMatrix3f(const CMatrix3f&);
  //  fake but useful for CEulerAngles?
  CMatrix3f(const CTransform4f& xf); /*
   : m0(xf.GetRow(kDX))
   , m1(xf.GetRow(kDY))
   , m2(xf.GetRow(kDZ)) {}*/

  static CMatrix3f RotateY(const CRelAngle& angle);
  static CMatrix3f RotateZ(const CRelAngle& angle);
  const CMatrix3f& operator=(const CMatrix3f& other);
  const CVector3f operator*(const CVector3f&) const;
  const CMatrix3f operator*(const CMatrix3f&) const;

  static const CMatrix3f& Identity() { return sIdentity; }

  CMatrix3f Orthonormalized() const;
  float Determinant() const;
  CMatrix3f Inverse() const;
  CMatrix3f GetTranspose() const { return CMatrix3f(m00, m10, m20, m01, m11, m21, m02, m12, m22); }
  void AddScaledMatrix(const CMatrix3f& mat, float scale);

  // TODO: names/check
  inline const CVector3f& GetRow(EDimX dim) const {
    return *reinterpret_cast< const CVector3f* >(&m00);
  }
  inline const CVector3f& GetRow(EDimY dim) const {
    return *reinterpret_cast< const CVector3f* >(&m10);
  }
  inline const CVector3f& GetRow(EDimZ dim) const {
    return *reinterpret_cast< const CVector3f* >(&m20);
  }

  float Get00() const { return m00; }
  float Get01() const { return m01; }
  float Get02() const { return m02; }
  float Get10() const { return m10; }
  float Get11() const { return m11; }
  float Get12() const { return m12; }
  float Get20() const { return m20; }
  float Get21() const { return m21; }
  float Get22() const { return m22; }

  inline CVector3f GetColumn(EDimX dim) const {
#if VERSION >= VERSION_R3IJ_00
    float x, y, z;
    z = GetRow(kDZ)[dim];
    y = GetRow(kDY)[dim];
    x = GetRow(kDX)[dim];
    return CVector3f(x, y, z);
#else
    return CVector3f(GetRow(kDX)[dim], GetRow(kDY)[dim], GetRow(kDZ)[dim]);
#endif
  }

  inline CVector3f GetColumn(EDimY dim) const {
#if VERSION >= VERSION_R3IJ_00
    float x, y, z;
    z = GetRow(kDZ)[dim];
    y = GetRow(kDY)[dim];
    x = GetRow(kDX)[dim];
    return CVector3f(x, y, z);
#else
    return CVector3f(GetRow(kDX)[dim], GetRow(kDY)[dim], GetRow(kDZ)[dim]);
#endif
  }

  inline CVector3f GetColumn(EDimZ dim) const {
    return CVector3f(GetRow(kDX)[dim], GetRow(kDY)[dim], GetRow(kDZ)[dim]);
  }

  inline CVector3f GetColumn(int idx) const {
    switch (idx) {
    case 0:
      return CVector3f(m00, m10, m20);
    case 1:
      return CVector3f(m01, m11, m21);
    case 2:
      return CVector3f(m02, m12, m22);
    default:
      return CVector3f::Zero();
    }
  }

  static inline CMatrix3f Scale(float s) {
    return CMatrix3f(s, 0.f, 0.f, 0.f, s, 0.f, 0.f, 0.f, s);
  }

  static inline CMatrix3f Scale(float x, float y, float z) {
    return CMatrix3f(x, 0.f, 0.f, 0.f, y, 0.f, 0.f, 0.f, z);
  }

  static CMatrix3f FromTransform(const CTransform4f& xf);

private:
  float m00;
  float m01;
  float m02;
  float m10;
  float m11;
  float m12;
  float m20;
  float m21;
  float m22;
};
CHECK_SIZEOF(CMatrix3f, 0x24);

#endif // _CMATRIX3F
