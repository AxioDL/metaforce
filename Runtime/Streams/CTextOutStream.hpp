#pragma once
#include "Runtime/Streams/COutputStream.hpp"

namespace metaforce {
class CTextOutStream {
  COutputStream* m_out;

public:
  explicit CTextOutStream(COutputStream& out);

  void WriteString(const std::string& str);
  void WriteString(const char* str, u32 len);
};
} // namespace metaforce
