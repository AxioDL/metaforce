#ifndef __URDE_CPLAYERVISOR_HPP__
#define __URDE_CPLAYERVISOR_HPP__

namespace urde
{
class CStateManager;
class CTargetingManager;

namespace MP1
{

class CPlayerVisor
{
public:
    CPlayerVisor(CStateManager& stateMgr);
    void Update(float dt, const CStateManager& stateMgr);
    void Draw(const CStateManager& stateMgr, const CTargetingManager* tgtManager) const;
    void Touch();
    float GetDesiredViewportScaleX(const CStateManager& stateMgr) const;
    float GetDesiredViewportScaleY(const CStateManager& stateMgr) const;
};

}
}

#endif // __URDE_CPLAYERVISOR_HPP__
