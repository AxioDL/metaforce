#ifndef _DNAMP1_COLORMODULATE_HPP_
#define _DNAMP1_COLORMODULATE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct ColorModulate : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec4f> unknown1;
    Value<atVec4f> unknown2;
    Value<atUint32> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<bool> unknown6;
    Value<bool> unknown7;
    Value<bool> unknown8;
    Value<bool> unknown9;
    Value<bool> unknown10;
    Value<bool> unknown11;
};
}

#endif
