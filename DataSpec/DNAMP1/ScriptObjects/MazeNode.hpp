#ifndef _DNAMP1_MAZENODE_HPP_
#define _DNAMP1_MAZENODE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct MazeNode : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> unknown1;
    Value<atUint32> unknown2;
    Value<atUint32> unknown3;
    Value<atUint32> unknown4;
    Value<atVec3f> unknown5;
    Value<atVec3f> unknown6;
    Value<atVec3f> unknown7;
};
}

#endif
