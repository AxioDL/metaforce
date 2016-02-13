#ifndef _DNAMP1_CAMERAHINTRIGGER_HPP_
#define _DNAMP1_CAMERAHINTRIGGER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct CameraHintTrigger : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> volume;
    Value<bool> unknown1;
    Value<bool> unknown2;
    Value<bool> unknown3;
};
}
}

#endif
