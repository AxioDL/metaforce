#pragma once

#include <string_view>
#include <vector>

#include "Runtime/RetroTypes.hpp"
#include "Runtime/World/CDamageInfo.hpp"
#include "Runtime/World/CScriptActor.hpp"

namespace metaforce {
class CFlameThrower;
class CWeaponDescription;
namespace MP1 {
class CScriptContraption : public CScriptActor {
  /* AKA Why Zoid?!?!?!? */
  std::vector<std::pair<TUniqueId, std::string>> x2e8_children;
  TToken<CWeaponDescription> x300_flameThrowerGen;
  CAssetId x308_flameFxId;
  CDamageInfo x30c_dInfo;

public:
  DEFINE_ENTITY
  CScriptContraption(TUniqueId uid, std::string_view name, const CEntityInfo& info, const zeus::CTransform& xf,
                    CModelData&& mData, const zeus::CAABox& aabox, const CMaterialList& matList, float mass,
                    float zMomentum, const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                    const CActorParameters& aParams, CAssetId part, const CDamageInfo& dInfo, bool active);

  void Accept(IVisitor& visitor) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr) override;
  void Think(float dt, CStateManager& mgr) override;
  void DoUserAnimEvent(CStateManager& mgr, const CInt32POINode& node, EUserEventType evType, float dt) override;
  CFlameThrower* CreateFlameThrower(std::string_view name, CStateManager& mgr);
  void ResetFlameThrowers(CStateManager& mgr);
};
} // namespace MP1
} // namespace metaforce
