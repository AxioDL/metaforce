#ifndef _DNAMP1_WORLDTELEPORTER_HPP_
#define _DNAMP1_WORLDTELEPORTER_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct WorldTeleporter : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<bool> unknown1;
    UniqueID32 mlvl;
    UniqueID32 mrea;
    AnimationParameters animationParameters;
    Value<atVec3f> unknown2;
    UniqueID32 model1;
    Value<atVec3f> unknown3;
    UniqueID32 model2;
    Value<atVec3f> unknown4;
    Value<bool> unknown5;
    Value<atUint32> soundID; // needs verifcation
    Value<atUint32> unknown6;
    Value<atUint32> unknown7;
    Value<bool> unknown8;
    UniqueID32 font;
    UniqueID32 strg;
    Value<bool> unknown9;
    Value<float> unknown10;
    Value<float> unknown11;
    Value<float> unknown12;
};
}
}

#endif
