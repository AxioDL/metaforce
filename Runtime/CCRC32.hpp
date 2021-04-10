#pragma once

#include <cstdint>

namespace metaforce {

class CCRC32 {
public:
  static uint32_t Calculate(const void* data, uint32_t length);
};

} // namespace metaforce
