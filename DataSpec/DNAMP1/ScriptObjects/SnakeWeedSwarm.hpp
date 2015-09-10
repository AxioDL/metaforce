#ifndef _DNAMP1_SNAKEWEEDSWARM_HPP_
#define _DNAMP1_SNAKEWEEDSWARM_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct SnakeWeedSwarm : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> volume;
    Value<bool> unknown1;
    AnimationParameters animationParameters;
    ActorParameters actorParameters;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;
    Value<float> unknown10;
    Value<float> unknown11;
    Value<float> unknown12;
    Value<float> unknown13;
    Value<float> unknown14;
    Value<float> unknown15;
    DamageInfo damageInfo;
    Value<float> unknown16;
    Value<atUint32> unknown17;
    Value<atUint32> unknown18;
    Value<atUint32> unknown19;
};
}
}

#endif
