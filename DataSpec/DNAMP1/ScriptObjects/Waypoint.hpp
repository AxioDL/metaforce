#ifndef _DNAMP1_WAYPOINT_HPP_
#define _DNAMP1_WAYPOINT_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct Waypoint : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<atUint32> unknown4;
    Value<atUint32> unknown5;
    Value<atUint32> unknown6;
    Value<atUint32> unknown7;
    Value<atUint32> unknown8;
    Value<atUint32> unknown9;
    Value<atUint32> unknown10;
};
}
}

#endif
