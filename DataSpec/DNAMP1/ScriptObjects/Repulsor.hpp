#ifndef _DNAMP1_REPULSOR_HPP_
#define _DNAMP1_REPULSOR_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Repulsor : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<bool> active;
    Value<float> unknown;
};
}
}

#endif
