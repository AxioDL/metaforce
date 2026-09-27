#ifndef _CANIMATION
#define _CANIMATION

#include "Kyoto/Animation/IMetaAnim.hpp"

#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"

class CInputStream;

class CAnimation {
public:
  CAnimation(CInputStream& in);
  const rstl::rc_ptr< IMetaAnim >& GetMetaAnim() const { return mAnim; }

private:
  rstl::string mName;
  rstl::rc_ptr< IMetaAnim > mAnim;
};

#endif // _CANIMATION
