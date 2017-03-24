#include "CScriptEffect.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "CActorParameters.hpp"
#include "TCastTo.hpp"

namespace urde
{

u32 CScriptEffect::g_NumParticlesUpdating = 0;
u32 CScriptEffect::g_NumParticlesRendered = 0;

CScriptEffect::CScriptEffect(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                             const zeus::CTransform& xf, const zeus::CVector3f& scale,
                             ResId partId, ResId elscId, bool, bool, bool, bool active,
                             bool, float, float, float, float, bool, float, float, float,
                             bool, bool, bool, const CLightParameters& lParms, bool)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(),
         CActorParameters::None(), kInvalidUniqueId)
{
}

void CScriptEffect::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

}
