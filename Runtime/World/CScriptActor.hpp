#ifndef __URDE_CSCRIPTACTOR_HPP__
#define __URDE_CSCRIPTACTOR_HPP__

#include "CPhysicsActor.hpp"

namespace urde
{

class CScriptActor : public CPhysicsActor
{
public:
    CScriptActor(TUniqueId, const std::string&, const CEntityInfo&,
                 const zeus::CTransform&, const CModelData&, const zeus::CAABox& aabb, float, float,
                 const CMaterialList& matList, const CHealthInfo&, const CDamageVulnerability&,
                 const CActorParameters&, bool, bool, u32, float, bool, bool, bool, bool);
};

}

#endif // __URDE_CSCRIPTACTOR_HPP__
