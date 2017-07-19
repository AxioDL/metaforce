#ifndef _DNAMP1_CTWEAKPLAYERGUN_HPP_
#define _DNAMP1_CTWEAKPLAYERGUN_HPP_

#include "../../DNACommon/Tweaks/ITweakPlayerGun.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct CTweakPlayerGun : ITweakPlayerGun
{
    DECL_YAML
    Value<float> x4_;
    Value<float> x8_;
    Value<float> xc_;
    Value<float> x10_;
    Value<float> x14_;
    Value<float> x18_;
    Value<float> x1c_;
    Value<float> x20_;
    Value<float> x24_;
    Value<float> x28_;
    Value<float> x2c_;
    Value<float> x30_;
    Value<float> x34_;
    Value<float> x38_;
    Value<float> x3c_gunHolsterTime;
    Value<float> x40_gunNotFiringTime;
    Value<float> x44_fixedVerticalAim;
    Value<float> x48_;
    Value<zeus::CVector3f> x4c_gunPosition;
    Value<zeus::CVector3f> x58_;
    Value<zeus::CVector3f> x64_grapplingArmPosition;
    SShotParam x70_bomb;
    SShotParam x8c_powerBomb;
    SShotParam x1d4_missile;
    struct SWeaponInfo : BigYAML
    {
        DECL_YAML
        Value<float> x0_coolDown = 0.1f;
        SShotParam x4_normal;
        SChargedShotParam x20_charged;
    };
    SWeaponInfo xa8_beams[5];
    SComboShotParam x1f0_combos[5];    // Originally rstl::prereserved_vector<SShotParam,5>
    Value<float> x280_ricochetData[6]; // Originally rstl::prereserved_vector<float,5>, extended to 6 to capture
                                       // PhazonBeam's value
    CTweakPlayerGun() = default;
    CTweakPlayerGun(athena::io::IStreamReader& r)
    {
        this->read(r);
        x44_fixedVerticalAim = zeus::degToRad(x44_fixedVerticalAim);
    }

    float GetX24() const { return x24_; }
    float GetX28() const { return x28_; }
    float GetX2c() const { return x2c_; }
    float GetX30() const { return x30_; }
    float GetX34() const { return x34_; }
    float GetX38() const { return x38_; }
    float GetGunHolsterTime() const { return x3c_gunHolsterTime; }
    float GetGunNotFiringTime() const { return x40_gunNotFiringTime; }
    float GetFixedVerticalAim() const { return x44_fixedVerticalAim; }
    const zeus::CVector3f& GetGunPosition() const { return x4c_gunPosition; }
    const zeus::CVector3f& GetGrapplingArmPosition() const { return x64_grapplingArmPosition; }
    float GetRichochetDamage(atUint32 type) const
    {
        switch (type)
        {
        case 0: // Power
            return x280_ricochetData[0];
        case 1: // Ice
            return x280_ricochetData[1];
        case 2: // Wave
            return x280_ricochetData[2];
        case 3: // Plasma
            return x280_ricochetData[3];
        case 6: // Missile
            return x280_ricochetData[4];
        case 8: // Phazon
                /* Note: In order to return the same value as retail we have to do a bit of a hack
                 * Retro accidentally forgot to load in PhazonBeam's richochet value, as a result, it loads the
                 * pointer to CTweakParticle's vtable.
                 */
#if MP_v1088

            return float(0x803D9CC4);
#else
            return x280_ricochetData[5];
#endif
        default:
            return 1.f;
        }
    }

    SWeaponInfo GetBeamInfo(atInt32 beam) const
    {
        if (beam < 0 || beam > 5)
            return xa8_beams[0];
        return xa8_beams[beam];
    }
};
}
}

#endif // _DNAMP1_CTWEAKPLAYERGUN_HPP_
