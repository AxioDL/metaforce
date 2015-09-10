#ifndef _DNAMP1_METROIDPRIMESTAGE2_HPP_
#define _DNAMP1_METROIDPRIMESTAGE2_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct MetroidPrimeStage2 : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    UniqueID32 particle1;
    DamageInfo damageInfo;
    UniqueID32 elsc;
    Value<atUint32> unknown;
    UniqueID32 particle2;
};
}
}

#endif
