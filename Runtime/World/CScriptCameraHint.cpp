#include "CScriptCameraHint.hpp"
#include "Character/CModelData.hpp"
#include "CActorParameters.hpp"

namespace urde
{

CScriptCameraHint::CScriptCameraHint(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                     const zeus::CTransform& xf, bool active, u32, u32, u32, float, float, float,
                                     const zeus::CVector3f&, const zeus::CVector3f&, const zeus::CVector3f&,
                                     float, float, float, float, float, float, float, float, float, float)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Zero),
         CActorParameters::None(), kInvalidUniqueId)
{
}

}
