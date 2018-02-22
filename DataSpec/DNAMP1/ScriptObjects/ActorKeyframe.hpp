#ifndef _DNAMP1_ACTORKEYFRAME_HPP_
#define _DNAMP1_ACTORKEYFRAME_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct ActorKeyframe : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1>      name;
    Value<atUint32> animationId;
    Value<bool>     unknown1;
    Value<float>    unknown2;
    Value<bool>     active;
    Value<atUint32> unknown3;
    Value<float>    totalPlayback;
};
}

#endif
