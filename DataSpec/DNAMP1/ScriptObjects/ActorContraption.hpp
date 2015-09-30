#ifndef _DNAMP1_ACTORCONTRAPTION_HPP_
#define _DNAMP1_ACTORCONTRAPTION_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct ActorContraption : IScriptObject
{
    DECL_YAML
    String<-1>          name;
    Value<atVec3f>      location;
    Value<atVec3f>      orientation;
    Value<atVec3f>      scale;
    Value<atVec3f>      unknown1;
    Value<atVec3f>      scanOffset;
    Value<float>        unknown2;
    Value<float>        unknown3;
    HealthInfo          healthInfo;
    DamageVulnerability damageVulnerability;
    AnimationParameters animationParameters;
    ActorParameters     actorParameters;
    UniqueID32          particle;
    DamageInfo          damageInfo;
    Value<bool>         active; // needs verification
};
}
}

#endif
