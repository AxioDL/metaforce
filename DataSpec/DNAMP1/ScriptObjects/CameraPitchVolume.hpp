#ifndef _DNAMP1_CAMERAPITCHVOLUME_HPP_
#define _DNAMP1_CAMERAPITCHVOLUME_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct CameraPitchVolume : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> volume;
    Value<bool> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
};
}

#endif
