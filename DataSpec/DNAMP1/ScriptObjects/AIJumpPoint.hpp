#ifndef _DNAMP1_AIJUMPPOINT_HPP_
#define _DNAMP1_AIJUMPPOINT_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct AIJumpPoint : IScriptObject
{
    DECL_YAML
    String<-1>      name;
    Value<atVec3f>  location;
    Value<atVec3f>  orientation;
    Value<bool>     active;
    Value<float>    unknown1;
};
}
}

#endif
