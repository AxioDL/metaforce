#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Camera : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
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

