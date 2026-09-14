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
  DECLARE_TYPES_MATCH_OR_ACCEPT;
  void Think(float dt, CStateManager& mgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;

  // CActor
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType type,
                       float dt) override;

  CFlameThrower* CreateFlameThrower(const rstl::string& name, CStateManager& mgr);

private:
  rstl::list< SFlameThrower > x2e8_children;
  TToken< CWeaponDescription > x300_flameThrowerGenDesc;
  CAssetId x308_flameFxId;
  CDamageInfo x30c_dInfo;
};
CHECK_SIZEOF(CScriptContraption, (VERSION >= VERSION_GM8P_00 ? 0x338 : 0x328))

#endif // _CSCRIPTCONTRAPTION
