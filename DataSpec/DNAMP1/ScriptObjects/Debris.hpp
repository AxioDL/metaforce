#ifndef _DNAMP1_DEBRIS_HPP_
#define _DNAMP1_DEBRIS_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Debris : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<float> unknown1;
    Value<atVec3f> unknown2;
    Value<atVec4f> unknown3; // CColor
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<atUint32> unknown7;
    Value<bool> unknown8;
    UniqueID32 model;
    ActorParameters actorParameters;
    UniqueID32 particle;
    Value<atVec3f> unknown9;
    Value<bool> unknown10;
    Value<bool> unknown11;
};
}
}

#endif
