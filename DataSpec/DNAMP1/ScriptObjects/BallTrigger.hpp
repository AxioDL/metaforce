#ifndef _DNAMP1_BALLTRIGGER_HPP_
#define _DNAMP1_BALLTRIGGER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct BallTrigger : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> volume;
    Value<bool> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<atVec3f> unknown5;
    Value<bool> unknown6;
};
}
}

#endif
