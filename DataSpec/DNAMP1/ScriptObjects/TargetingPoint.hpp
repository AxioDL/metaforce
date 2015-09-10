#ifndef _DNAMP1_TARGETTINGPOINT_HPP_
#define _DNAMP1_TARGETTINGPOINT_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct TargetingPoint : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> active;
};
}
}

#endif
