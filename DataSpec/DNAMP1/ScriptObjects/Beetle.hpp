#ifndef _DNAMP1_BEETLE_HPP_
#define _DNAMP1_BEETLE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Beetle : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> unknown1;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    DamageInfo damageInfo;
    Value<atVec3f> unknown2;
    Value<float> unknown3;
    DamageVulnerability damageVulnerabilty1;
    DamageVulnerability damageVulnerabilty2;
    UniqueID32 model;
    Value<atUint32> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
};
}
}

#endif
