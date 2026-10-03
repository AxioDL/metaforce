#ifndef _CCOLOR
#define _CCOLOR

#include "types.h"

#include "Kyoto/Basics/CCast.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include <dolphin/gx/GXStruct.h>

#ifdef __MWERKS__
#pragma cpp_extensions on
#endif

class CInputStream;
class CColor {
public:
  CColor() { Set(255, 0, 255); }
  CColor(const uint col) { Set(col); }
  CColor(CInputStream& in);
  CColor(const float r, const float g, const float b, const float a = 1.f);
  CColor(const uchar r, const uchar g, const uchar b, const uchar a = 255) {
    mR = r;
    mG = g;
    mB = b;
    mA = a;
  }
#ifdef __MWERKS__
  CColor(const CColor& other) : mRgba(other.mRgba) {}
#else
  CColor(const CColor& other) : mR(other.mR), mG(other.mG), mB(other.mB), mA(other.mA) {}
#endif

  CColor& operator=(const CColor& other) {
#ifdef __MWERKS__
    mRgba = other.mRgba;
#else
    Set(other.mR, other.mG, other.mB, other.mA);
#endif
    return *this;
  }

  void Set(const float r, const float g, const float b, const float a);
  void Set(uchar r, uchar g, uchar b, uchar a = 255) {
    mR = r;
    mG = g;
    mB = b;
    mA = a;
  }
#ifdef __MWERKS__
  void Set(uint col) { mRgba = col; }
#else
  void Set(uint col) {
    mR = static_cast< uchar >(col >> 24);
    mG = static_cast< uchar >(col >> 16);
    mB = static_cast< uchar >(col >> 8);
    mA = static_cast< uchar >(col);
  }
#endif
  void Get(float& r, float& g, float& b, float& a) const;
  void Get(float& r, float& g, float& b) const;
  // TODO check. Maybe this calls SetAlpha(uchar)?
  void SetAlpha(float a) { mA = CCast::ToUint8(a * 255.f); }
  void SetAlpha(uchar a) { mA = a; }
  void SetRed(uchar r) { mR = r; }
  void SetGreen(uchar g) { mG = g; }
  void SetBlue(uchar b) { mB = b; }

  static CColor Lerp(const CColor& a, const CColor& b, const float t);
  static uint Lerp(uint a, uint b, float t);
  static CColor Modulate(const CColor& a, const CColor& b);
  static CColor Add(const CColor& a, const CColor& b);
  float GetRed() const { return CCast::ToReal32(mR) * (1.f / 255.f); }
  float GetGreen() const { return CCast::ToReal32(mG) * (1.f / 255.f); }
  float GetBlue() const { return CCast::ToReal32(mB) * (1.f / 255.f); }
  float GetAlpha() const { return CCast::ToReal32(mA) * (1.f / 255.f); }
  uchar GetRedu8() const { return mR; }
  uchar GetGreenu8() const { return mG; }
  uchar GetBlueu8() const { return mB; }
  uchar GetAlphau8() const { return mA; }
  ushort ToRGB5A3() const;
#ifdef __MWERKS__
  uint GetColor_u32() const { return mRgba; }
#else
  uint GetColor_u32() const {
    return (uint(mR) << 24) | (uint(mG) << 16) | (uint(mB) << 8) | mA;
  }
#endif
#ifdef __MWERKS__
  const GXColor& GetGXColor() const { return *reinterpret_cast< const GXColor* >(this); }
#else
  GXColor GetGXColor() const {
    const GXColor color = {mR, mG, mB, mA};
    return color;
  }
#endif

#ifdef __MWERKS__
  CColor WithAlphaOf(float a) const { return CColor((mRgba & ~0xff) | CCast::ToUint8(a * 255.f)); }
#else
  CColor WithAlphaOf(float a) const { return CColor(mR, mG, mB, CCast::ToUint8(a * 255.f)); }
#endif
  CColor WithAlphaModulatedBy(float a) const {
#ifdef __MWERKS__
    return CColor((mRgba & ~0xff) | CCast::ToUint8(a * static_cast< float >(mA)));
#else
    return CColor(mR, mG, mB, CCast::ToUint8(a * static_cast< float >(mA)));
#endif
  }

#ifdef __MWERKS__
  bool operator==(const CColor& other) const { return mRgba == other.mRgba; }
#else
  bool operator==(const CColor& other) const { return GetColor_u32() == other.GetColor_u32(); }
#endif

  // TODO check
#ifdef __MWERKS__
  static GXColor ToGX(uint c) { return *reinterpret_cast< const GXColor* >(&c); }
#else
  static GXColor ToGX(uint c) {
    const GXColor color = {static_cast< uchar >(c >> 24), static_cast< uchar >(c >> 16),
                           static_cast< uchar >(c >> 8), static_cast< uchar >(c)};
    return color;
  }
#endif

  // Fake?
  CVector3f ToVector3f() const { return CVector3f(GetRed(), GetGreen(), GetBlue()); }

  static const CColor& Black();
#if VERSION >= VERSION_R3IJ_00
  static const CColor& White() { return sWhiteColor; }
#else
  static const CColor& White();
#endif
  static const CColor& Grey();
  static const CColor& Red();
  static const CColor& Green();
  static const CColor& Blue();
  static const CColor& Yellow();
  static const CColor& Purple();
  static const CColor& Orange();

private:
#ifdef __MWERKS__
  union {
    struct {
      uchar mR;
      uchar mG;
      uchar mB;
      uchar mA;
    };
    uint mRgba;
  };
#else
  ALIGNAS(uint) uchar mR;
  uchar mG;
  uchar mB;
  uchar mA;
#endif

  static const CColor sBlackColor;
  static const CColor sWhiteColor;
  static const CColor sGreyColor;
  static const CColor sRedColor;
  static const CColor sGreenColor;
  static const CColor sBlueColor;
  static const CColor sYellowColor;
  static const CColor sPurpleColor;
  static const CColor sOrangeColor;
};
CHECK_SIZEOF(CColor, 0x4)

#ifdef __MWERKS__
#pragma cpp_extensions off
#endif

#endif // _CCOLOR
