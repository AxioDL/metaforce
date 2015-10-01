#ifndef _DNAMP1_Babygoth_HPP_
#define _DNAMP1_Babygoth_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Babygoth : IScriptObject
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
    UniqueID32 wpsc1;
    DamageInfo damageInfo1;
    DamageInfo damageInfo2;
    UniqueID32 wpsc2;
    UniqueID32 particle1;
    DamageInfo damageInfo3;
    DamageVulnerability damageVulnerabilty1;
    DamageVulnerability damageVulnerabilty2;
    UniqueID32 model;
    UniqueID32 skin;
    Value<float> unknown3;
    Value<atUint32> unknown4;
    UniqueID32 particle2;
    UniqueID32 particle3;
    UniqueID32 particle4;
    UniqueID32 particle5;
    Value<atUint32> unknown5;
    Value<atUint32> unknown6;
    Value<atUint32> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;
    Value<float> unknown10;
    UniqueID32 texture;
    Value<atUint32> unknown11;
    UniqueID32 particle6;
};
}
}

#endif
