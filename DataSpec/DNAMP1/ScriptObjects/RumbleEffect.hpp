#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct RumbleEffect : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> location;
    Value<bool> unknown1;
    Value<float> unknown2;
    Value<atUint32> unknown3;
    struct RumbleEffectParameters : BigDNA
    {
        AT_DECL_DNA
        Value<atUint32> propertyCount;
        Value<bool> unknown1;
        Value<bool> unknown2;
    } rumbleParameters;
};
}

