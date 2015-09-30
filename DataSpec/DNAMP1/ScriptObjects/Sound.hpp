#ifndef _DNAMP1_SOUND_HPP_
#define _DNAMP1_SOUND_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Sound : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atUint32> soundID;
    Value<bool> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<atUint32> unknown5;
    Value<atUint32> unknown6;
    Value<atUint32> unknown7;
    Value<atUint32> unknown8;
    Value<bool> unknown9;
    Value<bool> unknown10;
    Value<bool> unknown11;
    Value<bool> unknown12;
    Value<bool> unknown13;
    Value<bool> unknown14;
    Value<bool> unknown15;
    Value<atUint32> unknown16;
};
}
}

#endif
