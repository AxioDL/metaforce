#ifndef _DNAMP1_NEWINTROBOSS_HPP_
#define _DNAMP1_NEWINTROBOSS_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct NewIntroBoss : IScriptObject
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
    DamageInfo damageInfo;
    UniqueID32 particle1;
    UniqueID32 particle2;
    UniqueID32 texture1;
    UniqueID32 texture2;
};
}
}

#endif
