#ifndef _CANIMATION
#define _CANIMATION

#include "Kyoto/Animation/IMetaAnim.hpp"

#include "rstl/rc_ptr.hpp"
#include "rstl/string.hpp"

class CInputStream;

class CAnimation {
public:
  CAnimation(CInputStream& in);
  const rstl::rc_ptr< IMetaAnim >& GetMetaAnim() const { return x10_anim; }

private:
  rstl::string x0_name;
  rstl::rc_ptr< IMetaAnim > x10_anim;
};

#endif // _CANIMATION
