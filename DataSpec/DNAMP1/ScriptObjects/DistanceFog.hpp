#ifndef _DNAMP1_DISTANCEFOG_HPP_
#define _DNAMP1_DISTANCEFOG_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct DistanceFog : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> unknown1;
    Value<atVec4f> unknown2; // CColor
    Value<atVec2f> unknown3;
    Value<atVec2f> unknown4;
    Value<float> unknown5;
    Value<bool> unknown6;
    Value<bool> unknown7;
};
}
}

#endif
