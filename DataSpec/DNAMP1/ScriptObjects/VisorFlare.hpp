#ifndef _DNAMP1_VISORFLARE_HPP_
#define _DNAMP1_VISORFLARE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
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

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        flareDefinitions[0].nameIDs(pakRouter, name + "_flare1");
        flareDefinitions[1].nameIDs(pakRouter, name + "_flare2");
        flareDefinitions[2].nameIDs(pakRouter, name + "_flare3");
        flareDefinitions[3].nameIDs(pakRouter, name + "_flare4");
        flareDefinitions[4].nameIDs(pakRouter, name + "_flare5");
    }
};
}
}

#endif
