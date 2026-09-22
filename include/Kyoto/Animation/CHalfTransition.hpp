#ifndef _CHALFTRANSITION
#define _CHALFTRANSITION

#include "Kyoto/Animation/IMetaTrans.hpp"

#include "rstl/rc_ptr.hpp"

class CInputStream;
class CHalfTransition {
public:
  explicit CHalfTransition(CInputStream& in);
  uint GetPrimitiveIndex() const { return x0_id; }
  const rstl::rc_ptr< IMetaTrans >& GetMetaTrans() const { return x4_trans; }

private:
  uint x0_id;
  rstl::rc_ptr< IMetaTrans > x4_trans;
};
CHECK_SIZEOF(CHalfTransition, 0x8)

#endif // _CHALFTRANSITION
