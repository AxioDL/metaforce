#ifndef __DNAMP1_CTWEAKAUTOMAPPER_HPP__
#define __DNAMP1_CTWEAKAUTOMAPPER_HPP__

#include "../../DataSpec/DNACommon/Tweaks/ITweakAutoMapper.hpp"
#include "zeus/CVector3f.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CTweakAutoMapper : public ITweakAutoMapper
{
    DECL_YAML
    Value<bool> x4_24_ : 1;
    Value<bool> x4_25_ : 1;
    Value<bool> x4_26_ : 1;
    Value<float> x8_;
    Value<float> xc_;
    Value<float> x10_;
    Value<float> x14_;
    Value<float> x18_;
    Value<float> x1c_;
    Value<float> x20_;
    DNAColor x24_;
    Value<float> x28_;
    Value<float> x2c_;
    Value<float> x30_;
    Value<float> x34_;
    DNAColor x38_;
    DNAColor x3c_;
    DNAColor x40_;
    DNAColor x44_;
    DNAColor x48_;
    DNAColor x4c_;
    DNAColor x50_;
    Value<float> x54_;
    Value<float> x58_;
    Value<float> x5c_;
    float x60_ = 0.4f;
    Value<float> x64_;
    Value<float> x68_;
    Value<float> x6c_;
    Value<float> x70_;
    Value<float> x74_;
    Value<float> x78_;
    DNAColor x7c_;
    DNAColor x80_;
    Value<float> x84_;
    Value<float> x88_;
    Value<float> x8c_;
    Value<float> x90_;
    Value<float> x94_;
    Value<float> x98_;
    Value<float> x9c_;
    Value<float> xa0_;
    /* Originally 4 separate floats */
    Value<zeus::CVector3f> xa4_doorCenter;
    Value<float> xb0_;
    Value<float> xb4_;
    Value<float> xb8_;
    Value<float> xbc_;
    Value<float> xc0_;
    Value<float> xc4_;
    Value<float> xc8_;
    Value<bool> xcc_;
    Value<float> xd0_;
    Value<float> xd4_;
    Value<float> xd8_;
    Value<float> xdc_;
    Value<float> xe0_;
    Value<float> xe4_;
    Value<float> xe8_;
    Value<float> xec_camVerticalOffset;
    DNAColor xf0_;
    DNAColor xf4_;
    DNAColor xf8_;
    DNAColor xfc_;
    Value<atUint32> x100_doorColorCount;
    Vector<DNAColor, DNA_COUNT(x100_doorColorCount)> x104_doorColors;
    DNAColor x118_doorBorderColor;
    DNAColor x11c_openDoorColor;

    CTweakAutoMapper() = default;
    CTweakAutoMapper(athena::io::IStreamReader& r) { this->read(r); }
    const zeus::CVector3f& GetDoorCenter() const { return xa4_doorCenter; }
    float GetCamVerticalOffset() const { return xec_camVerticalOffset; }
};
}
}

#endif // __DNAMP1_CTWEAKAUTOMAPPER_HPP__
