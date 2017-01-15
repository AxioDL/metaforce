#ifndef __URDE_CSCRIPTPICKUP_HPP__
#define __URDE_CSCRIPTPICKUP_HPP__

#include "CPhysicsActor.hpp"

namespace urde
{
class CScriptPickup : public CPhysicsActor
{
public:
    CScriptPickup(TUniqueId, const std::string&, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                  const CActorParameters&, const zeus::CAABox&, s32, s32, s32, s32, float, float, float, float, bool);

    void Accept(IVisitor& visitor);
};
}

#endif // __URDE_CSCRIPTPICKUP_HPP__
