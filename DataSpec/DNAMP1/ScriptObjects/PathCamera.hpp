#ifndef _DNAMP1_PATHCAMERA_HPP_
#define _DNAMP1_PATHCAMERA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct PathCamera : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> unknown1;
    struct CameraParameters : BigDNA
    {
        AT_DECL_DNA
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

#endif
