#ifndef _DNAMP1_SPIDERBALLATTRACTIONSURFACE_HPP_
#define _DNAMP1_SPIDERBALLATTRACTIONSURFACE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct SpiderBallAttractionSurface : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<bool> active;
};
}
}

#endif
