#ifndef _CLOSEENOUGH
#define _CLOSEENOUGH

#include "types.h"

#include "Kyoto/Math/CVector2f.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "math.h"

struct Real32 {
  static inline float Epsilon() { return 1.e-5f; }
};
struct Double {
  static inline double Epsilon() { return 1.0 / 60000.0; }
};
static inline float vector3_epsilon() { return 1.e-4f; }
static inline float vector2_epsilon() { return 1.e-4f; }

bool close_enough(const CVector2f& a, const CVector2f& b, float epsilon = vector2_epsilon());
bool close_enough(const CVector3f& a, const CVector3f& b, float epsilon = vector3_epsilon());
inline bool close_enough(float a, float b, float epsilon = Real32::Epsilon()) {
#if VERSION >= VERSION_R3IJ_00
  return fabsf(a - b) < epsilon;
#else
  return fabs(a - b) < epsilon;
#endif
}
inline bool close_enough(double a, double b, double epsilon = Double::Epsilon()) {
  return fabs(a - b) < epsilon;
}

#endif // _CLOSEENOUGH
