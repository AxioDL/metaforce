#ifndef _DNAMP1_CAMERABLURKEYFRAME_HPP_
#define _DNAMP1_CAMERABLURKEYFRAME_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct CameraBlurKeyframe : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<bool> unknown1;
    Value<atUint32> unknown2;
    Value<float> unknown3;
    Value<atUint32> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
};
}
}

#endif
