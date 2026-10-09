#ifndef _CSCRIPTCONTRAPTION
#define _CSCRIPTCONTRAPTION

#include "types.h"

#include "MetroidPrime/CDamageInfo.hpp"
#include "MetroidPrime/ScriptObjects/CScriptActor.hpp"
#include "MetroidPrime/TGameTypes.hpp"

#include "Kyoto/TToken.hpp"

#include "rstl/list.hpp"
#include "rstl/string.hpp"

class CFlameThrower;
class CWeaponDescription;

class CScriptContraption : public CScriptActor {
public:
  struct SFlameThrower {
    TUniqueId id;
    rstl::string name;
    bool unk;

    SFlameThrower();
    SFlameThrower(TUniqueId uid, const rstl::string& locatorName)
    : id(uid), name(locatorName), unk(false) {}
  };

  CScriptContraption(TUniqueId uid, const rstl::string& name, const CEntityInfo& info,
                     const CTransform4f& xf, const CModelData& mData, const CAABox& aabox,
                     const CMaterialList& matList, const float mass, const float zMomentum,
                     const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                     const CActorParameters& aParams, const CAssetId part, CDamageInfo dInfo,
                     bool active);

  // CEntity
  DECLARE_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  CFlameThrower* CreateFlameThrower(const rstl::string& name, CStateManager& mgr);

private:
  rstl::list< SFlameThrower > mChildren;
  TToken< CWeaponDescription > mFlameThrowerGenDesc;
  CAssetId mFlameFxId;
  CDamageInfo mDInfo;
};
CHECK_CHILD_SIZEOF(CScriptContraption, CScriptActor, 0x40)

#endif // _CSCRIPTCONTRAPTION
