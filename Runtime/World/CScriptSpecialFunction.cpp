#include "CScriptSpecialFunction.hpp"
#include "Character/CModelData.hpp"
#include "CActorParameters.hpp"
#include "Audio/CSfxManager.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptSpecialFunction::CScriptSpecialFunction(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                               const zeus::CTransform& xf, ESpecialFunction func,
                                               const std::string& lcName, float radius, float f2, float f3, float f4,
                                               const zeus::CVector3f& vec, const zeus::CColor& col, bool active,
                                               const CDamageInfo& dInfo, ResId aId1, ResId aId2, ResId aId3, s16 sId1,
                                               s16 sId2, s16 sId3)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(),
         kInvalidUniqueId)
, xe8_function(func)
, xec_locatorName(lcName)
, xfc_radius(radius)
, x100_(f2)
, x104_(f3)
, x108_(f4)
, x10c_(vec)
, x118_(col)
, x11c_damageInfo(dInfo)
, x170_(CSfxManager::TranslateSFXID(sId1))
, x172_(CSfxManager::TranslateSFXID(sId2))
, x174_(CSfxManager::TranslateSFXID(sId3))
, x1bc_(aId1)
, x1c0_(aId2)
, x1c4_(aId3)
{
    x1e4_26_ = true;
    if (xe8_function == ESpecialFunction::HUDTarget)
        x1c8_ = {{zeus::CVector3f(-1.f), zeus::CVector3f(1.f)}};
}

void CScriptSpecialFunction::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptSpecialFunction::Think(float, CStateManager &)
{

}

void CScriptSpecialFunction::AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager &)
{

}

void CScriptSpecialFunction::PreRender(const zeus::CFrustum &, const CStateManager &)
{

}

void CScriptSpecialFunction::AddToRenderer(const zeus::CFrustum &, const CStateManager &) const
{

}

void CScriptSpecialFunction::Render(const CStateManager &) const
{

}
}
