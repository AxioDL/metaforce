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
#include "Graphics/CRainSplashGenerator.hpp"

namespace urde
{
class CFinalInput;

class CPlayerGun
{
public:
    enum class EMissleMode
    {
        Inactive,
        Active
    };
    enum class EBWeapon
    {

    };
    enum class EGunOverrideMode
    {
        Normal,
        One,
        Two,
        Three
    };
private:
    class CGunMorph
    {
    public:
        enum class EGunState
        {
            Zero,
            One
        };
    private:
        float x0_ = 0.f;
        float x4_gunTransformTime;
        float x8_ = 0.f;
        float xc_ = 0.1f;
        float x10_holoHoldTime;
        float x14_ = 2.f;
        float x18_transitionFactor = 1.f;
        u32 x1c_ = 2;
        EGunState x20_gunState = EGunState::One;

        union
        {
            struct
            {
                bool x24_24_ : 1;
                bool x24_25_ : 1;
            };
            u32 _dummy = 0;
        };

    public:
        CGunMorph(float gunTransformTime, float holoHoldTime)
        : x4_gunTransformTime(gunTransformTime), x10_holoHoldTime(std::fabs(holoHoldTime)) {}
        float GetTransitionFactor() const { return x18_transitionFactor; }
        EGunState GetGunState() const { return x20_gunState; }
    };

    class CMotionState
    {
        static float gGunExtendDistance;
    public:
        static void SetExtendDistance(float d) { gGunExtendDistance = d; }
    };

    CActorLights x0_lights;
    u32 x2e0_ = 0;
    u32 x2e4_ = 0;
    u32 x2e8_ = 0;
    u32 x2ec_firing = 0;
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
    u32 x330_chargeWeaponIdx = 0;
    u32 x334_ = 0;
    u32 x338_ = 0;
    EGunOverrideMode x33c_gunOverrideMode = EGunOverrideMode::Normal;
    float x340_chargeBeamFactor = 0.f;
    float x344_ = 0.f;
    float x348_ = 0.f;
    float x34c_ = 0.f;
    float x350_ = 0.f;
    float x354_bombFuseTime;
    float x358_bombDropDelayTime;
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
    float x394_damageTimer = 0.f;
    float x398_damageAmt = 0.f;
    float x39c_ = 0.f;
    float x3a0_ = 0.f;
    CFidget x3a4_fidget;
    zeus::CVector3f x3dc_damageLocation;
    zeus::CTransform x3e8_xf;
    zeus::CTransform x418_;
    zeus::CTransform x448_;
    zeus::CTransform x478_assistAimXf;
    zeus::CTransform x4a8_;
    zeus::CTransform x4d8_;
    zeus::CTransform x508_;
    TUniqueId x538_playerId;
    TUniqueId x53a_ = kInvalidUniqueId;
    TUniqueId x53c_lightId = kInvalidUniqueId;
    std::vector<CToken> x540_handAnimTokens;
    CPlayerCameraBob x550_camBob;
    u32 x658_ = 1;
    float x65c_ = 0.f;
    float x660_ = 0.f;
    float x664_ = 0.f;
    float x668_aimVerticalSpeed;
    float x66c_aimHorizontalSpeed;
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
    CModelData x6e0_rightHandModel;
    CGunWeapon* x72c_currentBeam = nullptr;
    u32 x730_ = 0;
    CGunWeapon* x734_ = nullptr;
    CGunWeapon* x738_nextBeam = nullptr;
    std::unique_ptr<CGunMotion> x73c_gunMotion;
    std::unique_ptr<CGrappleArm> x740_grappleArm;
    std::unique_ptr<CAuxWeapon> x744_auxWeapon;
    std::unique_ptr<CRainSplashGenerator> x748_rainSplashGenerator;
    std::unique_ptr<CPowerBeam> x74c_powerBeam;
    std::unique_ptr<CIceBeam> x750_iceBeam;
    std::unique_ptr<CWaveBeam> x754_waveBeam;
    std::unique_ptr<CPlasmaBeam> x758_plasmaBeam;
    std::unique_ptr<CPhazonBeam> x75c_phazonBeam;
    CGunWeapon* x760_selectableBeams[4] = {}; // Used to be reserved_vector
    std::unique_ptr<CElementGen> x774_holoTransitionGen;
    std::unique_ptr<u32> x77c_;
    rstl::reserved_vector<rstl::reserved_vector<TLockedToken<CGenDescription>, 2>, 2> x784_bombEffects;
    rstl::reserved_vector<TLockedToken<CGenDescription>, 5> x7c0_auxMuzzleEffects;
    rstl::reserved_vector<std::unique_ptr<CElementGen>, 5> x800_auxMuzzleGenerators;
    std::unique_ptr<CWorldShadow> x82c_shadow;
    s16 x830 = -1;

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

            bool x833_24_isFidgeting : 1;
            bool x833_25_ : 1;
            bool x833_26_ : 1;
            bool x833_27_ : 1;
            bool x833_28_phazonBeamActive : 1;
            bool x833_29_ : 1;
            bool x833_30_ : 1;
            bool x833_31_ : 1;

            bool x834_24_charging : 1;
            bool x834_25_ : 1;
            bool x834_26_ : 1;
            bool x834_27_underwater : 1;
            bool x834_28_ : 1;
            bool x834_29_ : 1;
            bool x834_30_damaged : 1;
            bool x834_31_ : 1;

            bool x835_24_canFirePhazon : 1;
            bool x835_25_inPhazonBeam : 1;
            bool x835_26_phazonBeamMorphing : 1;
            bool x835_27_intoPhazonBeam : 1;
            bool x835_28_bombReady : 1;
            bool x835_29_powerBombReady : 1;
            bool x835_30_inPhazonPool : 1;
            bool x835_31_actorAttached : 1;
        };
        u32 _dummy = 0;
    };

    void InitBeamData();
    void InitBombData();
    void InitMuzzleData();
    void InitCTData();
    void LoadHandAnimTokens();
    void CreateGunLight(CStateManager& mgr);
    void DeleteGunLight(CStateManager& mgr);
    void SetGunLightActive(bool active, CStateManager& mgr);
    void SetPhazonBeamMorph(bool intoPhazonBeam);
    void Reset(CStateManager& mgr, bool b1);
    void ResetBeamParams(CStateManager& mgr, const CPlayerState& playerState, bool playSelectionSfx);
    void PlaySfx(u16 sfx, bool underwater, bool looped, float pan);
    void PlayAnim(NWeaponTypes::EGunAnimType type, bool b1);
    void CancelCharge(CStateManager& mgr, bool withEffect);

public:
    explicit CPlayerGun(TUniqueId playerId);
    void TakeDamage(bool attack, bool notFromMetroid, CStateManager& mgr);
    void AcceptScriptMsg(EScriptObjectMessage, TUniqueId, CStateManager&);
    void AsyncLoadSuit(CStateManager& mgr);
    void TouchModel(const CStateManager& stateMgr);
    EMissleMode GetMissleMode() const { return x31c_missileMode; }
    bool IsFidgeting() const { return x833_24_isFidgeting; }
    bool IsCharging() const { return x834_24_charging; }
    float GetChargeBeamFactor() const { return x340_chargeBeamFactor; }
    bool IsBombReady() const { return x835_28_bombReady; }
    u32 GetBombCount() const { return x308_bombCount; }
    bool IsPowerBombReady() const { return x835_29_powerBombReady; }
    u32 GetSelectedBeam() const { return x310_selectedBeam; }
    u32 GetPendingSelectedBeam() const { return x314_pendingSelectedBeam; }
    const CGunMorph& GetGunMorph() const { return x678_morph; }
    float GetHoloTransitionFactor() const { return x678_morph.GetTransitionFactor(); }
    void SetTransform(const zeus::CTransform& xf) { x3e8_xf = xf; }
    void SetAssistAimTransform(const zeus::CTransform& xf) { x478_assistAimXf = xf; }
    CGrappleArm& GetGrappleArm() { return *x740_grappleArm; }
    void DamageRumble(const zeus::CVector3f& location, float damage, const CStateManager& mgr);
    void ResetCharge(CStateManager& mgr, bool b1);
    void HandleBeamChange(const CFinalInput& input, CStateManager& mgr);
    void HandlePhazonBeamChange(CStateManager& mgr);
    void HandleWeaponChange(const CFinalInput& input, CStateManager& mgr);
    void ProcessInput(const CFinalInput& input, CStateManager& mgr);
    void ResetIdle(CStateManager& mgr);
    void CancelFiring(CStateManager& mgr);
    float GetBeamVelocity() const;
    void StopContinuousBeam(CStateManager& mgr, bool b1);
    void Update(float grappleSwingT, float cameraBobT, float dt, CStateManager& mgr);
    void PreRender(const CStateManager& mgr, const zeus::CFrustum& frustum, const zeus::CVector3f& camPos);
    void Render(const CStateManager& mgr, const zeus::CVector3f& pos, const CModelFlags& flags) const;
    void AddToRenderer(const zeus::CFrustum& frustum, const CStateManager& mgr) const;
    bool GetFiring() const { return x2ec_firing != 0; }
    TUniqueId DropPowerBomb(CStateManager& mgr);
    void SetActorAttached(bool b) { x835_31_actorAttached = b; }
};

}

#endif // __URDE_CPLAYERGUN_HPP__
