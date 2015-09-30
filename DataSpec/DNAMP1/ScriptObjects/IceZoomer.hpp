#ifndef _DNAMP1_ICEZOOMER_HPP_
#define _DNAMP1_ICEZOOMER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct IceZoomer : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<atUint32> unknown7;
    Value<atUint32> unknown8;
    DamageVulnerability damageVulnerabilty;
    Value<float> unknown9;
};
}
}

#endif
