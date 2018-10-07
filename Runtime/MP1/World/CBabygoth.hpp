#pragma once

#include "World/CPatterned.hpp"

namespace urde
{
class CBabygothData
{
    float x0_;
    float x4_;
    u32 x8_;
    CDamageInfo xc_;
    CDamageInfo x28_;
    u32 x44_;
    u32 x48_;
    CDamageInfo x4c_;
    CDamageVulnerability x68_;
    CDamageVulnerability xd0_;
    u32 x138_;
    u32 x13c_;
    float x140_;
    s16 x144_;
    u32 x148_;
    u32 x14c_;
    u32 x150_;
    u32 x154_;
    s16 x158_;
    s16 x15a_;
    s16 x15c_;
    float x160_;
    float x164_;
    float x168_;
    u32 x16c_;
    s16 x170_;
    u32 x174_;
public:
    CBabygothData(CInputStream&);
    CAssetId GetShellModelResId() const;
    void GetFireballDamage() const;
    CAssetId GetFireballResID() const;
    const CDamageVulnerability& GetShellDamageVulnerability() const;
    void GetSHellHitPoints() const;
    void GetFireballAttackVariance() const;
    void GetFireballAttackTime() const;
    void GetFireBreathDamage() const;
    CAssetId GetFireBreathResId() const;
};

}
