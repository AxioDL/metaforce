#pragma once

#include "Runtime/Streams/COutputStream.hpp"
#include <cstdio>

namespace metaforce {
class CFileOutStream final : public COutputStream {
  FILE* m_file;
public:
  explicit CFileOutStream(std::string_view name, u32 blockLen = 4096);
  virtual ~CFileOutStream();

protected:
  void Write(const u8* ptr, u32 len);
};
} // namespace metaforce
