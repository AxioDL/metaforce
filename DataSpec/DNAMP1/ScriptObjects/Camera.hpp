#ifndef _DNAMP1_CAMERA_HPP_
#define _DNAMP1_CAMERA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct Camera : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> active;
    Value<float> shotDuration;
    Value<bool> unknown3;
    Value<bool> unknown4;
    Value<bool> unknown5;
    Value<bool> unknown6;
    Value<bool> unknown7;
    Value<bool> disableInput;
    Value<bool> unknown9;
    Value<float> fov;
    Value<bool> unknown11;
    Value<bool> unknown12;
};
}
}

#endif
