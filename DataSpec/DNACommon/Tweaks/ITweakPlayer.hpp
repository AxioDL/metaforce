#ifndef __DNACOMMON_ITWEAKPLAYER_HPP__
#define __DNACOMMON_ITWEAKPLAYER_HPP__

#include "ITweak.hpp"
#include "zeus/CAABox.hpp"

namespace DataSpec
{

struct ITweakPlayer : ITweak
{
    virtual float GetX50() const=0;
    virtual float GetX54() const=0;
    virtual float GetX58() const=0;
    virtual float GetX5C() const=0;
    virtual float GetNormalGravAccel() const=0;
    virtual float GetFluidGravAccel() const=0;
    virtual float GetEyeOffset() const=0;
    virtual float GetHudLagAmount() const=0;
    virtual float GetFreeLookCenteredThresholdAngle() const=0;
    virtual float GetFreeLookCenteredTime() const=0;
    virtual float GetOrbitNormalDistance() const=0;
    virtual float GetMaxUpwardOrbitLookAngle() const=0;
    virtual float GetMaxDownwardOrbitLookAngle() const=0;
    virtual float GetOrbitHorizAngle() const=0;
    virtual float GetOrbitDistanceThreshold() const=0;
    virtual uint32_t GetOrbitScreenBoxHalfExtentX(int zone) const=0;
    virtual uint32_t GetOrbitScreenBoxHalfExtentY(int zone) const=0;
    virtual uint32_t GetOrbitScreenBoxCenterX(int zone) const=0;
    virtual uint32_t GetOrbitScreenBoxCenterY(int zone) const=0;
    virtual uint32_t GetOrbitZoneIdealX(int zone) const=0;
    virtual uint32_t GetOrbitZoneIdealY(int zone) const=0;
    virtual float GetOrbitNearX() const=0;
    virtual float GetOrbitNearZ() const=0;
    virtual float GetOrbitZRange() const=0;
    virtual float GetScanningRange() const=0; // x218
    virtual bool GetScanRetention() const=0;
    virtual bool GetScanFreezesGame() const=0; // x21c_25
    virtual bool GetOrbitWhileScanning() const=0;
    virtual bool GetFiringCancelsCameraPitch() const=0;
    virtual bool GetAssistedAimingIgnoreHorizontal() const=0;
    virtual bool GetAssistedAimingIgnoreVertical() const=0;
    virtual float GetAimMaxDistance() const=0;
    virtual float GetAimThresholdDistance() const=0;
    virtual float GetAimBoxWidth() const=0;
    virtual float GetAimBoxHeight() const=0;
    virtual float GetAimTargetTimer() const=0;
    virtual float GetAimAssistHorizontalAngle() const=0;
    virtual float GetAimAssistVerticalAngle() const=0;
    virtual float GetScanningFrameSenseRange() const=0;
    virtual bool GetEnableFreeLook() const=0;
    virtual bool GetTwoButtonsForFreeLook() const=0;
    virtual bool GetAimWhenOrbitingPoint() const=0;
    virtual bool GetStayInFreeLookWhileFiring() const=0;
    virtual bool GetGunButtonTogglesHolster() const=0;
    virtual bool GetGunNotFiringHolstersGun() const=0;
    virtual float GetPlayerHeight() const=0; // x26c
    virtual float GetPlayerXYHalfExtent() const=0; // x270
    virtual bool GetX228_24() const=0; // x228_24
    virtual float GetX274() const=0; // x274
    virtual float GetX278() const=0; // x278
    virtual float GetPlayerBallHalfExtent() const=0; // x27c
    virtual float GetOrbitDistanceMax() const=0;
    virtual float GetGrappleSwingLength() const=0;
    virtual float GetGrappleSwingPeriod() const=0;
    virtual float GetGrapplePullSpeedMin() const=0;
    virtual float GetMaxGrappleLockedTurnAlignDistance() const=0;
    virtual float GetGrapplePullSpeedProportion() const=0;
    virtual float GetGrapplePullSpeedMax() const=0;
    virtual float GetGrappleLookCenterSpeed() const=0;
    virtual float GetMaxGrappleTurnSpeed() const=0;
    virtual float GetGrappleJumpForce() const=0;
    virtual float GetGrappleReleaseTime() const=0;
    virtual uint32_t GetGrappleJumpMode() const=0;
    virtual bool InvertGrappleTurn() const=0;
    virtual float GetHorizontalFreeLookAngleVel() const=0;
    virtual float GetVerticalFreeLookAngleVel() const=0; // x134
    virtual float GetX184() const=0; // x184
    virtual float GetX1fc() const=0;
    virtual float GetX288() const=0; // x288
    virtual float GetX28c() const=0; // x28c
    virtual float GetX290() const=0; // x290
    virtual float GetX294() const=0; // x294
    virtual float GetX298() const=0; // x298
    virtual float GetX29C() const=0; // x29c
    virtual float GetX280() const=0; // x280
    virtual float GetX2B0() const=0; // x2b0
    virtual float GetX138() const=0; // x138
    virtual float GetX14C() const=0; // x14c
    virtual float GetLeftLogicalThreshold() const=0;
    virtual float GetRightLogicalThreshold() const=0;
    virtual float GetX164(int type) const=0;
    virtual float GetFrozenTimeout() const=0;
    virtual uint32_t GetIceBreakJumpCount() const=0;
    virtual float GetVariaDamageReduction() const=0;
    virtual float GetGravityDamageReduction() const=0;
    virtual float GetPhazonDamageReduction() const=0;
};

}

#endif // __DNACOMMON_ITWEAKPLAYER_HPP__
