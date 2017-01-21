#ifndef _DNAMP1_CTWEAKTARGETING_HPP_
#define _DNAMP1_CTWEAKTARGETING_HPP_

#include "../../DNACommon/Tweaks/ITweakTargeting.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CTweakTargeting : public ITweakTargeting
{
    DECL_YAML
    Value<atUint32> x4_;
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
    Value<atVec3f> x30_;
    Value<atVec3f> x3c_;
    Value<float> x48_;
    Value<float> x4c_;
    Value<float> x50_;
    Value<float> x54_;
    Value<float> x58_;
    Value<float> x5c_;
    Value<atVec3f> x60_;
    Value<atVec3f> x6c_;
    Value<atVec3f> x78_;
    Value<atVec3f> x84_;
    Value<float> x90_;
    Value<float> x94_;
    Value<float> x98_;
    Value<float> x9c_;
    Value<float> xa0_;
    Value<float> xa4_;
    Value<float> xa8_;
    Value<float> xac_;
    Value<atVec4f> xb0_;
    Value<float> xb4_;
    Value<atVec4f> xb8_;
    Value<float> xbc_;
    Value<float> xc0_;
    Value<float> xc4_;
    Value<float> xc8_;
    Value<atVec4f> xcc_;
    Value<float> xd0_;
    Value<float> xd4_;
    Value<atVec4f> xd8_;
    Value<atVec4f> xdc_;
    Value<atVec4f> xe0_;
    Value<atVec4f> xe4_;
    Value<float> xe8_;
    Value<float> xec_;
    Value<float> xf0_;
    Value<atVec4f> xf4_;
    Value<atUint32> xf8_entryCount;
    struct UnkVec : BigYAML
    {
        DECL_YAML
        Value<atUint32> count;
        Vector<float, DNA_COUNT(count)> floats;
    };
    Vector<UnkVec, DNA_COUNT(xf8_entryCount)> xf8_;
    Value<atUint32> x108_entryCount;
    Vector<float, DNA_COUNT(x108_entryCount)> x108_;
    Value<float> x118_;
    Value<atVec4f> x11c_;
    Value<atUint32> x120_;
    Value<float> x124_;
    Value<float> x128_;
    Value<float> x12c_;
    Value<atVec4f> x130_;
    Value<float> x134_;
    Value<float> x138_;
    Value<atVec4f> x13c_;
    Value<float> x140_;
    Value<float> x144_;
    Value<float> x148_;
    Value<atVec4f> x14c_;
    Value<atVec4f> x150_;
    Value<float> x154_;
    Value<float> x158_;
    Value<float> x15c_;
    Value<float> x160_;
    Value<float> x164_;
    Value<float> x168_;
    Value<float> x16c_;
    Value<float> x170_;
    Value<float> x174_;
    Value<float> x178_;
    Value<float> x17c_;
    Value<float> x180_;
    Value<float> x184_;
    Value<float> x188_;
    Value<float> x18c_;
    Value<float> x190_;
    Value<float> x194_;
    Value<float> x198_;
    Value<float> x19c_;
    Value<float> x1a0_;
    Value<atVec4f> x1a4_;
    Value<atVec4f> x1a8_;
    Value<atVec4f> x1ac_;
    Value<float> x1b0_;
    Value<atVec4f> x1b4_;
    Value<float> x1b8_;
    Value<atVec4f> x1bc_;
    Value<atVec4f> x1c0_;
    Value<float> x1c4_;
    Value<bool> x1c8_;
    Value<atVec4f> x1cc_;
    Value<float> x1d0_;
    Value<atVec4f> x1d4_;
    Value<atVec4f> x1d8_;
    Value<atVec4f> x1dc_;
    Value<float> x1e0_;
    Value<float> x1e4_;
    Value<float> x1e8_;
    Value<float> x1ec_;
    Value<float> x1f0_;
    Value<float> x1f4_;
    Value<float> x1f8_;
    Value<float> x1fc_;
    Value<float> x200_;
    Value<float> x204_;
    Value<float> x208_;
    Value<float> x20c_;
    Value<float> x210_;
    Value<atVec4f> x214_;
    Value<float> x218_;
    Value<float> x21c_;
    Value<float> x220_;
    Value<float> x224_;

    CTweakTargeting() = default;
    CTweakTargeting(athena::io::IStreamReader& r)
    {
        this->read(r);
        x124_ = -1.f * ((2 * M_PIF) * (0.003f * x124_));
        x140_ = (2.f * M_PIF) * (0.003f * x140_);
        x144_ = (2.f * M_PIF) * (0.003f * x144_);
        x208_ = (2.f * M_PIF) * (0.003f * x208_);
    }
};
}
}
#endif // _DNAMP1_CTWEAKTARGETING_HPP_
