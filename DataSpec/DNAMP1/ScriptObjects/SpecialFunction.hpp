#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct SpecialFunction : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<ESpecialFunctionType> function;
    String<-1> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    struct LayerSwitch : BigDNA
    {
        AT_DECL_DNA
        Value<atUint32> area;
        Value<atUint32> layerIdx;
    } layerSwitch;
    Value<EPickupType> pickup;
    Value<bool> unknown5;
    Value<float> unknown6;
    Value<atUint32> unknown7; // Used by SpinnerController 1
    Value<atUint32> unknown8; // Used by SpinnerController 2
    Value<atUint32> unknown9; // Used by SpinnerController 3
};
}

