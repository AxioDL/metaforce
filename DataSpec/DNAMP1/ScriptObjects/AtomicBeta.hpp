#ifndef _DNAMP1_ATOMICBETA_HPP_
#define _DNAMP1_ATOMICBETA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct AtomicBeta : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    UniqueID32 elsc;
    UniqueID32 wpsc;
    DamageInfo damageInfo;
    UniqueID32 part;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    DamageVulnerability damageVulnerabilty;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<atUint32> unknown7;
    Value<atUint32> unknown8;
    Value<atUint32> unknown9;
    Value<float> unknown10;
};
}
}

#endif
