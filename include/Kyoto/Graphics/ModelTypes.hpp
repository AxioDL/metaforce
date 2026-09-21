#ifndef _MODELTYPES
#define _MODELTYPES

#include "types.h"

#if defined(TARGET_PC)
#include <span>

typedef std::span< const uchar > TModelData;
typedef std::span< const float > TModelPositions;
typedef std::span< const float > TModelNormals;
#else
typedef const void* TModelData;
typedef const float* TModelPositions;
typedef const float* TModelNormals;
#endif

inline const void* GetModelDataPointer(TModelData data) {
#if defined(TARGET_PC)
  return data.data();
#else
  return data;
#endif
}

#endif // _MODELTYPES
