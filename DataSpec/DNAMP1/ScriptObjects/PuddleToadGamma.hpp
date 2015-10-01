#ifndef _DNAMP1_PUDDLETOADGAMMA_HPP_
#define _DNAMP1_PUDDLETOADGAMMA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct PuddleToadGamma : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> unknown1;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<atVec3f> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<float> unknown8;
    DamageInfo damageInfo1;
    DamageInfo damageInfo2;
    UniqueID32 dcln;
};
}
}

#endif
