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
    Value<atUint32> xf8_;
    Value<atUint32> xfc_;
    Value<atUint32> x100_;
    Value<atUint32> x104_;
    Value<atUint32> x108_;
    Value<atUint32> x10c_;
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
    atVec4f x200_;
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
    Value<float> x244_;
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
    Value<bool> x275_ = true;
    Value<float> x278_;
    Value<atUint32> x27c_;
    Value<float> x280_;
    Value<float> x284_;
    Value<atVec4f> x288_;
    Value<float> x28c_;
    Value<atVec4f> x290_;
    Value<atVec4f> x294_;
    Value<atVec4f> x298_;
    Value<atVec4f> x29c_;
    Value<atVec4f> x2a0_;
    Value<atVec4f> x2a4_;
    Value<atVec4f> x2a8_;
    Value<atVec4f> x2ac_;
    Value<atVec4f> x2b0_;
    Value<atVec4f> x2b4_;
    Value<float> x2b8_;
    Value<float> x2bc_;
    Value<float> x2c0_;
    Value<atUint32> m_scanSpeedsCount;
    Vector<float, DNA_COUNT(m_scanSpeedsCount)> x2c4_scanSpeeds;
    String<-1> x2d0_;
    String<-1> x2e0_;
    String<-1> x2f0_;
    Value<atVec4f> x300_;
    Value<atVec4f> x304_;
    Value<float> x308_;
    Value<float> x30c_;
    Value<float> x310_;
    String<-1> x314_;
    String<-1> x324_;
    String<-1> x334_;
    Value<atVec4f> x344_;
    Value<atVec4f> x348_;
    Value<atVec4f> x34c_;
    Value<atVec4f> x350_;
    Value<atVec4f> x354_;
    Value<atVec4f> x358_;
    Value<float> x35c_;
    Value<float> x360_;
    Value<float> x364_;

    float GetXrayBlurScaleLinear() const { return x204_xrayBlurScaleLinear; }
    float GetXrayBlurScaleQuadratic() const { return x208_xrayBlurScaleQuadratic; }

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

        x200_.vec[0] = x1f4_ * 0.25f;
        x200_.vec[1] = x1f8_ * 0.25f;
        x200_.vec[2] = x1fc_ * 0.25f;
        x200_.vec[3] = 1.f;

        x210_ = zeus::degToRad(x210_);
        x228_ = x220_ + x224_;

        if (x27c_ == 1)
            x27c_ = 2;
        else if (x27c_ == 2)
            x27c_ = 4;
        else if (x27c_ == 3)
            x27c_ = 5;
        else
            x27c_ = 0;

        x84_ *= 2.0f;
    }
};

}
}

#endif // _DNAMP1_CTWEAKGUI_HPP_
