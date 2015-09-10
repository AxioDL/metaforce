#ifndef _DNAMP1_FLAAHGRA_HPP_
#define _DNAMP1_FLAAHGRA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Flaahgra : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters1;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    DamageVulnerability damageVulnerabilty;
    UniqueID32 wpsc1;
    DamageInfo damageInfo1;
    UniqueID32 wpsc2;
    DamageInfo damageInfo2;
    UniqueID32 particle;
    DamageInfo damageInfo3;
    ActorParameters actorParameters2;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    AnimationParameters animationParameters;
    UniqueID32 dependencyGroup;
};
}
}

#endif
