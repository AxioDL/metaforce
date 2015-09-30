#ifndef _DNAMP1_WARWASP_HPP_
#define _DNAMP1_WARWASP_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Warwasp : IScriptObject
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
    DamageInfo damageInfo1;
    UniqueID32 wpsc1;
    DamageInfo damageInfo2;
    UniqueID32 particle;
    Value<atUint32> unknown3;
};
}
}

#endif
