#ifndef _MATH_H_
#define _MATH_H_

#include "GameVersions.h"

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _MATH_INLINE
#define _MATH_INLINE static inline
#endif

#ifdef __MWERKS__

/* Metrowerks */
#if __option(little_endian)
#define __IEEE_LITTLE_ENDIAN
#else
#define __IEEE_BIG_ENDIAN
#endif

#else

/* GCC */
#ifdef __BIG_ENDIAN__
#define __IEEE_BIG_ENDIAN
#endif
#ifdef __LITTLE_ENDIAN__
#define __IEEE_LITTLE_ENDIAN
#endif

#endif

#ifndef __IEEE_BIG_ENDIAN
#ifndef __IEEE_LITTLE_ENDIAN
#error Must define endianness
#endif
#endif

#ifndef _INT32
typedef int _INT32;
typedef unsigned int _UINT32;
#endif

#ifdef __MWERKS__
_MATH_INLINE double fabs(double x) { return __fabs(x); }
#endif

extern _INT32 __float_huge[];
extern _INT32 __float_nan[];
extern _INT32 __double_huge[];
extern _INT32 __extended_huge[];
extern _INT32 __extended_min[];
extern _INT32 __extended_max[];
extern _INT32 __extended_epsilon[];
extern _INT32 __double_min[];
extern _INT32 __double_max[];

#define HUGE_VAL (*(double*)__double_huge)
#define INFINITY (*(float*)__float_huge)
#define NAN (*(float*)__float_nan)
#define HUGE_VALF (*(float*)__float_huge)
#define HUGE_VALL (*(long double*)__extended_huge)

double fabs(double x);
double fmod(double x, double m);
double modf(double x, double* integerPart);
double sin(double x);
double cos(double x);
double atan(double x);
double atan2(double y, double x);
double tan(double x);
double ceil(double x);

_MATH_INLINE float fabsf(float x) { return (float)fabs((double)x); }
_MATH_INLINE float sinf(float x) { return (float)sin((double)x); }
_MATH_INLINE float cosf(float x) { return (float)cos((double)x); }
_MATH_INLINE float atan2f(float y, float x) { return (float)atan2((double)y, (double)x); }
_MATH_INLINE float fmodf(float x, float m) { return (float)fmod((double)x, (double)m); }
float tanf(float x);
double asin(double x);
double acos(double x);
_MATH_INLINE float acosf(float x) { return (float)acos((double)x); }
double log(double x);
double exp(double x);

double ldexp(double x, int exp);

double copysign(double x, double y);

double floor(double x);
_MATH_INLINE float floorf(float x) { return floor(x); }

double fabs(double x);
double pow(double x, double y);
_MATH_INLINE float powf(float __x, float __y) { return pow(__x, __y); }

#ifdef __MWERKS__
#pragma cplusplus on
#endif

#ifdef __IEEE_LITTLE_ENDIAN
#define __HI(x) (sizeof(x) == 8 ? *(1 + (_INT32*)&x) : (*(_INT32*)&x))
#define __LO(x) (*(_INT32*)&x)
#define __UHI(x) (sizeof(x) == 8 ? *(1 + (_UINT32*)&x) : (*(_UINT32*)&x))
#define __ULO(x) (*(_UINT32*)&x)
#else
#define __LO(x) (sizeof(x) == 8 ? *(1 + (_INT32*)&x) : (*(_INT32*)&x))
#define __HI(x) (*(_INT32*)&x)
#define __ULO(x) (sizeof(x) == 8 ? *(1 + (_UINT32*)&x) : (*(_UINT32*)&x))
#define __UHI(x) (*(_UINT32*)&x)
#endif

#define signbit(x)((int)(__HI(x)&0x80000000))

#if VERSION >= VERSION_R3IJ_00
#define FP_NAN 1
#define FP_INFINITE 2
#define FP_ZERO 3
#define FP_NORMAL 4
#define FP_SUBNORMAL 5

int __fpclassifyf(float x);

#else
/* The pre-2.4.7 runtime headers use different floating-point classification values. */
#if !defined(__MWERKS__) || __MWERKS__ >= 0x2407
#define FP_NAN 0
#define FP_INFINITE 1
#define FP_ZERO 3
#define FP_NORMAL 4
#define FP_SUBNORMAL 2

static inline int __fpclassifyf(float x) {
  switch ((*(_INT32*)&x) & 0x7f800000) {
  case 0:
    if ((*(_INT32*)&x) & 0x007fffff)
      return FP_SUBNORMAL;
    else
      return FP_ZERO;
  default:
    return FP_NORMAL;
  case 0x7f800000:
    if ((*(_INT32*)&x) & 0x007fffff)
      return FP_NAN;
    else
      return FP_INFINITE;
  }
}

#else
#define FP_NAN 1
#define FP_INFINITE 2
#define FP_ZERO 3
#define FP_NORMAL 4
#define FP_SUBNORMAL 5

static inline int __fpclassifyf(float x) {
  switch ((*(_INT32*)&x) & 0x7f800000) {
  case 0x7f800000: {
    if ((*(_INT32*)&x) & 0x007fffff)
      return FP_NAN;
    else
      return FP_INFINITE;
    break;
  }
  case 0: {
    if ((*(_INT32*)&x) & 0x007fffff)
      return FP_SUBNORMAL;
    else
      return FP_ZERO;
    break;
  }
  }
  return FP_NORMAL;
}

#endif

#endif

static inline int __fpclassifyd(double x) {
  switch (__HI(x) & 0x7ff00000) {
  case 0x7ff00000: {
    if ((__HI(x) & 0x000fffff) || (__LO(x) & 0xffffffff))
      return FP_NAN;
    else
      return FP_INFINITE;
    break;
  }
  case 0: {
    if ((__HI(x) & 0x000fffff) || (__LO(x) & 0xffffffff))
      return FP_SUBNORMAL;
    else
      return FP_ZERO;
    break;
  }
  }
  return FP_NORMAL;
}

#define fpclassify(x)                                                                              \
  (sizeof(x) == sizeof(float) ? __fpclassifyf((float)(x)) : __fpclassifyd((double)(x)))
#define isnormal(x) (fpclassify(x) == FP_NORMAL)
#define isnan(x) (fpclassify(x) == FP_NAN)
#define isinf(x) (fpclassify(x) == FP_INFINITE)
#define isfinite(x) ((fpclassify(x) > FP_INFINITE))

#ifdef __MWERKS__
#if VERSION >= VERSION_R3IJ_00
static inline float sqrtf(float x) {
  static const double _half = .5;
  static const double _three = 3.0;
  if (x > 0.0f) {
    double dmag = x;
    double guess = __frsqrte(dmag);
    guess = _half * guess * (_three - guess * guess * dmag);
    guess = _half * guess * (_three - guess * guess * dmag);
    guess = _half * guess * (_three - guess * guess * dmag);
    return static_cast< float >(dmag * guess);
  } else if (x < 0.0) {
    return NAN;
  } else if (isnan(x)) {
    return NAN;
  } else {
    return x;
  }
}
#else
extern inline float sqrtf(float x) {
  static const double _half = .5;
  static const double _three = 3.0;
  volatile float y;

  if (x > 0.0f) {
    double guess = __frsqrte((double)x);                  /* returns an approximation to	*/
    guess = _half * guess * (_three - guess * guess * x); /* now have 12 sig bits
                                                           */
    guess = _half * guess * (_three - guess * guess * x); /* now have 24 sig bits
                                                           */
    guess = _half * guess * (_three - guess * guess * x); /* now have 32 sig bits
                                                           */
    y = (float)(x * guess);
    return y;
  }
  return x;
}
#endif

_MATH_INLINE double sqrt(double x) {
  if (x > 0.0) {
    double guess = __frsqrte(x);                    /* returns an approximation to  */
    guess = .5 * guess * (3.0 - guess * guess * x); /* now have 8 sig bits          */
    guess = .5 * guess * (3.0 - guess * guess * x); /* now have 16 sig bits         */
    guess = .5 * guess * (3.0 - guess * guess * x); /* now have 32 sig bits         */
    guess = .5 * guess * (3.0 - guess * guess * x); /* now have > 53 sig bits       */
    return x * guess;
  } else if (x == 0.0) {
    return 0;
  } else if (x) {
    return NAN;
  }
  return INFINITY;
}
#else
float sqrtf(float x);
double sqrt(double x);
#endif

#ifdef __MWERKS__
#pragma cplusplus reset
#endif

static inline float ldexpf(float x, int exp) { return (float)ldexp((double)x, exp); }
double frexp(double, int *exp);
static inline double scalbn(double x, int n) { return ldexp(x, n); }
static inline float scalbnf(float x, int n) { return (float)ldexpf(x, n); }
double nextafter(double, double);


#ifdef __cplusplus
}
#endif

#endif
