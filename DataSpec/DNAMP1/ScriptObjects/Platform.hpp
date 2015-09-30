#ifndef _DNAMP1_PLATFORM_HPP_
#define _DNAMP1_PLATFORM_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Platform : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<atVec3f> unknown1;
    Value<atVec3f> scanOffset;
    UniqueID32 model;
    AnimationParameters animationParameters;
    ActorParameters actorParameters;
    Value<float> unknown2;
    Value<bool> active;
    UniqueID32 dcln;
    HealthInfo healthInfo;
    DamageVulnerability damageVulnerabilty;
    Value<bool> unknown3;
    Value<float> unknown4;
    Value<bool> unknown5;
    Value<atUint32> unknown6;
    Value<atUint32> unknown7;
};
}
}

#endif
