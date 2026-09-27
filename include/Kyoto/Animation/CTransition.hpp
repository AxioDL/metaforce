#ifndef _CTRANSITION
#define _CTRANSITION

#include "Kyoto/Animation/IMetaTrans.hpp"

#include "rstl/rc_ptr.hpp"

class CInputStream;
class CTransition {
  uint mId;
  uint mAnimA;
  uint mAnimB;
  rstl::rc_ptr< IMetaTrans > mTrans;

public:
  explicit CTransition(CInputStream& in);
  uint GetFromAnimIndex() const { return mAnimA; }
  uint GetToAnimIndex() const { return mAnimB; }
  const rstl::rc_ptr< IMetaTrans >& GetMetaTrans() const { return mTrans; }
};
CHECK_SIZEOF(CTransition, 0x10)

#endif // _CTRANSITION
