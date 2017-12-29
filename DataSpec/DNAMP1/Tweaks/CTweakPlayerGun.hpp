#ifndef _DNAMP1_CTWEAKPLAYERGUN_HPP_
#define _DNAMP1_CTWEAKPLAYERGUN_HPP_

#include "../../DNACommon/Tweaks/ITweakPlayerGun.hpp"

namespace DataSpec::DNAMP1
{

struct CTweakPlayerGun final : ITweakPlayerGun
{
    DECL_YAML
    Value<float> x4_upLookAngle;
    Value<float> x8_downLookAngle;
    Value<float> xc_verticalSpread;
    Value<float> x10_horizontalSpread;
    Value<float> x14_highVerticalSpread;
    Value<float> x18_highHorizontalSpread;
    Value<float> x1c_lowVerticalSpread;
    Value<float> x20_lowHorizontalSpread;
    Value<float> x24_aimVerticalSpeed;
    Value<float> x28_aimHorizontalSpeed;
    Value<float> x2c_bombFuseTime;
    Value<float> x30_bombDropDelayTime;
    Value<float> x34_holoHoldTime;
    Value<float> x38_gunTransformTime;
    Value<float> x3c_gunHolsterTime;
    Value<float> x40_gunNotFiringTime;
    Value<float> x44_fixedVerticalAim;
    Value<float> x48_gunExtendDistance;
    Value<zeus::CVector3f> x4c_gunPosition;
    Value<zeus::CVector3f> x58_;
    Value<zeus::CVector3f> x64_grapplingArmPosition;
    SShotParam x70_bomb;
    SShotParam x8c_powerBomb;
    SShotParam x1d4_missile;
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

    float GetUpLookAngle() const { return x4_upLookAngle; }
    float GetDownLookAngle() const { return x8_downLookAngle; }
    float GetVerticalSpread() const { return xc_verticalSpread; }
    float GetHorizontalSpread() const { return x10_horizontalSpread; }
    float GetHighVerticalSpread() const { return x14_highVerticalSpread; }
    float GetHighHorizontalSpread() const { return x18_highHorizontalSpread; }
    float GetLowVerticalSpread() const { return x1c_lowVerticalSpread; }
    float GetLowHorizontalSpread() const { return x20_lowHorizontalSpread; }
    float GetAimVerticalSpeed() const { return x24_aimVerticalSpeed; }
    float GetAimHorizontalSpeed() const { return x28_aimHorizontalSpeed; }
    float GetBombFuseTime() const { return x2c_bombFuseTime; }
    float GetBombDropDelayTime() const { return x30_bombDropDelayTime; }
    float GetHoloHoldTime() const { return x34_holoHoldTime; }
    float GetGunTransformTime() const { return x38_gunTransformTime; }
    float GetGunHolsterTime() const { return x3c_gunHolsterTime; }
    float GetGunNotFiringTime() const { return x40_gunNotFiringTime; }
    float GetFixedVerticalAim() const { return x44_fixedVerticalAim; }
    float GetGunExtendDistance() const { return x48_gunExtendDistance; }
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

    const SWeaponInfo& GetBeamInfo(atInt32 beam) const
    {
        if (beam < 0 || beam > 5)
            return xa8_beams[0];
        return xa8_beams[beam];
    }

    const SComboShotParam& GetComboShotInfo(atInt32 beam) const
    {
        if (beam < 0 || beam > 5)
            return x1f0_combos[0];
        return x1f0_combos[beam];
    }

    const SShotParam& GetBombInfo() const { return x70_bomb; }
    const SShotParam& GetPowerBombInfo() const { return x8c_powerBomb; }
    const SShotParam& GetMissileInfo() const { return x1d4_missile; }
};
}

#endif // _DNAMP1_CTWEAKPLAYERGUN_HPP_
