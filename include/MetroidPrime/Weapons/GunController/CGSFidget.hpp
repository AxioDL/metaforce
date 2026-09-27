#ifndef _CGSFIDGET
#define _CGSFIDGET

#include "rstl/vector.hpp"

#include "Kyoto/TToken.hpp"

class CAnimData;
class CStateManager;

class CGSFidget {
public:
  CGSFidget();
  
  bool Update(CAnimData& data, float dt, CStateManager& mgr);
  int SetAnim(CAnimData& data, int type, int gunId, int animSet, CStateManager& mgr);
  void LoadAnimAsync(CAnimData& data, int type, int gunId, int animSet, CStateManager& mgr);
  void UnLoadAnim();
  bool IsAnimLoaded() const;

private:
  rstl::vector< CToken > mAnims;
  int x10_;
  int mGunId;
  int mAnimSet;
};

#endif // _CGSFIDGET
