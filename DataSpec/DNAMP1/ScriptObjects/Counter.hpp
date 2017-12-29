#ifndef _DNAMP1_COUNTER_HPP_
#define _DNAMP1_COUNTER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Counter : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atUint32> startValue; // needs verification
    Value<atUint32> maxValue;
    Value<bool> unknown1;
    Value<bool> unknown2;
};
}

#endif
