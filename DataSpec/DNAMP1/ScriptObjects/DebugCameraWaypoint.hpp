#ifndef _DNAMP1_DEBUGCAMERAWAYPOINT_HPP_
#define _DNAMP1_DEBUGCAMERAWAYPOINT_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct DebugCameraWaypoint : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atUint32> unknown1;
};
}
}

#endif
