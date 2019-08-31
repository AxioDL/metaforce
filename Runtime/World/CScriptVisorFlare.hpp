#pragma once

#include "CActor.hpp"
#include "CVisorFlare.hpp"

namespace urde {

class CScriptVisorFlare : public CActor {
  CVisorFlare xe8_flare;
  bool x11c_notInRenderLast = true;

public:
  CScriptVisorFlare(TUniqueId, std::string_view name, const CEntityInfo& info, bool, const zeus::CVector3f&,
                    CVisorFlare::EBlendMode blendMode, bool, float, float, float, u32, u32,
                    const std::vector<CVisorFlare::CFlareDef>& flares);

  void Accept(IVisitor& visitor) override;
  void Think(float, CStateManager& stateMgr) override;
  void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr) override;
  void PreRender(CStateManager&, const zeus::CFrustum&) override;
  void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const override;
  void Render(const CStateManager&) const override;
};

} // namespace urde
