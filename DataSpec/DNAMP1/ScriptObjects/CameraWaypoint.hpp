#ifndef _DNAMP1_CAMERAWAYPOINT_HPP_
#define _DNAMP1_CAMERAWAYPOINT_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct CameraWaypoint : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> unknown1;
    Value<float> unknown2;
    Value<atUint32> unknown3;
};
}
}

#endif
