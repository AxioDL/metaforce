#ifndef _DNAMP1_SPINDLECAMERA_HPP_
#define _DNAMP1_SPINDLECAMERA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct SpindleCamera : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> unknown1;
    PlayerParameters playerParameters;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    struct SpindleCameraParameters : BigYAML
    {
        DECL_YAML
        Value<atUint32> unknown1;
        PlayerParameters playerParameters;
        Value<float> unknown5;
        Value<float> unknown6;
        Value<float> unknown7;
        Value<float> unknown8;
    } spindleCameraParameters[15];
};
}

#endif
