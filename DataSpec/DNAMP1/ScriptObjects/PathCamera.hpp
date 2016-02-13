#ifndef _DNAMP1_PATHCAMERA_HPP_
#define _DNAMP1_PATHCAMERA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct PathCamera : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> unknown1;
    struct CameraParameters : BigYAML
    {
        DECL_YAML
        Value<atUint32> propertyCount;
        Value<bool> unknown1;
        Value<bool> unknown2;
        Value<bool> unknown3;
        Value<bool> unknown4;
        Value<bool> unknown5;
        Value<bool> unknown6;
    } cameraParameters;

    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<atUint32> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
};
}
}

#endif
