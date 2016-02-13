#ifndef _DNAMP1_COVERPOINT_HPP_
#define _DNAMP1_COVERPOINT_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CoverPoint : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> unknown1;
    Value<atUint32> unknown2;
    Value<bool> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
};
}
}

#endif
