#ifndef _DNAMP1_METROIDALPHA_HPP_
#define _DNAMP1_METROIDALPHA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct MetroidAlpha : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> unknown1;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    DamageVulnerability damageVulnerabilty1;
    DamageVulnerability damageVulnerabilty2;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    AnimationParameters animationParameters1;
    AnimationParameters animationParameters2;
    AnimationParameters animationParameters3;
    AnimationParameters animationParameters4;
    Value<bool> unknown8;
};
}
}

#endif
