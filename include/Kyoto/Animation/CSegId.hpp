#ifndef _CSEGID
#define _CSEGID

#include <types.h>

#include "Kyoto/Streams/CInputStream.hpp"

class CSegId {
public:
  CSegId(int id) : mId(id) {}
  CSegId(const char& id) : mId(id) {}
  CSegId(uint id) : mId(id) {}
  CSegId(CInputStream& in) : mId(in.Get< uint >()) {}

  const uchar val() const { return mId; }

  static CSegId Null() { return CSegId(0); }
  static CSegId Invalid() { return CSegId(-1); }
  static CSegId Root() { return CSegId(3); }
  static CSegId Character() { return CSegId(2); }

  bool operator==(const CSegId& rhs) const { return mId == rhs.mId; }
  bool operator!=(const CSegId& rhs) const { return mId != rhs.mId; }

private:
  uchar mId;
};

#endif // _CSEGID
