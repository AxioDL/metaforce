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
    virtual float GetHardLandingVelocityThreshold() const=0;
    virtual float GetHudLagAmount() const=0;
    virtual float GetOrbitNormalDistance() const=0;
    virtual uint32_t GetOrbitScreenBoxHalfExtentX(int zone) const=0;
    virtual uint32_t GetOrbitScreenBoxHalfExtentY(int zone) const=0;
    virtual uint32_t GetOrbitScreenBoxCenterX(int zone) const=0;
    virtual uint32_t GetOrbitScreenBoxCenterY(int zone) const=0;
    virtual uint32_t GetEnemyScreenBoxCenterX(int zone) const=0;
    virtual uint32_t GetEnemyScreenBoxCenterY(int zone) const=0;
    virtual float GetOrbitNearX() const=0;
    virtual float GetOrbitNearZ() const=0;
    virtual float GetOrbitZRange() const=0;
    virtual float GetScanningRange() const=0; // x218
    virtual bool GetScanFreezesGame() const=0; // x21c_25
    virtual bool GetFiringCancelsCameraPitch() const=0;
    virtual float GetScanningFrameSenseRange() const=0;
    virtual float GetPlayerHeight() const=0; // x26c
    virtual float GetPlayerXYHalfExtent() const=0; // x270
    virtual bool GetX228_24() const=0; // x228_24
    virtual float GetX274() const=0; // x274
    virtual float GetX278() const=0; // x278
    virtual float GetPlayerBallHalfExtent() const=0; // x27c
    virtual float GetGrappleSwingPeriod() const=0;
    virtual float GetGrappleJumpForce() const=0;
    virtual float GetGrappleReleaseTime() const=0;
    virtual uint32_t GetGrappleJumpMode() const=0;
    virtual float GetX124() const=0; // x134
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
