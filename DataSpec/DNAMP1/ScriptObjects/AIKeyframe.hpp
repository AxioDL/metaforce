#pragma once

#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{

struct AIKeyframe : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1>      name;
    Value<atUint32> unknown1;
    Value<bool>     unknown2;
    Value<float>    unknown3;
    Value<bool>     unknown4;
    Value<atUint32> unknown5;
    Value<float>    unknown6;
};
}

