#ifndef _DNAMP1_ROOMACOUSTICS_HPP_
#define _DNAMP1_ROOMACOUSTICS_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct RoomAcoustics : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<bool> unknown1;
    Value<atUint32> unknown2;
    Value<bool> unknown3;
    Value<bool> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;
    Value<float> unknown10;
    Value<bool> unknown11;
    Value<float> unknown12;
    Value<float> unknown13;
    Value<float> unknown14;
    Value<bool> unknown15;
    Value<bool> unknown16;
    Value<float> unknown17;
    Value<float> unknown18;
    Value<float> unknown19;
    Value<float> unknown20;
    Value<float> unknown21;
    Value<bool> unknown22;
    Value<atUint32> unknown23;
    Value<atUint32> unknown24;
    Value<atUint32> unknown25;
    Value<atUint32> unknown26;
    Value<atUint32> unknown27;
    Value<atUint32> unknown28;
    Value<atUint32> unknown29;
    Value<atUint32> unknown30;
    Value<atUint32> unknown31;
};
}
}

#endif
