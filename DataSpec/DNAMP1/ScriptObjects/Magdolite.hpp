#ifndef _DNAMP1_MAGDOLITE_HPP_
#define _DNAMP1_MAGDOLITE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Magdolite : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<float> unknown1;
    Value<float> unknown2;
    DamageInfo damageInfo1;
    DamageInfo damageInfo2;
    DamageVulnerability damageVulnerabilty1;
    DamageVulnerability damageVulnerabilty2;
    UniqueID32 model;
    UniqueID32 skin;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    struct MagdoliteParameters : BigYAML
    {
        DECL_YAML
        Value<atUint32> propertyCount;
        Value<atUint32> unknown1;
        UniqueID32 particle;
        Value<atUint32> unknown2;
        Value<float> unknown3;
        Value<float> unknown4;
        Value<float> unknown5;
    } magdoliteParameters;
    Value<float> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;
};
}
}

#endif
