#ifndef _DNAMP1_PICKUPGENERATOR_HPP_
#define _DNAMP1_PICKUPGENERATOR_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct PickupGenerator : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> offset;
    Value<bool> active;
    Value<float> frequency;
};
}
}

#endif
