#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct FishCloudModifier : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> position;
    Value<bool> unknown1;
    Value<bool> unknown2;
    Value<bool> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
};
}

