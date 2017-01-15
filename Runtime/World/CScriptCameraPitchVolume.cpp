#include "CScriptCameraPitchVolume.hpp"
#include "CActorParameters.hpp"
#include "CStateManager.hpp"
#include "CPlayer.hpp"
#include "Camera/CCameraManager.hpp"
#include "Camera/CFirstPersonCamera.hpp"
#include "TCastTo.hpp"

namespace urde
{
const zeus::CVector3f CScriptCameraPitchVolume::skScaleFactor = {0.5f};

CScriptCameraPitchVolume::CScriptCameraPitchVolume(TUniqueId uid, bool active, const std::string& name,
                                                   const CEntityInfo& info, const zeus::CVector3f& scale,
                                                   const zeus::CTransform& xf, const zeus::CRelAngle& r1,
                                                   const zeus::CRelAngle& r2, float maxInterpDistance)
: CActor(uid, active, name, info, xf, CModelData::CModelDataNull(), CMaterialList(EMaterialTypes::Trigger),
         CActorParameters::None(), kInvalidUniqueId)
, xe8_obbox(xf, scale * skScaleFactor)
, x124_(r1)
, x128_(r2)
, x12c_scale(scale * skScaleFactor)
, x138_maxInterpDistance(maxInterpDistance)
{
}

void CScriptCameraPitchVolume::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptCameraPitchVolume::Think(float, CStateManager&)
{
    if (!x30_24_active)
        return;
}

rstl::optional_object<zeus::CAABox> CScriptCameraPitchVolume::GetTouchBounds() const
{
    return {xe8_obbox.calculateAABox(zeus::CTransform::Identity())};
}

void CScriptCameraPitchVolume::Touch(CActor& act, CStateManager& mgr)
{
    CPlayer* pl = TCastToPtr<CPlayer>(act);
    if (!pl)
        return;

    auto plBox = pl->GetTouchBounds();
    if (!plBox)
        return;

    zeus::COBBox plOBox = zeus::COBBox::FromAABox(plBox.value(), zeus::CTransform::Identity());
    xe8_obbox;
}

const zeus::CVector3f& CScriptCameraPitchVolume::GetScale() const { return x12c_scale; }

float CScriptCameraPitchVolume::GetMaxInterpolationDistance() const { return x138_maxInterpDistance; }

void CScriptCameraPitchVolume::Entered(urde::CStateManager& mgr)
{
    x13c_25_occupied = true;
    mgr.GetCameraManager()->GetFirstPersonCamera()->SetScriptPitchId(GetUniqueId());
}

void CScriptCameraPitchVolume::Exited(CStateManager& mgr)
{
    x13c_25_occupied = false;
    mgr.GetCameraManager()->GetFirstPersonCamera()->SetScriptPitchId(kInvalidUniqueId);
}
}
