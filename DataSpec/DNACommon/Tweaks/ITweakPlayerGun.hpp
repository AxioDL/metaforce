#ifndef __DNACOMMON_ITWEAKPLAYERGUN_HPP__
#define __DNACOMMON_ITWEAKPLAYERGUN_HPP__

#include "ITweak.hpp"
#include "zeus/CAABox.hpp"

namespace DataSpec
{
struct SShotParam : BigYAML
{
    DECL_YAML
    Value<atUint32> weaponType = -1;
    Value<float> damage = 0.f;
    Value<float> radiusDamage = 0.f;
    Value<float> radius = 0.f;
    Value<float> knockback = 0.f;
    virtual bool Charged() const { return false; }
    virtual bool Comboed() const { return false; }
    virtual bool InstaKill() const { return false; }
};

struct SComboShotParam : SShotParam
{
    DECL_YAML
    bool Comboed() const { return true; }
};

struct SChargedShotParam : SShotParam
{
    DECL_YAML
    bool Charged() const { return true; }
};

struct SWeaponInfo : BigYAML
{
    DECL_YAML
    Value<float> x0_coolDown = 0.1f;
    SShotParam x4_normal;
    SChargedShotParam x20_charged;
};

struct ITweakPlayerGun : ITweak
{
    DECL_YAML
    virtual float GetUpLookAngle() const = 0;
    virtual float GetDownLookAngle() const = 0;
    virtual float GetVerticalSpread() const = 0;
    virtual float GetHorizontalSpread() const = 0;
    virtual float GetHighVerticalSpread() const = 0;
    virtual float GetHighHorizontalSpread() const = 0;
    virtual float GetLowVerticalSpread() const = 0;
    virtual float GetLowHorizontalSpread() const = 0;
    virtual float GetAimVerticalSpeed() const = 0; // x24
    virtual float GetAimHorizontalSpeed() const = 0; // x28
    virtual float GetBombFuseTime() const = 0; // x2c
    virtual float GetBombDropDelayTime() const = 0; // x30
    virtual float GetHoloHoldTime() const = 0; // x34
    virtual float GetGunTransformTime() const = 0; // x38
    virtual float GetGunHolsterTime() const = 0;
    virtual float GetGunNotFiringTime() const = 0;
    virtual float GetFixedVerticalAim() const = 0;
    virtual float GetGunExtendDistance() const = 0;
    virtual const zeus::CVector3f& GetGunPosition() const = 0;
    virtual const zeus::CVector3f& GetGrapplingArmPosition() const = 0;
    virtual float GetRichochetDamage(atUint32) const = 0;
    virtual const SWeaponInfo& GetBeamInfo(atInt32 beam) const = 0;
    virtual const SComboShotParam& GetComboShotInfo(atInt32 beam) const = 0;
    virtual const SShotParam& GetBombInfo() const=0;
    virtual const SShotParam& GetPowerBombInfo() const=0;
};
}

#endif // __DNACOMMON_ITWEAKPLAYERGUN_HPP__
