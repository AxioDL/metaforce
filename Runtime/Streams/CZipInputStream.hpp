#pragma once

#include "CInputStream.hpp"

#include <memory>

#include <zlib.h>

namespace metaforce {
class CZipInputStream final : public CInputStream {
  std::unique_ptr<u8[]> x24_compBuf;
  std::unique_ptr<CInputStream> x28_strm;
  std::unique_ptr<z_stream> x30_zstrm = {};

  u32 Read(void* ptr, u32 len) override;
public:
  explicit CZipInputStream(std::unique_ptr<CInputStream>&& strm);
  ~CZipInputStream() override;

};
} // namespace metaforce
