#ifndef _DNAMP1_THARDUSROCKPROJECTILE_HPP_
#define _DNAMP1_THARDUSROCKPROJECTILE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct ThardusRockProjectile : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<bool> unknown1;
    Value<bool> unknown2;
    Value<float> unknown3;
    UniqueID32 model;
    UniqueID32 stateMachine;
};
}
}

#endif
