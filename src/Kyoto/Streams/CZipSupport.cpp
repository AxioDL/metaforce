#include "Kyoto/Streams/CZipSupport.hpp"
#include "Kyoto/Alloc/CMemory.hpp"

#include <zlib.h>

static char* hack() {
  return ZLIB_VERSION;
}

void* CZipSupport::Alloc(void* ptr1, uint w1, uint w2) {
  return rs_new uchar[w1 * w2];
}

void CZipSupport::Free(void* ptr1, void* ptr2) {
  if (ptr2 == nullptr) {
    return;
  }
  delete[] ptr2;
}
