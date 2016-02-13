#ifndef _DNAMP1_NEWCAMERASHAKER_HPP_
#define _DNAMP1_NEWCAMERASHAKER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct NewCameraShaker : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<bool> unknown1;
    struct LongBool : BigYAML
    {
        DECL_YAML
        Value<atUint32> unknown1;
        Value<bool> unknown2;
    } unknown2;

    Value<float> unknown3;
    Value<float> unknown4;
    struct CameraShakerParameters : BigYAML
    {
        DECL_YAML
        Value<atUint32> unknown1;
        Value<bool> unknown2;
        Value<atUint32> unknown3;
        Value<bool> unknown4;
        Value<float> unknown5;
        Value<float> unknown6;
        Value<float> unknown7;
        Value<float> unknown8;
        Value<atUint32> unknown9;
        Value<bool> unknown10;
        Value<float> unknown11;
        Value<float> unknown12;
        Value<float> unknown13;
        Value<float> unknown14;
    } cameraShakerParameters1, cameraShakerParameters2, cameraShakerParameters3;
};
}
}

#endif
