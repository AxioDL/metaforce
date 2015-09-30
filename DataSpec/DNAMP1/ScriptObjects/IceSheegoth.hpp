#ifndef _DNAMP1_ICESHEEGOTH_HPP_
#define _DNAMP1_ICESHEEGOTH_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct IceSheegoth : IScriptObject
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
    Value<atVec3f> unknown3;
    Value<float> unknown4;
    DamageVulnerability damageVulnerabilty1;
    DamageVulnerability damageVulnerabilty2;
    DamageVulnerability damageVulnerabilty3;
    UniqueID32 wpsc1;
    DamageInfo damageInfo1;
    Value<float> unknown5;
    Value<float> unknown6;
    UniqueID32 wpsc2;
    UniqueID32 particle1;
    DamageInfo damageInfo2;
    UniqueID32 particle2;
    UniqueID32 particle3;
    UniqueID32 particle4;
    UniqueID32 particle5;
    UniqueID32 elsc;
    Value<float> unknown7;
    Value<float> unknown8;
    DamageInfo damageInfo3;
    Value<atUint32> soundID1;
    Value<float> unknown9;
    Value<float> unknown10;
    Value<float> unknown11;
    UniqueID32 texture;
    Value<atUint32> soundID2;
    UniqueID32 particle6;
    Value<bool> unknown12;
    Value<bool> unknown13;
};
}
}

#endif
