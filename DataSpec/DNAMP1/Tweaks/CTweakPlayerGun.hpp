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
    Value<float> x3c_;
    Value<float> x40_;
    Value<float> x44_;
    Value<float> x48_;
    Value<zeus::CVector3f> x4c_;
    Value<zeus::CVector3f> x58_;
    Value<zeus::CVector3f> x64_;
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
    Value<float> x280_ricochetData[5]; // Originally rstl::prereserved_vector<float,5>
    Value<float> unused;               // Kept for consistency
    CTweakPlayerGun() = default;
    CTweakPlayerGun(athena::io::IStreamReader& r)
    {
        this->read(r);
        x44_ = zeus::degToRad(x44_);
    }

    float GetX24() const { return x24_; }
    float GetX28() const { return x28_; }
    float GetX2c() const { return x2c_; }
    float GetX30() const { return x30_; }
    float GetX34() const { return x34_; }
    float GetX38() const { return x38_; }
};
}
}

#endif // _DNAMP1_CTWEAKPLAYERGUN_HPP_
