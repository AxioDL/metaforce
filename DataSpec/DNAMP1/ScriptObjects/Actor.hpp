#ifndef _DNAMP1_ACTOR_HPP_
#define _DNAMP1_ACTOR_HPP_

#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Actor : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<atVec3f> unkown1;
    Value<atVec3f> scanOffset;
    Value<float>   unknown2;
    Value<float>   unknown3;
    HealthInfo     healthInfo;
    DamageVulnerability damageVulnerability;
    UniqueID32     model;
    AnimationParameters animationParameters;
    ActorParameters actorParameters;
    Value<bool>     unkown4;
    Value<bool>     unkown5;
    Value<bool>     unkown6;
    Value<bool>     unkown7;
    Value<bool>     unkown8;
    Value<atUint32> unknown9;
    Value<float>    unknown10;
    Value<bool>     unknown11;
    Value<bool>     unknown12;
    Value<bool>     unknown13;
    Value<bool>     unknown14;
};
}
}

#endif
