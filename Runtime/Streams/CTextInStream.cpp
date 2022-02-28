#include "Runtime/Streams/CTextInStream.hpp"
#include <algorithm>

namespace metaforce {
CTextInStream::CTextInStream(CInputStream& in, int len) : m_in(&in), m_len(len) {}

std::string CTextInStream::GetNextLine() {
  std::string ret;
  while (true) {
    auto chr = m_in->ReadChar();
    ret += chr;
    if (ret.back() == '\r' || ret.back() == '\n') {
      if (ret.back() == '\r') {
        m_in->ReadChar();
      }
      break;
    }
  }

  ret.erase(std::remove(ret.begin(), ret.end(), '\r'), ret.end());
  ret.erase(std::remove(ret.begin(), ret.end(), '\n'), ret.end());
  return ret;
}
} // namespace metaforce
