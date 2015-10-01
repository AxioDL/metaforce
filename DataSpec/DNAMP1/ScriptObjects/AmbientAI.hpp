#ifndef _DNAMP1_AMBIENTAI_HPP_
#define _DNAMP1_AMBIENTAI_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct AmbientAI : IScriptObject
{
    DECL_YAML
    String<-1>          name;
    Value<atVec3f>      location;
    Value<atVec3f>      orientation;
    Value<atVec3f>      scale;
    Value<atVec3f>      unknown1;
    Value<atVec3f>      scanOffset;
    Value<float>        unknown2;
    HealthInfo          healthInfo;
    DamageVulnerability damageVulnerability;
    AnimationParameters animationParameters;
    ActorParameters     actorParameters;
    Value<float>        unknown3;
    Value<float>        unknown4;
    Value<atUint32>     unknown5;
    Value<atUint32>     unknown6;
    Value<bool>         unknown7;
};
}
}

#endif
