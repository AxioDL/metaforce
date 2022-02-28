#pragma once
#include "Runtime/Streams/CInputStream.hpp"

namespace metaforce {
class CTextInStream {
  CInputStream* m_in;
  s32 m_len;

public:
  CTextInStream(CInputStream& in, int len);

  bool IsEOF() { return m_in->GetReadPosition() >= m_len; }
  std::string GetNextLine();
};
} // namespace metaforce