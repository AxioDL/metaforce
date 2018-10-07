#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct PlayerStateChange : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<bool> unknown;
    Value<atUint32> item;
    Value<atUint32> unknown2;
    Value<atUint32> unknown3;
    Value<atUint32> unknown4;
    Value<atUint32> unknown5;
};
}

