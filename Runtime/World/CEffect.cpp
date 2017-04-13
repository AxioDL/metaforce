#include "CEffect.hpp"
#include "CActorParameters.hpp"

namespace urde
{

CEffect::CEffect(TUniqueId uid, const CEntityInfo& info, bool active, const std::string& name, const zeus::CTransform& xf)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Unknown),
         CActorParameters::None(), kInvalidUniqueId)
{
}

}
