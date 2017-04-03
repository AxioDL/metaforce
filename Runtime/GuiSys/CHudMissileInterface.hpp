#ifndef __URDE_CHUDMISSILEINTERFACE_HPP__
#define __URDE_CHUDMISSILEINTERFACE_HPP__

namespace urde
{
class CStateManager;

class CHudMissileInterfaceCombat
{
public:
    void Update(float dt, const CStateManager& mgr);
};

}

#endif // __URDE_CHUDMISSILEINTERFACE_HPP__
