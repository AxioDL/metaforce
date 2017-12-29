#ifndef __URDE_CSAMUSFACEREFLECTION_HPP__
#define __URDE_CSAMUSFACEREFLECTION_HPP__

#include "Character/CModelData.hpp"
#include "Character/CActorLights.hpp"

namespace urde::MP1
{

class CSamusFaceReflection
{
    CModelData x0_modelData;
    std::unique_ptr<CActorLights> x4c_lights;
    zeus::CQuaternion x50_lookRot;
    zeus::CVector3f x60_lookDir;
    u32 x6c_ = 0;
    bool x70_hidden = true;
public:
    CSamusFaceReflection(CStateManager& stateMgr);
    void PreDraw(const CStateManager& stateMgr);
    void Draw(const CStateManager& stateMgr) const;
    void Update(float dt, const CStateManager& stateMgr, CRandom16& rand);
};

}

#endif // __URDE_CSAMUSFACEREFLECTION_HPP__
