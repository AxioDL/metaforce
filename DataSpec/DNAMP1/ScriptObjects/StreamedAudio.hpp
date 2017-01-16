#ifndef _DNAMP1_STREAMEDAUDIO_HPP_
#define _DNAMP1_STREAMEDAUDIO_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct StreamedAudio : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<bool> active;
    String<-1> fileName;
    Value<bool> noStopOnDeactivate;
    Value<float> fadeInTime;
    Value<float> fadeOutTime;
    Value<atUint32> volume;
    Value<atUint32> oneShot;
    Value<bool> music;
};
}
}

#endif
