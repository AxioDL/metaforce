#include "CGrappleArm.hpp"

namespace urde
{

CGrappleArm::CGrappleArm(const zeus::CVector3f& vec)
{
}

void CGrappleArm::AsyncLoadSuit(CStateManager& mgr)
{
}

void CGrappleArm::SetAnimState(EArmState state)
{

}

void CGrappleArm::Activate(bool)
{

}

void CGrappleArm::GrappleBeamDisconnected()
{

}

void CGrappleArm::GrappleBeamConnected()
{

}

void CGrappleArm::RenderGrappleBeam(const CStateManager& mgr, const zeus::CVector3f& pos)
{

}

void CGrappleArm::TouchModel(const CStateManager& mgr) const
{

}

void CGrappleArm::Update(float grappleSwingT, float dt, CStateManager& mgr)
{

}

void CGrappleArm::PreRender(const CStateManager& mgr, const zeus::CFrustum& frustum, const zeus::CVector3f& camPos)
{

}

void CGrappleArm::Render(const CStateManager& mgr, const zeus::CVector3f& pos,
                         const CModelFlags& flags, const CActorLights* lights) const
{

}

void CGrappleArm::AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&)
{

}

void CGrappleArm::EnterStruck(CStateManager& mgr, float angle, bool attack, bool b2)
{

}

void CGrappleArm::EnterIdle(CStateManager& mgr)
{

}

void CGrappleArm::EnterFidget(CStateManager& mgr, SamusGun::EFidgetType type, s32 gunId, s32 parm2)
{

}

void CGrappleArm::EnterFreeLook(s32 gunId, s32 setId, CStateManager& mgr)
{

}

void CGrappleArm::ReturnToDefault(CStateManager& mgr, float f1, bool b1)
{

}

}
