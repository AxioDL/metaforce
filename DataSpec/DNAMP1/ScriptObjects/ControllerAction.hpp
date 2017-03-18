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
    String<-1> name;
    Value<bool> active;
    Value<atUint32> command;
    Value<bool> deactivateOnClose;
};
}
}

#endif
