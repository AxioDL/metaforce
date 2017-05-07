#ifndef __URDE_CCOLLISIONACTORMANAGER_HPP__
#define __URDE_CCOLLISIONACTORMANAGER_HPP__

#include "RetroTypes.hpp"
#include "zeus/CAABox.hpp"
#include "Collision/CJointCollisionDescription.hpp"

namespace urde
{
class CMaterialList;
class CAnimData;
class CCollisionActor;
class CStateManager;
class CCollisionActorManager
{
public:
    enum class EUpdateOptions
    {
    };

private:
    std::vector<CJointCollisionDescription> x0_jointDescriptions;
    TUniqueId x10_;
    bool x13_ = false;
public:
    CCollisionActorManager(CStateManager&, TUniqueId, TAreaId, const std::vector<CJointCollisionDescription>&, bool);

    void Update(float, CStateManager&, EUpdateOptions) const;
    void Destroy(CStateManager&) const;
    void SetActive(CStateManager&, bool);
    void AddMaterial(CStateManager&, const CMaterialList&);

    u32 GetNumCollisionActors() const;
    CJointCollisionDescription GetCollisionDescFromIndex(u32) const;
    zeus::CTransform GetWRLocatorTransform(const CAnimData&, CSegId, const zeus::CTransform&, const zeus::CTransform&);
};
}
#endif // __URDE_CCOLLISIONACTORMANAGER_HPP__
