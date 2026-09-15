#ifndef _CSCRIPTCAMERASHAKER
#define _CSCRIPTCAMERASHAKER

#include "MetroidPrime/CEntity.hpp"

#include "MetroidPrime/Cameras/CCameraShakeData.hpp"

class CScriptCameraShaker : public CEntity {
  CCameraShakeData x34_shakeData;

public:
  CScriptCameraShaker(TUniqueId uid, const rstl::string& name, const CEntityInfo& info, bool active,
                      const CCameraShakeData& shakeData);
  ~CScriptCameraShaker();

  DECLARE_ACCEPT;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
};

#endif // _CSCRIPTCAMERASHAKER
