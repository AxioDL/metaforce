#ifndef _DNAMP1_FOGVOLUME_HPP_
#define _DNAMP1_FOGVOLUME_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct FogVolume : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> volume;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<atVec4f> unkonwn3;
    Value<bool> unknown4;
};
}
}

#endif
