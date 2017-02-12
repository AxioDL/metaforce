#ifndef __URDE_CSAMUSFACEREFLECTION_HPP__
#define __URDE_CSAMUSFACEREFLECTION_HPP__

#include "Character/CModelData.hpp"
#include "Character/CActorLights.hpp"

namespace urde
{
namespace MP1
{

class CSamusFaceReflection
{
    CModelData x0_modelData;
    std::unique_ptr<CActorLights> x4c_lights;
    zeus::CQuaternion x50_;
    zeus::CVector3f x60_;
    u32 x6c_ = 0;
    bool x70_ = true;
public:
    CSamusFaceReflection(CStateManager& stateMgr);
};

}
}

#endif // __URDE_CSAMUSFACEREFLECTION_HPP__
