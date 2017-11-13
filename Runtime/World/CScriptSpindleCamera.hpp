#ifndef __URDE_CSCRIPTSPINDLECAMERA_HPP__
#define __URDE_CSCRIPTSPINDLECAMERA_HPP__

#include "Camera/CGameCamera.hpp"

namespace urde
{

struct SSpindleProperty
{
    u32 x0_;
    u32 x4_paramFlags;
    float x8_;
    float xc_;
    float x10_;
    float x14_;

    SSpindleProperty(CInputStream& in);
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
    SSpindleProperty x1c0_seg1;
    SSpindleProperty x1d8_seg2;
    SSpindleProperty x1f0_seg3;
    SSpindleProperty x208_seg4;
    SSpindleProperty x220_seg5;
    SSpindleProperty x238_seg6;
    SSpindleProperty x250_seg7;
    SSpindleProperty x268_seg8;
    SSpindleProperty x280_seg9;
    SSpindleProperty x298_seg10;
    SSpindleProperty x2b0_seg11;
    SSpindleProperty x2c8_seg12;
    SSpindleProperty x2e0_seg13;
    SSpindleProperty x2f8_seg14;
    SSpindleProperty x310_seg15;
    float x328_ = 0.f;
    bool x32c_24 = false;
    zeus::CVector3f x330_lookDir;
public:
    CScriptSpindleCamera(TUniqueId uid, std::string_view name, const CEntityInfo& info,
                         const zeus::CTransform& xf, bool active, u32 r9,
                         float f1, float f2, float f3, float f4, const SSpindleProperty& seg1,
                         const SSpindleProperty& seg2, const SSpindleProperty& seg3,
                         const SSpindleProperty& seg4, const SSpindleProperty& seg5,
                         const SSpindleProperty& seg6, const SSpindleProperty& seg7,
                         const SSpindleProperty& seg8, const SSpindleProperty& seg9,
                         const SSpindleProperty& seg10, const SSpindleProperty& seg11,
                         const SSpindleProperty& seg12, const SSpindleProperty& seg13,
                         const SSpindleProperty& seg14, const SSpindleProperty& seg15);

    void Accept(IVisitor& visitor);
    void ProcessInput(const CFinalInput& input, CStateManager& mgr);
    void Reset(const zeus::CTransform& xf, CStateManager& mgr);
};

}

#endif // __URDE_CSCRIPTSPINDLECAMERA_HPP__
