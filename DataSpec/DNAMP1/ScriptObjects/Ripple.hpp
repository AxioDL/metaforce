#ifndef _DNAMP1_RIPPLE_HPP_
#define _DNAMP1_RIPPLE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct Ripple : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<bool> unknown1;
    Value<float> unknown2;
};
}
}

#endif
