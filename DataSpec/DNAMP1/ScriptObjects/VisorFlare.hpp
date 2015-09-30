#ifndef _DNAMP1_VISORFLARE_HPP_
#define _DNAMP1_VISORFLARE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct VisorFlare : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<bool> unknown1;
    Value<atUint32> unknown2;
    Value<bool> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<atUint32> unknown7;
    FlareDefinition flareDefinitions[5];
};
}
}

#endif
