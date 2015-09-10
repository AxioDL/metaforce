#ifndef _DNAMP1_ATOMICALPHA_HPP_
#define _DNAMP1_ATOMICALPHA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct AtomicAlpha : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    UniqueID32 wpsc;
    UniqueID32 model;
    DamageInfo damageInfo;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<bool>  unknown4;
    Value<bool>  unknown5;
};
}
}

#endif
