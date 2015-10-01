#ifndef _DNAMP1_MEMORYRELAY_HPP_
#define _DNAMP1_MEMORYRELAY_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct MemoryRelay : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<bool> unknown;
    Value<bool> active;
};
}
}

#endif
