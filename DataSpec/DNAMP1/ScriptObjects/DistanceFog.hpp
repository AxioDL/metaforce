#ifndef _DNAMP1_DISTANCEFOG_HPP_
#define _DNAMP1_DISTANCEFOG_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct DistanceFog : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atUint32> fogMode;
    Value<atVec4f> fogColor; // CColor
    Value<atVec2f> range;
    Value<float> colorDelta;
    Value<atVec2f> rangeDelta;
    Value<bool> expl;
    Value<bool> active;
};
}

#endif
