#ifndef _CSCRIPTRIPPLE
#define _CSCRIPTRIPPLE

#include "MetroidPrime/CEntity.hpp"

#include "Kyoto/Math/CVector3f.hpp"

class CScriptRipple : public CEntity {
  float x34_magnitude;
  CVector3f x38_center;

public:
  CScriptRipple(TUniqueId, const rstl::string&, const CEntityInfo&, const CVector3f&, bool, float);
  ~CScriptRipple();

  void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) override;
  void Think(float, CStateManager&) override;
  DECLARE_ACCEPT;
};

#endif // _CSCRIPTRIPPLE
