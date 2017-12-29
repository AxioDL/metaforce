#ifndef _DNAMP1_RADIALDAMAGE_HPP_
#define _DNAMP1_RADIALDAMAGE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct RadialDamage : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<bool> active;
    DamageInfo damageInfo;
    Value<float> radius;
};
}

#endif
