#include "CGameCamera.hpp"
#include "World/CActorParameters.hpp"

namespace urde
{

CGameCamera::CGameCamera(TUniqueId uid, bool active, const std::string& name, const CEntityInfo& info,
                         const zeus::CTransform& xf, float, float, float, float, TUniqueId, bool, u32)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(),
         CMaterialList(EMaterialTypes::Zero), CActorParameters::None(), kInvalidUniqueId)
{
}

}
