#ifndef _DNAMP1_CTWEAKPLAYER_HPP_
#define _DNAMP1_CTWEAKPLAYER_HPP_

#include "../../DNACommon/Tweaks/ITweakPlayer.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct CTweakPlayer : ITweakPlayer
{
    Delete _d;
    Value<float> x4_maxTranslationalAcceleration[8];
    Value<float> x24_[8];
    Value<float> x44_translationFriction[8];
    Value<float> x64_[8];
    Value<float> x84_[8];
    Value<float> xa4_translationMaxSpeed[8];
    Value<float> xc4_normalGravAccel;
    Value<float> xc8_fluidGravAccel;
    Value<float> xcc_;
    Value<float> xd0_;
    Value<float> xd4_;
    Value<float> xd8_;
    Value<float> xdc_;
    Value<float> xe0_;
    Value<float> xe4_;
    Value<float> xe8_;
    Value<float> xec_;
    Value<float> xf0_;
    Value<float> xf4_;
    Value<float> xf8_;
    Value<float> xfc_;
    Value<float> x100_;
    Value<float> x104_;
    Value<float> x108_;
    Value<float> x10c_;
    Value<float> x110_;
    Value<float> x114_;
    Value<float> x118_;
    Value<float> x11c_;
    Value<float> x120_;
    Value<float> x124_eyeOffset;
    Value<float> x128_;
    Value<float> x12c_;
    Value<float> x130_horizontalFreeLookAngleVel;
    Value<float> x134_verticalFreeLookAngleVel;
    Value<float> x138_freeLookSpeed;
    Value<float> x13c_freeLookSnapSpeed;
    Value<float> x140_;
    Value<float> x144_freeLookCenteredThresholdAngle;
    Value<float> x148_freeLookCenteredTime;
    Value<float> x14c_;
    Value<float> x150_leftDiv;
    Value<float> x154_rightDiv;
    Value<float> x158_[3];
    Value<float> x164_[3];
    Value<float> x170_[3];
    Value<float> x17c_;
    Value<float> x180_orbitNormalDistance;
    Value<float> x184_;
    Value<float> x188_maxUpwardOrbitLookAngle;
    Value<float> x18c_maxDownwardOrbitLookAngle;
    Value<float> x190_orbitHorizAngle;
    Value<float> x194_;
    Value<float> x198_;
    Value<float> x19c_;
    Value<float> x1a0_;
    Value<float> x1a4_orbitDistanceThreshold;
    Value<atUint32> x1a8_orbitScreenBoxHalfExtentX[2];
    Value<atUint32> x1b0_orbitScreenBoxHalfExtentY[2];
    Value<atUint32> x1b8_orbitScreenBoxCenterX[2];
    Value<atUint32> x1c0_orbitScreenBoxCenterY[2];
    Value<atUint32> x1c8_orbitZoneIdealX[2];
    Value<atUint32> x1d0_orbitZoneIdealY[2];
    Value<float> x1d8_orbitNearX;
    Value<float> x1dc_orbitNearZ;
    Value<float> x1e0_;
    Value<float> x1e4_;
    Value<float> x1e8_;
    Value<float> x1ec_orbitZRange;
    Value<float> x1f0_;
    Value<float> x1f4_;
    Value<float> x1f8_;
    Value<float> x1fc_;
    Value<bool> x200_24_ : 1;
    Value<bool> x200_25_ : 1;
    Value<float> x204_;
    Value<float> x208_;
    Value<float> x20c_;
    Value<float> x210_;
    Value<float> x214_;
    Value<float> x218_scanningRange;
    Value<bool> x21c_24_scanRetention : 1;
    Value<bool> x21c_25_scanFreezesGame : 1;
    Value<bool> x21c_26_orbitWhileScanning : 1;
    Value<float> x220_;
    Value<float> x224_scanningFrameSenseRange;
    Value<bool> x228_24_freelookTurnsPlayer : 1;
    Value<bool> x228_25_ : 1;
    Value<bool> x228_26_ : 1;
    Value<bool> x228_27_ : 1;
    Value<bool> x228_28_enableFreeLook : 1;
    Value<bool> x228_29_twoButtonsForFreeLook : 1;
    Value<bool> x228_30_ : 1;
    Value<bool> x228_31_ : 1;
    Value<bool> x229_24_ : 1;
    Value<bool> x229_25_aimWhenOrbitingPoint : 1;
    Value<bool> x229_26_stayInFreeLookWhileFiring : 1;
    Value<bool> x229_27_ : 1;
    Value<bool> x229_28_ : 1;
    Value<bool> x229_29_ : 1;
    Value<bool> x229_30_gunButtonTogglesHolster : 1;
    Value<bool> x229_31_gunNotFiringHolstersGun : 1;
    Value<bool> x22a_24_ : 1;
    Value<bool> x22a_25_ : 1;
    Value<bool> x22a_26_firingCancelsCameraPitch : 1;
    Value<bool> x22a_27_assistedAimingIgnoreHorizontal : 1;
    Value<bool> x22a_28_assistedAimingIgnoreVertical : 1;
    Value<float> x22c_;
    Value<float> x230_;
    Value<float> x234_aimMaxDistance;
    Value<float> x238_;
    Value<float> x23c_;
    Value<float> x240_;
    Value<float> x244_;
    Value<float> x248_;
    Value<float> x24c_aimThresholdDistance;
    Value<float> x250_;
    Value<float> x254_;
    Value<float> x258_aimBoxWidth;
    Value<float> x25c_aimBoxHeight;
    Value<float> x260_aimTargetTimer;
    Value<float> x264_aimAssistHorizontalAngle;
    Value<float> x268_aimAssistVerticalAngle;
    Value<float> x26c_playerHeight;
    Value<float> x270_playerXYHalfExtent;
    Value<float> x274_;
    Value<float> x278_;
    Value<float> x27c_playerBallHalfExtent;
    Value<float> x280_;
    Value<float> x284_;
    Value<float> x288_;
    Value<float> x28c_;
    Value<float> x290_;
    Value<float> x294_;
    Value<float> x298_;
    Value<float> x29c_;
    Value<float> x2a0_orbitDistanceMax;
    Value<float> x2a4_grappleSwingLength;
    Value<float> x2a8_grappleSwingPeriod;
    Value<float> x2ac_grapplePullSpeedMin;
    Value<float> x2b0_;
    Value<float> x2b4_maxGrappleLockedTurnAlignDistance;
    Value<float> x2b8_grapplePullSpeedProportion;
    Value<float> x2bc_grapplePullSpeedMax;
    Value<float> x2c0_grappleLookCenterSpeed;
    Value<float> x2c4_maxGrappleTurnSpeed;
    Value<float> x2c8_grappleJumpForce;
    Value<float> x2cc_grappleReleaseTime;
    Value<atUint32> x2d0_grappleJumpMode;
    Value<bool> x2d4_;
    Value<bool> x2d5_invertGrappleTurn;
    Value<float> x2d8_;
    Value<float> x2dc_;
    Value<float> x2e0_;
    Value<float> x2e4_;
    Value<float> x2e8_;
    Value<float> x2ec_;
    Value<float> x2f0_;
    Value<bool>  x2f4_;
    Value<float> x2f8_frozenTimeout;
    Value<atUint32> x2fc_iceBreakJumpCount;
    Value<float> x300_variaDamageReduction;
    Value<float> x304_gravityDamageReduction;
    Value<float> x308_phazonDamageReduction;
    float GetMaxTranslationalAcceleration(int s) const { return x4_maxTranslationalAcceleration[s]; }
    float GetPlayerTranslationFriction(int s) const { return x44_translationFriction[s]; }
    float GetPlayerTranslationMaxSpeed(int s) const { return xa4_translationMaxSpeed[s]; }
    float GetNormalGravAccel() const { return xc4_normalGravAccel; }
    float GetFluidGravAccel() const { return xc8_fluidGravAccel; }
    float GetEyeOffset() const { return x124_eyeOffset; }
    float GetFreeLookSpeed() const { return x138_freeLookSpeed; }
    float GetFreeLookSnapSpeed() const { return x13c_freeLookSnapSpeed; }
    float GetFreeLookCenteredThresholdAngle() const { return x144_freeLookCenteredThresholdAngle; }
    float GetFreeLookCenteredTime() const { return x148_freeLookCenteredTime; }
    float GetOrbitNormalDistance() const { return x180_orbitNormalDistance; }
    float GetMaxUpwardOrbitLookAngle() const { return x188_maxUpwardOrbitLookAngle; }
    float GetMaxDownwardOrbitLookAngle() const { return x18c_maxDownwardOrbitLookAngle; }
    float GetOrbitHorizAngle() const { return x190_orbitHorizAngle; }
    float GetOrbitDistanceThreshold() const { return x1a4_orbitDistanceThreshold; }
    uint32_t GetOrbitScreenBoxHalfExtentX(int zone) const { return x1a8_orbitScreenBoxHalfExtentX[zone]; }
    uint32_t GetOrbitScreenBoxHalfExtentY(int zone) const { return x1b0_orbitScreenBoxHalfExtentY[zone]; }
    uint32_t GetOrbitScreenBoxCenterX(int zone) const { return x1b8_orbitScreenBoxCenterX[zone]; }
    uint32_t GetOrbitScreenBoxCenterY(int zone) const { return x1c0_orbitScreenBoxCenterY[zone]; }
    uint32_t GetOrbitZoneIdealX(int zone) const { return x1c8_orbitZoneIdealX[zone]; }
    uint32_t GetOrbitZoneIdealY(int zone) const { return x1d0_orbitZoneIdealY[zone]; }
    float GetOrbitNearX() const { return x1d8_orbitNearX; }
    float GetOrbitNearZ() const { return x1dc_orbitNearZ; }
    float GetOrbitZRange() const { return x1ec_orbitZRange; }
    float GetScanningRange() const { return x218_scanningRange; }
    bool GetScanRetention() const { return x21c_24_scanRetention; }
    bool GetScanFreezesGame() const { return x21c_25_scanFreezesGame; }
    bool GetOrbitWhileScanning() const { return x21c_26_orbitWhileScanning; }
    float GetScanningFrameSenseRange() const { return x224_scanningFrameSenseRange; }
    bool GetEnableFreeLook() const { return x228_28_enableFreeLook; }
    bool GetTwoButtonsForFreeLook() const { return x228_29_twoButtonsForFreeLook; }
    bool GetAimWhenOrbitingPoint() const { return x229_25_aimWhenOrbitingPoint; }
    bool GetStayInFreeLookWhileFiring() const { return x229_26_stayInFreeLookWhileFiring; }
    bool GetGunButtonTogglesHolster() const { return x229_30_gunButtonTogglesHolster; }
    bool GetGunNotFiringHolstersGun() const { return x229_31_gunNotFiringHolstersGun; }
    bool GetFiringCancelsCameraPitch() const { return x22a_26_firingCancelsCameraPitch; }
    bool GetAssistedAimingIgnoreHorizontal() const { return x22a_27_assistedAimingIgnoreHorizontal; }
    bool GetAssistedAimingIgnoreVertical() const { return x22a_28_assistedAimingIgnoreVertical; }
    float GetAimMaxDistance() const { return x234_aimMaxDistance; }
    float GetAimThresholdDistance() const { return x24c_aimThresholdDistance; }
    float GetAimBoxWidth() const { return x258_aimBoxWidth; }
    float GetAimBoxHeight() const { return x25c_aimBoxHeight; }
    float GetAimTargetTimer() const { return x260_aimTargetTimer; }
    float GetAimAssistHorizontalAngle() const { return x264_aimAssistHorizontalAngle; }
    float GetAimAssistVerticalAngle() const { return x268_aimAssistVerticalAngle; }
    float GetPlayerHeight() const { return x26c_playerHeight; }
    float GetPlayerXYHalfExtent() const { return x270_playerXYHalfExtent; }
    float GetX274() const { return x274_; }
    float GetX278() const { return x278_; }
    float GetPlayerBallHalfExtent() const { return x27c_playerBallHalfExtent; }
    float GetOrbitDistanceMax() const { return x2a0_orbitDistanceMax; }
    float GetGrappleSwingLength() const { return x2a4_grappleSwingLength; }
    float GetGrappleSwingPeriod() const { return x2a8_grappleSwingPeriod; }
    float GetGrapplePullSpeedMin() const { return x2ac_grapplePullSpeedMin; }
    float GetMaxGrappleLockedTurnAlignDistance() const { return x2b4_maxGrappleLockedTurnAlignDistance; }
    float GetGrapplePullSpeedProportion() const { return x2b8_grapplePullSpeedProportion; }
    float GetGrapplePullSpeedMax() const { return x2bc_grapplePullSpeedMax; }
    float GetGrappleLookCenterSpeed() const { return x2c0_grappleLookCenterSpeed; }
    float GetMaxGrappleTurnSpeed() const { return x2c4_maxGrappleTurnSpeed; }
    float GetGrappleJumpForce() const { return x2c8_grappleJumpForce; }
    float GetGrappleReleaseTime() const { return x2cc_grappleReleaseTime; }
    uint32_t GetGrappleJumpMode() const { return x2d0_grappleJumpMode; }
    bool InvertGrappleTurn() const { return x2d5_invertGrappleTurn; }
    float GetHorizontalFreeLookAngleVel() const { return x130_horizontalFreeLookAngleVel; }
    float GetVerticalFreeLookAngleVel() const { return x134_verticalFreeLookAngleVel; }
    float GetX184() const { return x184_; }
    float GetX1fc() const { return x1fc_; }
    bool GetFreeLookTurnsPlayer() const { return x228_24_freelookTurnsPlayer; }
    float GetX288() const { return x288_; }
    float GetX28c() const { return x28c_; }
    float GetX290() const { return x290_; }
    float GetX294() const { return x294_; }
    float GetX298() const { return x298_; }
    float GetX29C() const { return x29c_; }
    float GetX280() const { return x280_; }
    float GetX2B0() const { return x2b0_; }
    float GetX14C() const { return x14c_; }
    float GetLeftLogicalThreshold() const { return x150_leftDiv; }
    float GetRightLogicalThreshold() const { return x154_rightDiv; }
    float GetX164(int type) const { return x164_[type]; }
    float GetFrozenTimeout() const { return x2f8_frozenTimeout; }
    uint32_t GetIceBreakJumpCount() const { return x2fc_iceBreakJumpCount; }
    float GetVariaDamageReduction() const { return x300_variaDamageReduction; }
    float GetGravityDamageReduction() const { return x304_gravityDamageReduction; }
    float GetPhazonDamageReduction() const { return x308_phazonDamageReduction; }
    CTweakPlayer() = default;
    CTweakPlayer(athena::io::IStreamReader& reader)
    {
        read(reader);
        FixupValues();
    }

    void read(athena::io::IStreamReader& __dna_reader);
    void write(athena::io::IStreamWriter& __dna_writer) const;
    void read(athena::io::YAMLDocReader& __dna_docin);
    void write(athena::io::YAMLDocWriter& __dna_docout) const;
    void FixupValues();
    static const char* DNAType();
    const char* DNATypeV() const { return DNAType(); }

    size_t binarySize(size_t __isz) const;
};

}
}

#endif // _DNAMP1_CTWEAKPLAYER_HPP_
