#include "CScriptSpindleCamera.hpp"
#include "Camera/CCameraManager.hpp"
#include "ScriptLoader.hpp"
#include "TCastTo.hpp"

namespace urde
{

SSpindleSegment::SSpindleSegment(CInputStream& in)
: x0_(in.readUint32Big()), x4_paramFlags(ScriptLoader::LoadParameterFlags(in)),
  x8_(in.readFloatBig()), xc_(in.readFloatBig()), x10_(in.readFloatBig()), x14_(in.readFloatBig())
{}

CScriptSpindleCamera::CScriptSpindleCamera(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                                           const zeus::CTransform& xf, bool active, u32 r9,
                                           float f1, float f2, float f3, float f4, const SSpindleSegment& seg1,
                                           const SSpindleSegment& seg2, const SSpindleSegment& seg3,
                                           const SSpindleSegment& seg4, const SSpindleSegment& seg5,
                                           const SSpindleSegment& seg6, const SSpindleSegment& seg7,
                                           const SSpindleSegment& seg8, const SSpindleSegment& seg9,
                                           const SSpindleSegment& seg10, const SSpindleSegment& seg11,
                                           const SSpindleSegment& seg12, const SSpindleSegment& seg13,
                                           const SSpindleSegment& seg14, const SSpindleSegment& seg15)
: CGameCamera(uid, active, name, info, xf, CCameraManager::ThirdPersonFOV(), CCameraManager::NearPlane(),
              CCameraManager::FarPlane(), CCameraManager::Aspect(), kInvalidUniqueId, false, 0),
  x188_r9(r9), x1b0_f1(f1), x1b4_f2(f2), x1b8_f3(f2), x1bc_f4(f4), x1c0_seg1(seg1), x1d8_seg2(seg2),
  x1f0_seg3(seg3), x208_seg4(seg4), x220_seg5(seg5), x238_seg6(seg6), x250_seg7(seg7),
  x268_seg8(seg8), x280_seg9(seg9), x298_seg10(seg10), x2b0_seg11(seg11), x2c8_seg12(seg12),
  x2e0_seg13(seg13), x2f8_seg14(seg14), x310_seg15(seg15), x330_lookDir(xf.basis[1])
{

}

void CScriptSpindleCamera::Accept(IVisitor& visitor)
{
    visitor.Visit(this);
}

void CScriptSpindleCamera::ProcessInput(const CFinalInput& input, CStateManager& mgr)
{

}

void CScriptSpindleCamera::Reset(const zeus::CTransform& xf, CStateManager& mgr)
{

}

}
