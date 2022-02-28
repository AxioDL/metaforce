#include "Runtime/Streams/CTextOutStream.hpp"

namespace metaforce {
CTextOutStream::CTextOutStream(COutputStream& out) : m_out(&out) {}

void CTextOutStream::WriteString(const std::string& str) { CTextOutStream::WriteString(str.c_str(), str.length()); }
void CTextOutStream::WriteString(const char* str, u32 len) {
  bool wroteCarriageReturn = false;
  bool wroteLineFeed = false;
  for (u32 i = 0; i < len; ++i) {
    if (str[i] == '\r') {
      wroteCarriageReturn = true;
    } else if (str[i] == '\n' && !wroteCarriageReturn) {
      m_out->WriteChar('\r');
      wroteLineFeed = true;
    }
    m_out->WriteChar(str[i]);
  }

  /* If we didn't write either a line feed or carriage return we need to do that now */
  if (!wroteCarriageReturn && !wroteLineFeed) {
    m_out->WriteChar('\r');
    m_out->WriteChar('\n');
  }

  /* Since this is a text buffer, we always want to flush after writing a string */
  m_out->Flush();
}
} // namespace metaforce
