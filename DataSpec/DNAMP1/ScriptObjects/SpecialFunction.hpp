#ifndef _DNAMP1_SPECIALFUNCTION_HPP_
#define _DNAMP1_SPECIALFUNCTION_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct SpecialFunction : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<ESpecialFunctionType> function;
    String<-1> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    struct LayerSwitch : BigYAML
    {
        DECL_YAML
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
}

#endif
