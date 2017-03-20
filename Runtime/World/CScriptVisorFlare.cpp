#include "CScriptVisorFlare.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"
#include "CStateManager.hpp"
#include "CPlayer.hpp"

namespace urde
{

CScriptVisorFlare::CScriptVisorFlare(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                  bool active, const zeus::CVector3f& pos, CVisorFlare::EBlendMode blendMode,
                  bool b1, float f1, float f2, float f3, u32 w1, u32 w2,
                  const std::vector<CVisorFlare::CFlareDef>& flares)
: CActor(uid, active, name, info, zeus::CTransform::Translate(pos), CModelData::CModelDataNull(),
         CMaterialList(EMaterialTypes::Unknown), CActorParameters::None(), kInvalidUniqueId),
  xe8_flare(blendMode, b1, f1, f2, f3, w1, w2, flares)
{
    xe6_27_ = 2;
}

void CScriptVisorFlare::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptVisorFlare::Think(float dt, CStateManager& stateMgr)
{
    if (GetActive())
        xe8_flare.Update(dt, GetTranslation(), this, stateMgr);
}

void CScriptVisorFlare::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId objId, CStateManager& stateMgr)
{
    CActor::AcceptScriptMsg(msg, objId, stateMgr);
}

void CScriptVisorFlare::PreRender(CStateManager& stateMgr, const zeus::CFrustum&)
{
    x11c_notInRenderLast = !stateMgr.RenderLast(x8_uid);
}

void CScriptVisorFlare::AddToRenderer(const zeus::CFrustum&, const CStateManager& stateMgr) const
{
    if (x11c_notInRenderLast)
        EnsureRendered(stateMgr, stateMgr.GetPlayer().GetTranslation(), GetSortingBounds(stateMgr));
}

void CScriptVisorFlare::Render(const CStateManager& stateMgr) const
{
    xe8_flare.Render(GetTranslation(), stateMgr);
}

}
