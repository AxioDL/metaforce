#ifndef __URDE_CTARGETINGMANAGER_HPP__
#define __URDE_CTARGETINGMANAGER_HPP__

#include "CCompoundTargetReticle.hpp"
#include "COrbitPointMarker.hpp"

namespace urde
{
class CStateManager;
class CTargetingManager
{
    zeus::CQuaternion x0_rot;
public:
    CTargetingManager(const CStateManager& stateMgr);
    bool CheckLoadComplete();
    void Update(float, const CStateManager& stateMgr);
    void Draw(const CStateManager& stateMgr, bool hideLockon) const;
    void Touch() const;
    void SetRotation(const zeus::CQuaternion& rot) { x0_rot = rot; }
};
}

#endif // __URDE_CTARGETINGMANAGER_HPP__
