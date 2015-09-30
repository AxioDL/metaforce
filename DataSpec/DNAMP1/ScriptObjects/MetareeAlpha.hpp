#ifndef _DNAMP1_METAREEALPHA_HPP_
#define _DNAMP1_METAREEALPHA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct MetareeAlpha : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    DamageInfo damageInfo;
    Value<float> unknown1;
    Value<atVec3f> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
};
}
}

#endif
