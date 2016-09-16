#ifndef _DNAMP1_CTWEAKSLIDESHOW_HPP_
#define _DNAMP1_CTWEAKSLIDESHOW_HPP_

#include "../../DNACommon/Tweaks/ITweakSlideShow.hpp"
#include "zeus/CColor.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct CTweakSlideShow : ITweakSlideShow
{
    DECL_YAML

    String<-1> x4_pakName;
    String<-1> x14_fontAssetName;
    DNAColor x24_;
    DNAColor x28_;
    Value<float> x2c_;
    Value<float> x30_;
    Value<float> x34_;
    Value<float> x38_;
    Value<float> x3c_;
    DNAColor x40_;
    Value<float> x44_;
    Value<float> x48_;
    Value<float> x4c_;
    Value<float> x50_;
    Value<float> x54_;
    Value<float> x58_;

    CTweakSlideShow() = default;
    CTweakSlideShow(athena::io::IStreamReader& in) { read(in); }
};

}
}

#endif // _DNAMP1_CTWEAKSLIDESHOW_HPP_
