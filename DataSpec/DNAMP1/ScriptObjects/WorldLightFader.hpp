#ifndef _DNAMP1_WORLDLIGHTFADER_HPP_
#define _DNAMP1_WORLDLIGHTFADER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct WorldLightFader : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<bool> activated;
    Value<float> fadedLevel;
    Value<float> intialLevel;
};
}
}

#endif
