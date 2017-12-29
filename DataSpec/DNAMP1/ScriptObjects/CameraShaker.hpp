#ifndef _DNAMP1_CAMERASHAKER_HPP_
#define _DNAMP1_CAMERASHAKER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct CameraShaker : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<float> xMag;
    Value<float> xB;
    Value<float> yMag;
    Value<float> yB;
    Value<float> zMag;
    Value<float> zB;
    Value<float> duration;
    Value<bool> active;
};
}

#endif
