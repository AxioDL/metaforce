#include "CScriptSpecialFunction.hpp"
#include "Character/CModelData.hpp"
#include "CActorParameters.hpp"

namespace urde
{

CScriptSpecialFunction::CScriptSpecialFunction(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                               const zeus::CTransform& xf, ESpecialFunction,
                                               const std::string&, float, float, float, float, const zeus::CVector3f&,
                                               const zeus::CColor&, bool active, const CDamageInfo&, u32, u32, u32, u16, u16, u16)
    : CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(), CActorParameters::None(), kInvalidUniqueId)
{
}


}
