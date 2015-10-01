#ifndef _DNAMP1_AIKEYFRAME_HPP_
#define _DNAMP1_AIKEYFRAME_HPP_

#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{

struct AIKeyframe : IScriptObject
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
