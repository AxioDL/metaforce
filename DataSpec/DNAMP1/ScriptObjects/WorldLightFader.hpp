#ifndef _DNAMP1_WORLDLIGHTFADER_HPP_
#define _DNAMP1_WORLDLIGHTFADER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct WorldLightFader : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<bool> active;
    Value<float> fadedLevel;
    Value<float> intialLevel;
};
}
}

#endif
