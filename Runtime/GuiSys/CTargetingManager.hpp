#ifndef __URDE_CTARGETINGMANAGER_HPP__
#define __URDE_CTARGETINGMANAGER_HPP__

#include "CCompoundTargetReticle.hpp"
#include "COrbitPointMarker.hpp"

namespace urde
{
class CStateManager;
class CTargetingManager
{
    CCompoundTargetReticle x0_targetReticule;
    COrbitPointMarker x21c_orbitPointMarker;
public:
    CTargetingManager(const CStateManager& stateMgr);
    bool CheckLoadComplete();
    void Update(float, const CStateManager& stateMgr);
    void Draw(const CStateManager& stateMgr, bool hideLockon) const;
    void Touch();
    CCompoundTargetReticle& CompoundTargetReticle() { return x0_targetReticule; }
};
}

#endif // __URDE_CTARGETINGMANAGER_HPP__
