#ifndef _DNAMP1_ACTORKEYFRAME_HPP_
#define _DNAMP1_ACTORKEYFRAME_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct ActorKeyframe : IScriptObject
{
    DECL_YAML
    String<-1>      name;
    Value<atUint32> unknown1;
    Value<bool>     unknown2;
    Value<float>    unknown3;
    Value<bool>     unknown4;
    Value<atUint32> unknown5;
    Value<float>    unknown6;
};
}
}

#endif
