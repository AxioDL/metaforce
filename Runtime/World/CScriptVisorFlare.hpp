#pragma once

#include <string_view>

#include "Runtime/World/CActor.hpp"
#include "Runtime/World/CVisorFlare.hpp"

namespace metaforce {

class CScriptVisorFlare : public CActor {
  CVisorFlare xe8_flare;
  bool x11c_notInRenderLast = true;

public:
  CScriptVisorFlare(TUniqueId uid, std::string_view name, const CEntityInfo& info, bool active,
                    const zeus::CVector3f& pos, CVisorFlare::EBlendMode blendMode, bool, float, float, float, u32, u32,
                    std::vector<CVisorFlare::CFlareDef> flares);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager& stateMgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void AddToRenderer(const zeus::CFrustum&, CStateManager&) override;
  void Render(CStateManager&) override;
};

} // namespace metaforce
