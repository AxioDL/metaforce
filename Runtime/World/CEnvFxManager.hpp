#ifndef __URDE_CENVFXMANAGER_HPP__
#define __URDE_CENVFXMANAGER_HPP__

namespace urde
{
class CStateManager;

enum class EEnvFxType
{
    None,
    Rain,
    Snow
};

enum class EPhazonType
{
    None,
    Blue,
    Orange
};

class CEnvFxManager
{
public:
    void AsyncLoadResources(CStateManager& mgr);
};

}

#endif // __URDE_CENVFXMANAGER_HPP__
