#include "CInterpolationCamera.hpp"
#include "CCameraManager.hpp"
#include "TCastTo.hpp"

namespace urde
{

CInterpolationCamera::CInterpolationCamera(TUniqueId uid, const zeus::CTransform& xf)
: CGameCamera(uid, false, "Interpolation Camera",
              CEntityInfo(kInvalidAreaId, CEntity::NullConnectionList, kInvalidEditorId),
              xf, CCameraManager::ThirdPersonFOV(), CCameraManager::NearPlane(),
              CCameraManager::FarPlane(), CCameraManager::Aspect(), kInvalidUniqueId, false, 0)
{

}

void CInterpolationCamera::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CInterpolationCamera::AcceptScriptMsg(EScriptObjectMessage msg, TUniqueId sender, CStateManager& mgr)
{
    CGameCamera::AcceptScriptMsg(msg, sender, mgr);
}

void CInterpolationCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr)
{
    // Empty
}

void CInterpolationCamera::Render(const CStateManager& mgr) const
{
    // Empty
}

void CInterpolationCamera::Reset(const zeus::CTransform&, CStateManager& mgr)
{
    // Empty
}

void CInterpolationCamera::Think(float, CStateManager& mgr)
{

}

void CInterpolationCamera::SetInterpolation(const zeus::CTransform& xf, const zeus::CVector3f& lookDir,
                                            float f1, float f2, float f3, TUniqueId camId, bool b1, CStateManager& mgr)
{

}

}
