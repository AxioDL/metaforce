#ifndef _CTRANSITION
#define _CTRANSITION

#include "Kyoto/Animation/IMetaTrans.hpp"

#include "rstl/rc_ptr.hpp"

class CInputStream;
class CTransition {
  uint x0_id;
  uint x4_animA;
  uint x8_animB;
  rstl::rc_ptr< IMetaTrans > xc_trans;

public:
  explicit CTransition(CInputStream& in);
  uint GetFromAnimIndex() const { return x4_animA; }
  uint GetToAnimIndex() const { return x8_animB; }
  const rstl::rc_ptr< IMetaTrans >& GetMetaTrans() const { return xc_trans; }
};
CHECK_SIZEOF(CTransition, 0x10)

#endif // _CTRANSITION
