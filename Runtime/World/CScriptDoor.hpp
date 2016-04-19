#ifndef __URDE_CSCRIPTDOOR_HPP__
#define __URDE_CSCRIPTDOOR_HPP__

#include "CPhysicsActor.hpp"

namespace urde
{

class CScriptDoor : public CPhysicsActor
{
public:
    CScriptDoor(TUniqueId, const std::string& name, const CEntityInfo& info,
                const zeus::CTransform&, const CModelData&, const CActorParameters&,
                const zeus::CVector3f&, const zeus::CAABox&,
                bool active, bool material, bool, float, bool ballDoor);
};

}

#endif // __URDE_CSCRIPTDOOR_HPP__
