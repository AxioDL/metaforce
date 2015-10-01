#ifndef _DNAMP1_CAMERA_HPP_
#define _DNAMP1_CAMERA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Camera : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> unknown1;
    Value<float> unknown2;
    Value<bool> unknown3;
    Value<bool> unknown4;
    Value<bool> unknown5;
    Value<bool> unknown6;
    Value<bool> unknown7;
    Value<bool> unknown8;
    Value<bool> unknown9;
    Value<float> unknown10;
    Value<bool> unknown11;
    Value<bool> unknown12;
};
}
}

#endif
