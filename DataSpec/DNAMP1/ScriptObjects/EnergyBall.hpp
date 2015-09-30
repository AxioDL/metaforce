#ifndef _DNAMP1_ENERGYBALL_HPP_
#define _DNAMP1_ENERGYBALL_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct EnergyBall : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<atUint32> unknown1;
    Value<float> unknown2;
    DamageInfo damageInfo1;
    Value<float> unknown3;
    UniqueID32 texture;
    Value<atUint32> soundID1;
    UniqueID32 particle1;
    UniqueID32 elsc;
    Value<atUint32> soundID2;
    Value<float> unknown4;
    Value<float> unknown5;
    UniqueID32 particle2;
    DamageInfo damageInfo2;
    Value<float> unknown6;
};
}
}

#endif
