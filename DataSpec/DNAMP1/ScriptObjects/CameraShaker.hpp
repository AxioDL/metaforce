#ifndef _DNAMP1_CAMERASHAKER_HPP_
#define _DNAMP1_CAMERASHAKER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct CameraShaker : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<bool> unknown8;
};
}
}

#endif
