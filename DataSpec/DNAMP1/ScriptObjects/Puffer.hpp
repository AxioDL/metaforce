#ifndef _DNAMP1_PUFFER_HPP_
#define _DNAMP1_PUFFER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Puffer : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<float> unknown1;
    UniqueID32 particle;
    DamageInfo damageInfo1;
    UniqueID32 texture;
    Value<float> unknown2;
    Value<bool> unknown3;
    Value<bool> unknown4;
    Value<bool> unknown5;
    DamageInfo damageInfo2;
    Value<atUint32> unknown6;
};
}
}

#endif
