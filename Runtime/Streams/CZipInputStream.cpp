#include "CZipInputStream.hpp"

namespace metaforce {
CZipInputStream::CZipInputStream(std::unique_ptr<CInputStream>&& strm)
: CInputStream(4096), x24_compBuf(new u8[4096]), x28_strm(std::move(strm)) {
  x30_zstrm = std::make_unique<z_stream>();
  x30_zstrm->next_in = x24_compBuf.get();
  x30_zstrm->avail_in = 0;
  x30_zstrm->zalloc = [](void*, u32 c, u32 n) -> void* { return new u8[size_t{c} * size_t{n}]; };
  x30_zstrm->zfree = [](void*, void* buf) { delete[] static_cast<u8*>(buf); };
  inflateInit(x30_zstrm.get());
}

CZipInputStream::~CZipInputStream() { inflateEnd(x30_zstrm.get()); }

u32 CZipInputStream::Read(void* buf, u32 len) {
  x30_zstrm->next_out = static_cast<Bytef *>(buf);
  x30_zstrm->avail_out = len;
  if (x30_zstrm->avail_in == 0) {
    x30_zstrm->avail_in = x28_strm->ReadBytes(x24_compBuf.get(),4096);
    x30_zstrm->next_in = x24_compBuf.get();
  }
  inflate(x30_zstrm.get(), Z_NO_FLUSH);
  return len - x30_zstrm->avail_out;
}

} // namespace metaforce