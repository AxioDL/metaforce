#ifndef _DNAMP1_CONTROLLERACTION_HPP_
#define _DNAMP1_CONTROLLERACTION_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct ControllerAction : IScriptObject
{
    DECL_YAML
    Value<atUint32> propertyCount;
    String<-1> name;
    Value<bool> unknown1;
    Value<atUint32> unknown2;
    Value<bool> unknown3;
};
}
}

#endif
