#ifndef _DNAMP1_RUMBLEEFFECT_HPP_
#define _DNAMP1_RUMBLEEFFECT_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct RumbleEffect : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<bool> unknown1;
    Value<float> unknown2;
    Value<atUint32> unknown3;
    struct RumbleEffectParameters : BigYAML
    {
        DECL_YAML
        Value<atUint32> propertyCount;
        Value<bool> unknown1;
        Value<bool> unknown2;
    } rumbleParameters;
};
}

#endif
