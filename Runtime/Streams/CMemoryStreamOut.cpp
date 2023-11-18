#include "CMemoryStreamOut.hpp"

#include <cstring>

namespace metaforce {
CMemoryStreamOut::~CMemoryStreamOut() {
  Flush();

  if (x88_owned) {
    delete[] x7c_ptr;
  }
}

void CMemoryStreamOut::Write(const u8* ptr, u32 len) {
  const auto offset = (x80_len - x84_position);
  if (offset < len) {
    len = offset;
  }

  if (len != 0) {
    memcpy(x7c_ptr + x84_position, ptr, len);
    x84_position += len;
  }
}
} // namespace metaforce
