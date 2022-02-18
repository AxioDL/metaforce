#include "Runtime/CStringExtras.hpp"
#include "Runtime/CInputStream.hpp"

namespace metaforce {
std::string CStringExtras::ReadString(CInputStream& in) {
  u32 strLen = in.ReadLong();
  std::string ret;
  u32 readLen = 512;
  char tmp[512] = {};
  for (; strLen > 0; strLen -= readLen) {
    readLen = 512;
    if (strLen <= 512) {
      readLen = strLen;
    }
    in.ReadBytes(tmp, readLen);
    ret.append(tmp, readLen);
  }

  return ret;
}
}