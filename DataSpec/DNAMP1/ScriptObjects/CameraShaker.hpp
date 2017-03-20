#ifndef _DNAMP1_CAMERASHAKER_HPP_
#define _DNAMP1_CAMERASHAKER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CameraShaker : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<float> xA;
    Value<float> xB;
    Value<float> yA;
    Value<float> yB;
    Value<float> zA;
    Value<float> zB;
    Value<float> duration;
    Value<bool> active;
};
}
}

#endif
