#pragma once

#include <zeus/CQuaternion.hpp>
#include <zeus/CVector3f.hpp>

namespace metaforce {

struct CAnimPerSegmentData {
  zeus::CQuaternion x0_rotation;
  zeus::CVector3f x10_offset;
  bool x1c_hasOffset = false;
};

} // namespace metaforce
