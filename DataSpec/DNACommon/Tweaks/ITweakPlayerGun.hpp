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

struct ITweakPlayerGun : ITweak
{
    DECL_YAML
    virtual float GetX24() const = 0; // x24
    virtual float GetX28() const = 0; // x28
    virtual float GetX2c() const = 0; // x2c
    virtual float GetX30() const = 0; // x30
    virtual float GetX34() const = 0; // x34
    virtual float GetX38() const = 0; // x38
    virtual float GetGunHolsterTime() const=0;
    virtual float GetGunNotFiringTime() const=0;
    virtual float GetRichochetDamage(atUint32) const = 0;
};
}

#endif // __DNACOMMON_ITWEAKPLAYERGUN_HPP__
