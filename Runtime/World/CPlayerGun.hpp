#ifndef __URDE_CPLAYERGUN_HPP__
#define __URDE_CPLAYERGUN_HPP__

#include "RetroTypes.hpp"
#include "Character/CActorLights.hpp"
#include "CFidget.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CAABox.hpp"
#include "CPlayerCameraBob.hpp"
#include "CGunMotion.hpp"
#include "CGrappleArm.hpp"
#include "CAuxWeapon.hpp"
#include "CPowerBeam.hpp"
#include "CIceBeam.hpp"
#include "CWaveBeam.hpp"
#include "CPlasmaBeam.hpp"
#include "CPhazonBeam.hpp"
#include "Particle/CElementGen.hpp"
#include "Character/CModelData.hpp"
#include "CWorldShadow.hpp"

namespace urde
{

class CPlayerGun
{
    struct CGunMorph
    {
        CGunMorph(float, float);
    };

    CActorLights x0_lights;
    u32 x2e0_ = 0;
    u32 x2e4_ = 0;
    u32 x2e8_ = 0;
    u32 x2ec_ = 0;
    u32 x2f0_ = 0;
    u32 x2f4_ = 0;
    u32 x2f8_ = 1;
    u32 x2fc_ = 0;
    u32 x300_ = 0;
    u32 x304_ = 0;
    u32 x308_ = 3;
    u32 x30c_ = 0;
    u32 x310_ = 0;
    u32 x314_ = 0;
    u32 x318_ = 0;
    u32 x31c_ = 0;
    u32 x320_ = 0;
    u32 x324_ = 4;
    u32 x328_ = 0x2000;
    u32 x32c_ = 0;
    u32 x330_ = 0;
    u32 x334_ = 0;
    u32 x338_ = 0;
    u32 x33c_ = 0;
    float x340_ = 0.f;
    float x344_ = 0.f;
    float x348_ = 0.f;
    float x34c_ = 0.f;
    float x350_ = 0.f;
    float x354_;
    float x358_;
    float x35c_ = 0.f;
    float x360_ = 0.f;
    float x364_ = 0.f;
    float x368_ = 0.f;
    float x36c_ = 1.f;
    float x370_ = 1.f;
    float x374_ = 0.f;
    float x378_ = 0.f;
    float x37c_ = 0.f;
    float x380_ = 0.f;
    float x384_ = 0.f;
    float x388_ = 0.f;
    float x38c_ = 0.f;
    float x390_ = 0.f;
    float x394_ = 0.f;
    float x398_ = 0.f;
    float x39c_ = 0.f;
    float x3a0_ = 0.f;
    CFidget x3a4_fidget;
    zeus::CVector3f x3dc_;
    zeus::CTransform x3e8_;
    zeus::CTransform x418_;
    zeus::CTransform x448_;
    zeus::CTransform x478_;
    zeus::CTransform x4a8_;
    zeus::CTransform x4d8_;
    zeus::CTransform x508_;
    TUniqueId x538_thisId;
    TUniqueId x53a_ = kInvalidUniqueId;
    TUniqueId x53c_ = kInvalidUniqueId;
    u32 x544_ = 0;
    u32 x548_ = 0;
    u32 x54c_ = 0;
    CPlayerCameraBob x550_camBob;
    u32 x658_ = 1;
    float x65c_ = 0.f;
    float x660_ = 0.f;
    float x664_ = 0.f;
    float x668_;
    float x66c_;
    TUniqueId x670_ = kInvalidUniqueId;
    u32 x674_ = 0;
    CGunMorph x678_morph;
    bool x6a0_24_ = true;
    float x6a4_ = 0.f;
    float x6a8_ = 0.f;
    float x6ac_ = 0.f;
    float x6b0_ = 0.f;
    float x6b4_ = 0.f;
    float x6b8_ = 0.f;
    float x6bc_ = 0.f;
    u32 x6c0_ = 0;
    u32 x6c4_ = 0;
    zeus::CAABox x6c8_;
    CModelData x6e0_;
    u32 x72c_ = 0;
    u32 x730_ = 0;
    u32 x734_ = 0;
    u32 x738_ = 0;
    std::unique_ptr<CGunMotion> x73c_;
    std::unique_ptr<CGrappleArm> x740_;
    std::unique_ptr<CAuxWeapon> x744_;
    std::unique_ptr<CPowerBeam> x74c_;
    std::unique_ptr<CIceBeam> x750_;
    std::unique_ptr<CWaveBeam> x754_;
    std::unique_ptr<CPlasmaBeam> x758_;
    std::unique_ptr<CPhazonBeam> x75c_;
    u32 x760_[4] = {};
    std::unique_ptr<CElementGen> x774_;
    std::unique_ptr<CWorldShadow> x82c_;

public:
    CPlayerGun(TUniqueId id);
};

}

#endif // __URDE_CPLAYERGUN_HPP__
