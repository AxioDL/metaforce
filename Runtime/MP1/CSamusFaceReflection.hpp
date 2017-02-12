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
    CActorLights x4c_lights;
public:
    CSamusFaceReflection(CStateManager& stateMgr);
};

}
}

#endif // __URDE_CSAMUSFACEREFLECTION_HPP__
