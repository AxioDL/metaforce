#ifndef _DNAMP1_SEEDLING_HPP_
#define _DNAMP1_SEEDLING_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Seedling : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    UniqueID32 unknown1;
    UniqueID32 unknown2;
    DamageInfo damageInfo1;
    DamageInfo damageInfo2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
};
}
}

#endif
