#ifndef _DNAMP1_RANDOMRELAY_HPP_
#define _DNAMP1_RANDOMRELAY_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct RandomRelay : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> unknown1;
    Value<atUint32> unknown2;
    Value<bool> unknown3;
    Value<bool> unknown4;
};
}
}

#endif
