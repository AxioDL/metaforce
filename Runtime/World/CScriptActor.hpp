#ifndef __URDE_CSCRIPTACTOR_HPP__
#define __URDE_CSCRIPTACTOR_HPP__

#include "CPhysicsActor.hpp"
#include "CHealthInfo.hpp"
#include "CDamageVulnerability.hpp"

namespace urde
{

class CScriptActor : public CPhysicsActor
{
protected:
    CHealthInfo x258_initialHealth;
    CHealthInfo x260_currentHealth;
    CDamageVulnerability x268_damageVulnerability;
    float x2d0_alphaMax;
    float x2d4_alphaMin;
    s32 x2d8_;
    float x2dc_xrayAlpha;
    TUniqueId x2e0_triggerId = kInvalidUniqueId;
    bool x2e2_24_ : 1;
    bool x2e2_25_dead : 1;
    bool x2e2_26_animating : 1;
    bool x2e2_27_ : 1;
    bool x2e2_28_ : 1;
    bool x2e2_29_ : 1;
    bool x2e2_30_transposeRotate : 1;
    bool x2e2_31_ : 1;
    bool x2e3_24_ : 1;

public:
    CScriptActor(TUniqueId, std::string_view, const CEntityInfo&, const zeus::CTransform&, CModelData&&,
                 const zeus::CAABox& aabb, float, float, const CMaterialList& matList, const CHealthInfo&,
                 const CDamageVulnerability&, const CActorParameters&, bool, bool, u32, float, bool, bool, bool, bool);
    void Accept(IVisitor& visitor);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void Think(float, CStateManager&);
    void PreRender(CStateManager&, const zeus::CFrustum&);
    zeus::CAABox GetSortingBounds(const CStateManager&) const;
    EWeaponCollisionResponseTypes GetCollisionResponseType(const zeus::CVector3f&, const zeus::CVector3f&,
                                                                   const CWeaponMode&, int) const;
    rstl::optional_object<zeus::CAABox> GetTouchBounds() const;
    void Touch(CActor&, CStateManager&);
    const CDamageVulnerability* GetDamageVulnerability() const { return &x268_damageVulnerability; }
    CHealthInfo* HealthInfo(CStateManager&) { return &x260_currentHealth; }
};
}

#endif // __URDE_CSCRIPTACTOR_HPP__
