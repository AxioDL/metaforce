#ifndef __URDE_CSCRIPTSPINDLECAMERA_HPP__
#define __URDE_CSCRIPTSPINDLECAMERA_HPP__

#include "Camera/CGameCamera.hpp"

namespace urde
{

struct SSpindleSegment
{
    u32 x0_;
    u32 x4_paramFlags;
    float x8_;
    float xc_;
    float x10_;
    float x14_;

    SSpindleSegment(CInputStream& in);
    void FixupAngles()
    {
        x8_ = zeus::degToRad(x8_);
        xc_ = zeus::degToRad(xc_);
    }
};

class CScriptSpindleCamera : public CGameCamera
{
    u32 x188_r9;
    u32 x18c_ = 0;
    float x1b0_f1;
    float x1b4_f2;
    float x1b8_f3;
    float x1bc_f4;
    SSpindleSegment x1c0_seg1;
    SSpindleSegment x1d8_seg2;
    SSpindleSegment x1f0_seg3;
    SSpindleSegment x208_seg4;
    SSpindleSegment x220_seg5;
    SSpindleSegment x238_seg6;
    SSpindleSegment x250_seg7;
    SSpindleSegment x268_seg8;
    SSpindleSegment x280_seg9;
    SSpindleSegment x298_seg10;
    SSpindleSegment x2b0_seg11;
    SSpindleSegment x2c8_seg12;
    SSpindleSegment x2e0_seg13;
    SSpindleSegment x2f8_seg14;
    SSpindleSegment x310_seg15;
    float x328_ = 0.f;
    bool x32c_24 = false;
    zeus::CVector3f x330_lookDir;
public:
    CScriptSpindleCamera(TUniqueId uid, const std::string& name, const CEntityInfo& info,
                         const zeus::CTransform& xf, bool active, u32 r9,
                         float f1, float f2, float f3, float f4, const SSpindleSegment& seg1,
                         const SSpindleSegment& seg2, const SSpindleSegment& seg3,
                         const SSpindleSegment& seg4, const SSpindleSegment& seg5,
                         const SSpindleSegment& seg6, const SSpindleSegment& seg7,
                         const SSpindleSegment& seg8, const SSpindleSegment& seg9,
                         const SSpindleSegment& seg10, const SSpindleSegment& seg11,
                         const SSpindleSegment& seg12, const SSpindleSegment& seg13,
                         const SSpindleSegment& seg14, const SSpindleSegment& seg15);

    void Accept(IVisitor& visitor);
    void ProcessInput(const CFinalInput& input, CStateManager& mgr);
    void Reset(const zeus::CTransform& xf, CStateManager& mgr);
};

}

#endif // __URDE_CSCRIPTSPINDLECAMERA_HPP__
