#ifndef _DNAMP1_CTWEAKGUI_HPP_
#define _DNAMP1_CTWEAKGUI_HPP_

#include "../../DNACommon/Tweaks/ITweakGui.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CTweakGui : ITweakGui
{
    DECL_YAML
    Value<bool> x4_;
    Value<float> x8_;
    Value<float> xc_;
    Value<float> x10_;
    Value<float> x14_;
    Value<float> x18_;
    Value<float> x1c_;
    Value<float> x20_;
    Value<float> x24_;
    Value<float> x28_;
    atUint32 x2c_ = 0;
    Value<float> x30_;
    Value<float> x34_;
    Value<float> x38_;
    Value<float> x3c_;
    Value<bool> x40_;
    Value<float> x44_;
    Value<float> x48_;
    Value<float> x4c_;
    Value<float> x50_;
    Value<float> x54_;
    Value<float> x58_;
    Value<float> x5c_;
    Value<float> x60_;
    Value<atVec3f> x64_;
    Value<atVec3f> x70_;
    Value<float> x7c_;
    Value<float> x80_;
    Value<float> x84_;
    Value<float> x88_;
    Value<float> x8c_;
    Value<float> x90_;
    Value<float> x94_;
    Value<float> x98_;
    Value<float> x9c_;
    Value<float> xa0_;
    Value<float> xa4_;
    Value<atUint32> xa8_;
    Value<atUint32> xac_;
    Value<atUint32> xb0_;
    Value<float> xb4_;
    Value<float> xb8_;
    Value<float> xbc_;
    Value<float> xc0_;
    Value<float> xc4_;
    Value<float> xc8_;
    Value<float> xcc_;
    Value<float> xd0_;
    Value<atUint32> xd4_;
    Value<float> xd8_;
    Value<float> xdc_;
    Value<float> xe0_;
    Value<float> xe4_;
    Value<float> xe8_;
    Value<float> xec_;
    Value<float> xf0_;
    Value<float> xf4_;
    Value<EHudVisMode> xf8_hudVisMode;
    Value<EHelmetVisMode> xfc_helmetVisMode;
    Value<atUint32> x100_enableAutoMapper;
    Value<atUint32> x104_;
    Value<atUint32> x108_enableTargetingManager;
    Value<atUint32> x10c_enablePlayerVisor;
    Value<float> x110_;
    Value<float> x114_;
    Value<float> x118_;
    Value<float> x11c_;
    Value<float> x120_;
    Value<float> x124_;
    Value<float> x128_;
    Value<float> x12c_;
    Value<bool> x130_;
    Value<float> x134_;
    Value<float> x138_;
    Value<atUint32> x13c_;
    Value<atUint32> x140_;
    Value<atUint32> x144_;
    Value<atUint32> x148_;
    Value<atUint32> x14c_;
    String<-1> x150_;
    String<-1> x160_;
    String<-1> x170_;
    String<-1> x180_;
    String<-1> x190_;
    Value<float> x1a0_;
    Value<float> x1a4_;
    Value<float> x1a8_;
    Value<float> x1ac_;
    Value<float> x1b0_;
    Value<float> x1b4_;
    Value<float> x1b8_;
    Value<float> x1bc_;
    Value<float> x1c0_;
    Value<float> x1c4_;
    Value<float> x1c8_;
    Value<bool> x1cc_;
    Value<bool> x1cd_;
    Value<float> x1d0_;
    Value<float> x1d4_;
    Value<float> x1d8_;
    Value<float> x1dc_;
    Value<float> x1e0_;
    Value<float> x1e4_;
    Value<float> x1e8_;
    Value<float> x1ec_;
    Value<float> x1f0_;
    Value<float> x1f4_;
    Value<float> x1f8_;
    Value<float> x1fc_;
    float x200_;
    float x204_xrayBlurScaleLinear = 0.0014f;
    float x208_xrayBlurScaleQuadratic = 0.0000525f;
    Value<float> x20c_;
    Value<float> x210_;
    Value<float> x214_;
    Value<float> x218_;
    Value<float> x21c_;
    Value<float> x220_;
    Value<float> x224_;
    float x228_;
    Value<float> x22c_;
    Value<float> x230_;
    Value<float> x234_;
    Value<float> x238_;
    Value<float> x23c_;
    Value<float> x240_;
    Value<float> x244_scanAppearanceOffset;
    Value<float> x248_;
    Value<float> x24c_;
    Value<float> x250_;
    Value<float> x254_;
    Value<float> x258_;
    Value<float> x25c_;
    Value<float> x260_;
    Value<float> x264_;
    Value<float> x268_;
    Value<float> x26c_;
    Value<float> x270_;
    Value<bool> x274_;
    bool x275_ = true;
    Value<float> x278_wtMgrCharsPerSfx;
    Value<atUint32> x27c_xrayFogMode;
    Value<float> x280_xrayFogNearZ;
    Value<float> x284_xrayFogFarZ;
    DNAColor x288_xrayFogColor;
    Value<float> x28c_thermalVisorLevel;
    DNAColor x290_thermalVisorColor;
    DNAColor x294_;
    DNAColor x298_;
    DNAColor x29c_;
    DNAColor x2a0_;
    DNAColor x2a4_;
    DNAColor x2a8_;
    DNAColor x2ac_;
    DNAColor x2b0_;
    DNAColor x2b4_;
    Value<float> x2b8_;
    Value<float> x2bc_;
    Value<float> x2c0_;
    Value<atUint32> m_scanSpeedsCount;
    Vector<float, DNA_COUNT(m_scanSpeedsCount)> x2c4_scanSpeeds;
    String<-1> x2d0_;
    String<-1> x2e0_;
    String<-1> x2f0_;
    DNAColor x300_;
    DNAColor x304_;
    Value<float> x308_;
    Value<float> x30c_;
    Value<float> x310_;
    String<-1> x314_;
    String<-1> x324_;
    String<-1> x334_;
    DNAColor x344_;
    DNAColor x348_;
    DNAColor x34c_;
    DNAColor x350_;
    DNAColor x354_;
    DNAColor x358_;
    Value<float> x35c_;
    Value<float> x360_;
    Value<float> x364_;

    CTweakGui() = default;
    CTweakGui(athena::io::IStreamReader& r) { this->read(r); }

    EHudVisMode GetHudVisMode() const { return xf8_hudVisMode; }
    EHelmetVisMode GetHelmetVisMode() const { return xfc_helmetVisMode; }
    atUint32 GetEnableAutoMapper() const { return x100_enableAutoMapper; }
    atUint32 GetEnableTargetingManager() const { return x108_enableTargetingManager; }
    atUint32 GetEnablePlayerVisor() const { return x10c_enablePlayerVisor; }
    float GetScanAppearanceOffset() const { return x244_scanAppearanceOffset; }
    float GetXrayBlurScaleLinear() const { return x204_xrayBlurScaleLinear; }
    float GetXrayBlurScaleQuadratic() const { return x208_xrayBlurScaleQuadratic; }
    float GetWorldTransManagerCharsPerSfx() const { return x278_wtMgrCharsPerSfx; }
    atUint32 GetXRayFogMode() const { return x27c_xrayFogMode; }
    float GetXRayFogNearZ() const { return x280_xrayFogNearZ; }
    float GetXRayFogFarZ() const { return x284_xrayFogFarZ; }
    const zeus::CColor& GetXRayFogColor() const { return x288_xrayFogColor; }
    float GetThermalVisorLevel() const { return x28c_thermalVisorLevel; }
    const zeus::CColor& GetThermalVisorColor() const { return x290_thermalVisorColor; }

    float GetScanSpeed(int idx) const
    {
        if (idx < 0 || idx >= x2c4_scanSpeeds.size())
            return 0.f;
        return x2c4_scanSpeeds[idx];
    }

    void FixupValues()
    {
        xd8_ = zeus::degToRad(xd8_);
        xdc_ = zeus::degToRad(xdc_);

        x200_ = x1f4_ * 0.25f;
        x204_xrayBlurScaleLinear = x1f8_ * 0.25f;
        x208_xrayBlurScaleQuadratic = x1fc_ * 0.25f;

        x210_ = zeus::degToRad(x210_);
        x228_ = x220_ + x224_;

        if (x27c_xrayFogMode == 1)
            x27c_xrayFogMode = 2;
        else if (x27c_xrayFogMode == 2)
            x27c_xrayFogMode = 4;
        else if (x27c_xrayFogMode == 3)
            x27c_xrayFogMode = 5;
        else
            x27c_xrayFogMode = 0;

        x84_ *= 2.0f;
    }
};

}
}

#endif // _DNAMP1_CTWEAKGUI_HPP_
