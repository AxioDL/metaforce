#ifndef _DNAMP1_PHAZONHEALINGNODULE_HPP_
#define _DNAMP1_PHAZONHEALINGNODULE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct PhazonHealingNodule : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<bool> unknown1;
    UniqueID32 elsc;
    String<-1> unknown2;
};
}
}

#endif
