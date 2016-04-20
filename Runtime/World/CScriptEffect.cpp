#include "CScriptEffect.hpp"
#include "Character/CModelData.hpp"
#include "Collision/CMaterialList.hpp"
#include "CActorParameters.hpp"

namespace urde
{

CScriptEffect::CScriptEffect(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                             const zeus::CTransform& xf, const zeus::CVector3f& scale,
                             ResId partId, ResId elscId, bool, bool, bool, bool active,
                             bool, float, float, float, float, bool, float, float, float,
                             bool, bool, bool, const CLightParameters& lParms, bool)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(0),
         CActorParameters::None(), kInvalidUniqueId)
{
}

}
