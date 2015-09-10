#ifndef _DNAMP1_WALLCRAWLERSWARM_HPP_
#define _DNAMP1_WALLCRAWLERSWARM_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct WallCrawlerSwarm : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> volume;
    Value<bool> active;
    ActorParameters actorParameters;
    Value<atUint32> unknown1;
    AnimationParameters animationParameters;
    Value<atUint32> unknown2;
    Value<atUint32> unknown3;
    UniqueID32 particle1;
    UniqueID32 particle2;
    Value<atUint32> unknown4; // always FF
    Value<atUint32> unknown5; // always FF
    DamageInfo damageInfo1;
    Value<float> unknown6;
    DamageInfo damageInfo2;
    Value<float> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;
    Value<float> unknown10;
    Value<atUint32> unknown11;
    Value<atUint32> unknown12;
    Value<float> unknown13;
    Value<float> unknown14;
    Value<float> unknown15;
    Value<float> unknown16;
    Value<float> unknown17;
    Value<float> unknown18;
    Value<float> unknown19;
    Value<float> unknown20;
    Value<atUint32> unknown21;
    Value<float> unkown22;
    Value<float> unkown23;
    Value<float> unkown24;
    HealthInfo healthInfo;
    DamageVulnerability damageVulnerabilty;
    Value<atUint32> soundID1; // verification needed
    Value<atUint32> soundID2; // verification needed
};
}
}

#endif
