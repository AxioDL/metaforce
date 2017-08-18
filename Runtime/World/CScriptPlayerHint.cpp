#include "CScriptPlayerHint.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptPlayerHint::CScriptPlayerHint(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                     const zeus::CTransform& xf, bool active, u32 priority, u32 overrideFlags)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), {EMaterialTypes::Unknown},
         CActorParameters::None(), kInvalidUniqueId), x100_priority(priority), x104_overrideFlags(overrideFlags)
{}

void CScriptPlayerHint::Accept(IVisitor& visit)
{
    visit.Visit(this);
}

}