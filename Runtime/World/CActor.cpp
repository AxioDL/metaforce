#include "CActor.hpp"

namespace urde
{

CActor::CActor(TUniqueId uid, bool active, const std::string& name, const CEntityInfo& info,
               const zeus::CTransform&, const CModelData&, const CMaterialList&,
               const CActorParameters&, TUniqueId)
: CEntity(uid, info, active, name)
{
}

}
