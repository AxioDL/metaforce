#ifndef _DNAMP1_ACTORROTATE_HPP_
#define _DNAMP1_ACTORROTATE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct ActorRotate : IScriptObject
{
    DECL_YAML
    String<-1>      name;
    Value<atVec3f>  rotationOffset;
    Value<float>    timeScale;
    Value<bool>     unknown1;
    Value<bool>     unknown2;
    Value<bool>     active;
};
}

#endif
