#include "Runtime/Streams/CFileOutStream.hpp"

namespace metaforce {
CFileOutStream::CFileOutStream(std::string_view name, u32 blockLen) : COutputStream(blockLen) {
  m_file = fopen(name.data(), "wbe");
}

CFileOutStream::~CFileOutStream() {
  Flush();
  if (m_file) {
    fclose(m_file);
  }
}

void CFileOutStream::Write(const u8* ptr, u32 len) {
  if (!m_file) {
    return;
  }
  fwrite(ptr, 1, len, m_file);
}
} // namespace metaforce
