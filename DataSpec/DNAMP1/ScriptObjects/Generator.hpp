#ifndef _DNAMP1_GENERATOR_HPP_
#define _DNAMP1_GENERATOR_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Generator : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> unknown1;
    Value<bool> unknown2;
    Value<bool> unknown3;
    Value<atVec3f> unknown4;
    Value<bool> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
};
}

#endif
