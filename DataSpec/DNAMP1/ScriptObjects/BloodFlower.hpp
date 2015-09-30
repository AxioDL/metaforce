#ifndef _DNAMP1_BLOODFLOWER_HPP_
#define _DNAMP1_BLOODFLOWER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct BloodFlower : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    UniqueID32 particle1;
    UniqueID32 wpsc1;
    UniqueID32 wpsc2;
    DamageInfo damageInfo1;
    DamageInfo damageInfo2;
    DamageInfo damageInfo3;
    UniqueID32 particle2;
    UniqueID32 particle3;
    UniqueID32 particle4;
    Value<float> unknown1;
    UniqueID32 particle5;
    Value<float> unknown2;
};
}
}

#endif
