#ifndef _DNAMP1_PUDDLESPORE_HPP_
#define _DNAMP1_PUDDLESPORE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct PuddleSpore : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> unknown1;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<bool> unknown2;
    UniqueID32 particle;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    UniqueID32 wpsc;
    DamageInfo damageInfo;
};
}
}

#endif
