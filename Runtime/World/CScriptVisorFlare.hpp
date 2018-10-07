#pragma once

#include "CActor.hpp"
#include "CVisorFlare.hpp"

namespace urde
{

class CScriptVisorFlare : public CActor
{
    CVisorFlare xe8_flare;
    bool x11c_notInRenderLast = true;

public:
    CScriptVisorFlare(TUniqueId, std::string_view name, const CEntityInfo& info,
                      bool, const zeus::CVector3f&, CVisorFlare::EBlendMode blendMode,
                      bool, float, float, float, u32, u32,
                      const std::vector<CVisorFlare::CFlareDef>& flares);

    void Accept(IVisitor& visitor);
    void Think(float, CStateManager& stateMgr);
    void AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr);
    void PreRender(CStateManager&, const zeus::CFrustum&);
    void AddToRenderer(const zeus::CFrustum&, const CStateManager&) const;
    void Render(const CStateManager&) const;
};

}

