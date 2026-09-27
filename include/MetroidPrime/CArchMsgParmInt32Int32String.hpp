#ifndef _CARCHMSGPARMINT32INT32STRING
#define _CARCHMSGPARMINT32INT32STRING

#include "MetroidPrime/CArchitectureMessage.hpp"
#include "rstl/string.hpp"

class CArchMsgParmInt32Int32String : public IArchitectureMessageParm {
public:
  int GetFirstInt32() const { return mInt1; }
  int GetSecondInt32() const { return mInt2; }
  const rstl::string& GetString() const { return mString; }

private:
  int mInt1;
  int mInt2;
  rstl::string mString;
};
CHECK_SIZEOF(CArchMsgParmInt32Int32String, 0x1c)

#endif // _CARCHMSGPARMINT32INT32STRING
