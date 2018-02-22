#ifndef _DNAMP1_CONTROLLERACTION_HPP_
#define _DNAMP1_CONTROLLERACTION_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct ControllerAction : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<bool> active;
    Value<atUint32> command;
    Value<bool> deactivateOnClose;
};
}

#endif
