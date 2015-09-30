#ifndef _DNAMP1_FLICKERBAT_HPP_
#define _DNAMP1_FLICKERBAT_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct FlickerBat : IScriptObject
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
    Value<bool> unknown3;
    Value<bool> unknown4;
};
}
}

#endif
