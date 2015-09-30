#ifndef _DNAMP1_STREAMEDAUDIO_HPP_
#define _DNAMP1_STREAMEDAUDIO_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct StreamedAudio : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<bool> active;
    String<-1> channels;
    Value<bool> unnknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<atUint32> unknown4;
    Value<atUint32> unknown5;
    Value<bool> unknown6;
};
}
}

#endif
