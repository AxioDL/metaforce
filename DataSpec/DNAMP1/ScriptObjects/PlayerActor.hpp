#ifndef _DNAMP1_PLAYERACTOR_HPP_
#define _DNAMP1_PLAYERACTOR_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct PlayerActor : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<atVec3f> unknown1;
    Value<atVec3f> scanOffset;
    Value<float> unknown2;
    Value<float> unknown3;
    HealthInfo healthInfo;
    DamageVulnerability damageVulnerability;
    UniqueID32 model;
    AnimationParameters animationParameters;
    ActorParameters actorParameters;
    Value<bool> unknown4;
    Value<bool> unknown5;
    Value<bool> unknown6;
    Value<bool> unknown7;
    struct PlayerParameters : BigYAML
    {
        DECL_YAML
        Value<atUint32> propertyCount;
        Value<bool> unknown1;
        Value<bool> unknown2;
        Value<bool> unknown3;
        Value<bool> unknown4;
        Value<bool> unknown5;
    } playerParameters;
    Value<atUint32> unknown8;
};
}
}

#endif
