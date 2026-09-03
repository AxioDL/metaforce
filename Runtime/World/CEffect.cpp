#include "Runtime/World/CEffect.hpp"

#include "Runtime/World/CActorParameters.hpp"

namespace metaforce {

CEffect::CEffect(TUniqueId uid, const CEntityInfo& info, bool active, std::string_view name, const zeus::CTransform4f& xf)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::NoStepLogic),
         CActorParameters::None(), kInvalidUniqueId) {}

} // namespace metaforce
