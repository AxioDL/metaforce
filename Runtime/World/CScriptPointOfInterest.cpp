#include "CScriptPointOfInterest.hpp"
#include "CActorParameters.hpp"
#include "CStateManager.hpp"
#include "CPlayerState.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptPointOfInterest::CScriptPointOfInterest(TUniqueId uid, std::string_view name, const CEntityInfo info,
                                               const zeus::CTransform& xf, bool active,
                                               const CScannableParameters& parms, float f1)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(u64(EMaterialTypes::Orbit)),
         CActorParameters::None().Scannable(parms), kInvalidUniqueId)
, xe8_pointSize(f1)
{
}

void CScriptPointOfInterest::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptPointOfInterest::Think(float dt, CStateManager& mgr)
{
    xe7_31_targetable = mgr.GetPlayerState()->GetCurrentVisor() == CPlayerState::EPlayerVisor::Scan;
    CEntity::Think(dt, mgr);
}

void CScriptPointOfInterest::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId uid, CStateManager& mgr)
{
    CActor::AcceptScriptMsg(msg, uid, mgr);
}

void CScriptPointOfInterest::AddToRenderer(const zeus::CFrustum&, const CStateManager&) const
{
}

void CScriptPointOfInterest::Render(const CStateManager&) const
{
}

void CScriptPointOfInterest::CalculateRenderBounds()
{
    if (xe8_pointSize == 0.f)
        CActor::CalculateRenderBounds();
    else
        x9c_renderBounds = zeus::CAABox(x34_transform.origin - xe8_pointSize, x34_transform.origin + xe8_pointSize);
}

std::experimental::optional<zeus::CAABox> CScriptPointOfInterest::GetTouchBounds() const
{
    return {zeus::CAABox{x34_transform.origin, x34_transform.origin}};
}
}
