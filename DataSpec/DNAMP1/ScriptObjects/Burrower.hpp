#ifndef _DNAMP1_BURROWER_HPP_
#define _DNAMP1_BURROWER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Burrower : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    UniqueID32 particle1;
    UniqueID32 particle2;
    UniqueID32 wpsc;
    DamageInfo damageInfo;
    UniqueID32 particle3;
    Value<atUint32> unknown; // always FF
    UniqueID32 particle4;
};
}
}

#endif
