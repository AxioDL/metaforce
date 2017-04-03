#ifndef __URDE_CHUDRADARINTERFACE_HPP__
#define __URDE_CHUDRADARINTERFACE_HPP__

namespace urde
{
class CGuiFrame;
class CStateManager;

class CHudRadarInterface
{
public:
    CHudRadarInterface(CGuiFrame& selHud, CStateManager& stateMgr);
    void Update(float dt, const CStateManager& mgr);
};

}

#endif // __URDE_CHUDRADARINTERFACE_HPP__
