#include "Kyoto/Math/CMath.hpp"

#include "Kyoto/Math/CVector3f.hpp"

#if VERSION >= VERSION_R3IJ_00
static inline double InvSqrtEstimate(double x) {
  double estimate = __frsqrte(x);
  const double halfX = 0.5 * x;
  for (int i = 0; i < 2; ++i) {
    const double square = estimate * estimate;
    const double product = halfX * square;
    estimate *= 1.5 - product;
  }
  const double square = estimate * estimate;
  const double product = halfX * square;
  return estimate * (1.5 - product);
}

float CMath::SqrtF(const float x) {
  if (x > 0.f) {
    return x * InvSqrtEstimate(x);
  }
  return 0.f;
}

double CMath::SqrtD(const double x) {
  if (x > 0.0) {
    return x * InvSqrtEstimate(x);
  }
  return 0.0;
}

float CMath::InvSqrtF(float x) {
  if (x > 0.f) {
    return InvSqrtEstimate(x);
  }
  return 0.f;
}
#else
float CMath::SqrtF(const float x) { return sqrt(x); }

double CMath::SqrtD(const double x) { return sqrt(x); }

float CMath::InvSqrtF(float x) { return 1.f / sqrt(x); }
#endif

float CMath::ArcSineR(float v) { return asin(v); }

float CMath::ArcCosineR(float v) { return acos(v); }

float CMath::ArcTangentR(float v) { return atan(v); }

float CMath::PowF(float x, float y) { return exp(log(x) * y); }

float CMath::SlowSineR(float x) { return sin(x); }

float CMath::SlowCosineR(float x) { return cos(x); }

float CMath::SlowTangentR(float x) { return tan(x); }

const float CMath::FloorF(float x) { return floor(x); }

float CMath::CeilingF(float x) {
#if VERSION >= VERSION_R3IJ_00
  float tmp = floorf(x);
#else
  float tmp = FloorF(x);
#endif
  if (tmp == x) {
    return x;
  }
  return tmp + 1.f;
}

CVector3f CMath::GetCatmullRomSplinePoint(const CVector3f& a, const CVector3f& b,
                                          const CVector3f& c, const CVector3f& d, float t) {
  if (t <= 0.0f)
    return b;
  if (t >= 1.0f)
    return c;

  return (a * (-0.5f * t * t * t + t * t - 0.5f * t) +
          b * (1.5f * t * t * t + -2.5f * t * t + 1.0f) +
          c * (-1.5f * t * t * t + 2.0f * t * t + 0.5f * t) +
          d * (0.5f * t * t * t - 0.5f * t * t));
}

float CMath::GetCatmullRomSplinePoint(float a, float b, float c, float d, float t) {
  if (t <= 0.0f)
    return b;
  if (t >= 1.0f)
    return c;

  return (a * (-0.5f * t * t * t + t * t - 0.5f * t) +
          b * (1.5f * t * t * t + -2.5f * t * t + 1.0f) +
          c * (-1.5f * t * t * t + 2.0f * t * t + 0.5f * t) +
          d * (0.5f * t * t * t - 0.5f * t * t));
}

CVector3f CMath::GetBezierPoint(const CVector3f& a, const CVector3f& b, const CVector3f& c,
                                const CVector3f& d, float t) {
  CVector3f ab = CVector3f::Lerp(a, b, t);
  CVector3f bc = CVector3f::Lerp(b, c, t);
  CVector3f cd = CVector3f::Lerp(c, d, t);

  return CVector3f::Lerp(CVector3f::Lerp(ab, bc, t), CVector3f::Lerp(bc, cd, t), t);
}

CVector3f CMath::BaryToWorld(const CVector3f& p0, const CVector3f& p1, const CVector3f& p2,
                             const CVector3f& bary) {
  return bary.GetX() * p0 + bary.GetY() * p1 + bary.GetZ() * p2;
}

static const uint skSinX1 = 0x3f7ff347; // 0.99980587f
static const uint skSinX2 = 0xbe2a34ae; // -0.16621658f
static const uint skSinX3 = 0x3c047fca; // 0.008087108f
static const uint skSinX4 = 0xb9206873; // -0.000152977f

float CMath::FastSinR(float x) {
  if (fabs(x) > M_PIF) {
    x = WrapPi(x);
  }

  float x2 = x * x;
  float acc = x;
  acc *= reinterpret_cast< const float& >(skSinX1);
  x *= x2;
  acc += x * reinterpret_cast< const float& >(skSinX2);
  x *= x2;
  acc += x * reinterpret_cast< const float& >(skSinX3);
  x *= x2;
  acc += x * reinterpret_cast< const float& >(skSinX4);
  return acc;
}

static const uint skCosX1 = 0x3f800000; // 1.0f
static const uint skCosX2 = 0xbefffd62; // -0.49998003f
static const uint skCosX3 = 0x3d2a7a18; // 0.041620344f
static const uint skCosX4 = 0xbab2bb2b; // -0.0013636103f
static const uint skCosX5 = 0x37a93188; // 0.000020169435f

float CMath::FastCosR(float x) {
  if (fabs(x) > M_PIF) {
    x = WrapPi(x);
  }

  float x2 = x * x;
  float acc = reinterpret_cast< const float& >(skCosX1);
  acc += x2 * reinterpret_cast< const float& >(skCosX2);
  float xn = x2 * x2;
  acc += xn * reinterpret_cast< const float& >(skCosX3);
  xn *= x2;
  acc += xn * reinterpret_cast< const float& >(skCosX4);
  xn *= x2;
  acc += xn * reinterpret_cast< const float& >(skCosX5);
  return acc;
}

static const uint skArcCosX1 = 0x3fc90fdb; // 1.5707964f (pi/2)
static const uint skArcCosX2 = 0xbf7f8bd1; // -0.9982272f
static const uint skArcCosX3 = 0xbe52ce8c; // -0.20586604f
static const uint skArcCosX4 = 0x3de9fe20; // 0.114254236f
static const uint skArcCosX5 = 0xbe980d88; // -0.29697824f

float CMath::FastArcCosR(float x) {
  if (fabs(x) > 0.925f) {
    return acosf(x);
  }

  float x2 = x * x;
  float acc = reinterpret_cast< const float& >(skArcCosX1);
  acc += x * reinterpret_cast< const float& >(skArcCosX2);
  float xn = x * x2;
  acc += xn * reinterpret_cast< const float& >(skArcCosX3);
  xn *= x2;
  acc += xn * reinterpret_cast< const float& >(skArcCosX4);
  xn *= x2;
  acc += xn * reinterpret_cast< const float& >(skArcCosX5);
  return acc;
}

int CMath::FloorPowerOfTwo(int v) {
  if (v == 0) {
    return 0;
  }
  const uint s1 = (0xffffU - v) >> 0x1b & 0x10;
  const uint sb1 = static_cast< uint >(v) >> s1 & 0xffff;
  const uint s2 = (0xff - sb1) >> 0x1c & 8;
  const uint sb2 = sb1 >> s2 & 0xff;
  const uint s3 = ((0xf - sb2) >> 0x1d) & 4;
  const uint sb3 = (sb2 >> s3) & 0xf;
  const uint s4 = (3 - sb3) >> 0x1e & 2;
  const uint totalShift = s1 + s2 + s3 + s4;
  const uint finalSig = sb3 >> s4 & 3;
  const uint finalShift = ((1 - finalSig) >> 0x1f) + totalShift;
  return 1 << finalShift;
}

#if VERSION >= VERSION_R3IJ_00
float CMath::EaseInOut(float t, EEaseTypes type, float easeInEnd, float easeOutStart,
                      float start, float end, float slope) {
  const float min = FastMin(start, end);
  const float max = FastMax(start, end);
  const float range = max - min;
  t = FastClamp(0.f, t, 1.f);
  easeInEnd = FastClamp(0.f, easeInEnd, 1.f);
  easeOutStart = FastClamp(0.f, easeOutStart, 1.f);

  switch (type) {
  case kET_Sinusoidal: {
    const float linearEndArea = easeOutStart + 2.f * easeInEnd / M_PIF - easeInEnd;
    const float denominator = linearEndArea + 2.f * (1.f - easeOutStart) / M_PIF;
    if (t <= easeInEnd) {
      const float phase = (M_PIF / 2.f) * (t / easeInEnd);
      t = (2.f * easeInEnd / M_PIF * (1.f + sinf(phase - M_PIF / 2.f))) / denominator;
    } else if (t >= easeOutStart) {
      const float tail = (2.f * (1.f - easeOutStart) / M_PIF) *
          sinf((M_PIF / 2.f) * ((t - easeOutStart) / (1.f - easeOutStart)));
      t = (easeOutStart + 2.f * easeInEnd / M_PIF - easeInEnd + tail) / denominator;
    } else {
      t = (t + 2.f * easeInEnd / M_PIF - easeInEnd) / denominator;
    }
    break;
  }
  case kET_Quadratic:
    if (t <= easeInEnd) {
      t = slope * (t * t / (2.f * easeInEnd));
    } else if (t >= easeOutStart) {
      const float easeInArea = 0.5f * (slope * easeInEnd);
      const float linearArea = slope * (easeOutStart - easeInEnd);
      const float slopeReduction = 0.5f * (slope * ((t - easeOutStart) / (1.f - easeOutStart)));
      const float easeOutArea = (slope - slopeReduction) * (t - easeOutStart);
      t = easeInArea + linearArea + easeOutArea;
    } else {
      const float easeInArea = slope * (0.5f * easeInEnd);
      const float linearArea = slope * (t - easeInEnd);
      t = easeInArea + linearArea;
    }
    break;
  }

  t = FastClamp(0.f, t, 1.f);
  const float scaled = range * t;
  return min + scaled;
}
#endif
