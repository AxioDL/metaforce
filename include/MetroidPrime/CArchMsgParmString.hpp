#ifndef _CARCHMSGPARMSTRING
#define _CARCHMSGPARMSTRING

#include "MetroidPrime/CArchitectureMessage.hpp"
#include "rstl/string.hpp"

class CArchMsgParmString : public IArchitectureMessageParm {
public:
  const rstl::string& GetString() const { return mString; }

private:
  rstl::string mString;
};
CHECK_SIZEOF(CArchMsgParmString, (RSTL_VERSION >= RSTL_R3IJ ? 0x10 : 0x14))

#endif // _CARCHMSGPARMSTRING
