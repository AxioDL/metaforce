#ifndef _DNAMP1_CAMERAHINT_HPP_
#define _DNAMP1_CAMERAHINT_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct CameraHint : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> unknown1;
    Value<atUint32> unknown2;
    Value<atUint32> unknown3;
    struct CameraHintParameters : BigYAML
    {
        DECL_YAML
        Value<atUint32> propertyCount;
        Value<bool> unknown1;
        Value<bool> unknown2;
        Value<bool> unknown3;
        Value<bool> unknown4;
        Value<bool> unknown5;
        Value<bool> unknown6;
        Value<bool> unknown7;
        Value<bool> unknown8;
        Value<bool> unknown9;
        Value<bool> unknown10;
        Value<bool> unknown11;
        Value<bool> unknown12;
        Value<bool> unknown13;
        Value<bool> unknown14;
        Value<bool> unknown15;
        Value<bool> unknown16;
        Value<bool> unknown17;
        Value<bool> unknown18;
        Value<bool> unknown19;
        Value<bool> unknown20;
        Value<bool> unknown21;
        Value<bool> unknown22;
    } cameraHintParameters;

    struct BoolFloat : BigYAML
    {
        DECL_YAML
        Value<bool> unknown1;
        Value<float> unknown2;
    } boolFloat1[3];
    struct BoolVec3f : BigYAML
    {
        DECL_YAML
        Value<bool> unknown1;
        Value<atVec3f> unknown2;
    } boolVec3f[2];
    Value<atVec3f> unknown4;
    BoolFloat boolFloat2[4];
    Value<float> unknown5;
    Value<float> unknown6;
    BoolFloat boolFloat3;
    Value<float> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;
};
}
}

#endif
