#include "CScriptActor.hpp"
#include "CStateManager.hpp"
#include "CScriptTrigger.hpp"
#include "CDamageVulnerability.hpp"
#include "TCastTo.hpp"

namespace urde
{

CScriptActor::CScriptActor(TUniqueId uid, const std::string& name, const CEntityInfo& info, const zeus::CTransform& xf,
                           CModelData&& mData, const zeus::CAABox& aabb, float f1, float zMomentum,
                           const CMaterialList& matList, const CHealthInfo& hInfo, const CDamageVulnerability& dVuln,
                           const CActorParameters& actParms, bool looping, bool active, u32 w1, float f3, bool b2,
                           bool castsShadow, bool b4, bool b5)
: CPhysicsActor(uid, active, name, info, xf, std::move(mData), matList, aabb, SMoverData(f1), actParms, 0.3f, 0.1f)
, x258_initialHealth(hInfo)
, x260_currentHealth(hInfo)
, x268_damageVulnerability(dVuln)
, x2d8_(w1)
, x2dc_(f3)
, x2e2_24_(b2)
, x2e2_25_(false)
, x2e2_26_(true)
, x2e2_27_(std::fabs(f3 - 1.f) > 0.00001)
, x2e2_28_(false)
, x2e2_29_((x2e2_24_ && x2e2_25_ && x2d8_ != 0))
, x2e2_30_(b4)
, x2e2_31_(b5)
{
    if (x64_modelData && (x64_modelData->HasAnimData() || x64_modelData->HasNormalModel()) && castsShadow)
        CreateShadow(true);

    if (x64_modelData && x64_modelData->HasAnimData())
        x64_modelData->EnableLooping(looping);

    x150_momentum = zeus::CVector3f(0.f, 0.f, zMomentum);
}

void CScriptActor::Accept(IVisitor& visitor) { visitor.Visit(this); }

void CScriptActor::AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&) {}

void CScriptActor::Think(float, CStateManager&) {}

void CScriptActor::PreRender(const zeus::CFrustum&, const CStateManager&) {}

zeus::CAABox CScriptActor::GetSortingBounds(const CStateManager& mgr) const
{
    if (x2e0_ != kInvalidUniqueId)
    {
        TCastToConstPtr<CScriptTrigger> trigger(mgr.GetObjectById(x2e0_));
        if (trigger)
            return trigger->GetTriggerBoundsWR();
    }

    return CActor::GetSortingBounds(mgr);
}

EWeaponCollisionResponseTypes
CScriptActor::GetCollisionResponseType(const zeus::CVector3f& v1, const zeus::CVector3f& v2, CWeaponMode& wMode, s32 w)
{
    const CDamageVulnerability* dVuln = GetDamageVulnerability();
    EVulnerability vuln = dVuln->GetVulnerability(wMode, 0);
    return CActor::GetCollisionResponseType(v1, v2, wMode, w);
}

rstl::optional_object<zeus::CAABox> CScriptActor::GetTouchBounds() const
{
    if (GetActive() && x68_material.HasMaterial(EMaterialTypes::Solid))
        return {CPhysicsActor::GetBoundingBox()};
    return {};
}

void CScriptActor::Touch(CActor&, CStateManager&)
{
}
}
