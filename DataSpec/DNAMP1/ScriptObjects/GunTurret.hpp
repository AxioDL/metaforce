#ifndef _DNAMP1_GUNTURRET_HPP_
#define _DNAMP1_GUNTURRET_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct GunTurret : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> unknown1;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<atVec3f> unknown2;
    Value<atVec3f> scanOffset;
    AnimationParameters animationParameters;
    ActorParameters actorParameters;
    HealthInfo healthInfo;
    DamageVulnerability damageVulnerabilty;
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
    Value<float> unknown16;
    Value<bool> unknown17;
    UniqueID32 unknown18;
    DamageInfo damageInfo;
    UniqueID32 particle1;
    UniqueID32 particle2;
    UniqueID32 particle3;
    UniqueID32 particle4;
    UniqueID32 particle5;
    UniqueID32 particle6;
    UniqueID32 particle7;
    Value<atUint32> unknown19;
    Value<atUint32> unknown20;
    Value<atUint32> unknown21;
    Value<atUint32> unknown22;
    Value<atUint32> unknown23;
    Value<atUint32> unknown24;
    UniqueID32 model;
    Value<float> unknown25;
    Value<atUint32> unknown26;
    Value<atUint32> unknown27;
    Value<atUint32> unknown28;
    Value<float> unknown29;
    Value<bool> unknown30;
};
}
}

#endif
