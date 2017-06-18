#ifndef __URDE_CPLAYERGUN_HPP__
#define __URDE_CPLAYERGUN_HPP__

#include "RetroTypes.hpp"
#include "Character/CActorLights.hpp"
#include "CFidget.hpp"
#include "zeus/CTransform.hpp"
#include "zeus/CAABox.hpp"
#include "World/CPlayerCameraBob.hpp"
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
#include "World/CWorldShadow.hpp"
#include "World/ScriptObjectSupport.hpp"

namespace urde
{

class CPlayerGun
{
public:
    enum class EMissleMode
    {
        Inactive,
        Active
    };
private:
    struct CGunMorph
    {
        float x0_ = 0.f;
        float x4_;
        float x8_ = 0.f;
        float xc_ = 0.1f;
        float x10_;
        float x14_ = 2.f;
        float x18_ = 1.f;
        u32 x1c_ = 2;
        u32 x20_ = 1;

        union
        {
            struct
            {
                bool x24_24_ : 1;
                bool x24_25_ : 1;
            };
            u8 _dummy = 0;
        };

        CGunMorph(float a, float b)
        : x4_(a), x10_(std::fabs(b)) {}
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
    u32 x308_bombCount = 3;
    u32 x30c_ = 0;
    u32 x310_selectedBeam = 0;
    u32 x314_pendingSelectedBeam = 0;
    u32 x318_ = 0;
    EMissleMode x31c_missileMode = EMissleMode::Inactive;
    u32 x320_ = 0;
    u32 x324_ = 4;
    u32 x328_ = 0x2000;
    u32 x32c_ = 0;
    u32 x330_ = 0;
    u32 x334_ = 0;
    u32 x338_ = 0;
    u32 x33c_ = 0;
    float x340_chargeBeamFactor = 0.f;
    float x344_ = 0.f;
    float x348_ = 0.f;
    float x34c_ = 0.f;
    float x350_ = 0.f;
    float x354_;
    float x358_;
    float x35c_bombTime = 0.f;
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
    CGunWeapon* x72c_currentBeam = nullptr;
    u32 x730_ = 0;
    u32 x734_ = 0;
    u32 x738_ = 0;
    std::unique_ptr<CGunMotion> x73c_gunMotion;
    std::unique_ptr<CGrappleArm> x740_grappleArm;
    std::unique_ptr<CAuxWeapon> x744_auxWeapon;
    std::unique_ptr<CPowerBeam> x74c_powerBeam;
    std::unique_ptr<CIceBeam> x750_iceBeam;
    std::unique_ptr<CWaveBeam> x754_waveBeam;
    std::unique_ptr<CPlasmaBeam> x758_plasmaBeam;
    std::unique_ptr<CPhazonBeam> x75c_phazonBeam;
    CGunWeapon* x760_selectableBeams[4] = {};
    std::unique_ptr<CElementGen> x774_;
    std::unique_ptr<CWorldShadow> x82c_shadow;

    union
    {
        struct
        {
            bool x832_24_ : 1;
            bool x832_25_ : 1;
            bool x832_26_ : 1;
            bool x832_27_ : 1;
            bool x832_28_ : 1;
            bool x832_29_ : 1;
            bool x832_30_ : 1;
            bool x832_31_ : 1;

            bool x833_24_ : 1;
            bool x833_25_ : 1;
            bool x833_26_ : 1;
            bool x833_27_ : 1;
            bool x833_28_ : 1;
            bool x833_29_ : 1;
            bool x833_30_ : 1;
            bool x833_31_ : 1;

            bool x834_24_charging : 1;
            bool x834_25_ : 1;
            bool x834_26_ : 1;
            bool x834_27_ : 1;
            bool x834_28_ : 1;
            bool x834_29_ : 1;
            bool x834_30_ : 1;
            bool x834_31_ : 1;

            bool x835_24_ : 1;
            bool x835_25_ : 1;
            bool x835_26_ : 1;
            bool x835_27_ : 1;
            bool x835_28_bombReady : 1;
            bool x835_29_powerBombReady : 1;
            bool x835_30_ : 1;
            bool x835_31_ : 1;
        };
        u32 _dummy = 0;
    };

public:
    CPlayerGun(TUniqueId id);

    void AcceptScriptMessage(EScriptObjectMessage, TUniqueId, CStateManager&);
    void AsyncLoadSuit(CStateManager& mgr);
    void TouchModel(CStateManager& stateMgr);
    EMissleMode GetMissleMode() const { return x31c_missileMode; }
    bool IsCharging() const { return x834_24_charging; }
    float GetChargeBeamFactor() const { return x340_chargeBeamFactor; }
    bool IsBombReady() const { return x835_28_bombReady; }
    u32 GetBombCount() const { return x308_bombCount; }
    bool IsPowerBombReady() const { return x835_29_powerBombReady; }
    u32 GetSelectedBeam() const { return x310_selectedBeam; }
    u32 GetPendingSelectedBeam() const { return x314_pendingSelectedBeam; }
    const CGunMorph& GetGunMorph() const { return x678_morph; }
    void SetX3e8(const zeus::CTransform& xf) { x3e8_ = xf; }
    CGrappleArm& GetGrappleArm() { return *x740_grappleArm; }
};

}

#endif // __URDE_CPLAYERGUN_HPP__
