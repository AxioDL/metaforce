#ifndef _DNAMP1_DEBRISEXTENDED_HPP_
#define _DNAMP1_DEBRISEXTENDED_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct DebrisExtended : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;
    Value<atVec4f> unknown10; // CColor
    Value<atVec4f> unknown11; // CColor
    Value<float> unknown12;
    Value<atVec3f> unknown13;
    Value<float> unknown14;
    Value<float> unknown15;
    Value<atVec3f> unknown16;
    UniqueID32 model;
    ActorParameters actorParameters;
    UniqueID32 particle1;
    Value<atVec3f> unknown17;
    Value<bool> unknown18;
    Value<bool> unknown19;
    Value<atUint32> unknown20;
    UniqueID32 particle2;
    Value<atVec3f> unknown21;
    Value<bool> unknown22;
    Value<bool> unknown23;
    Value<atUint32> unknown24;
    UniqueID32 particle3;
    Value<atVec3f> unknown25;
    Value<atUint32> unknown26;
    Value<bool> unknown27;
    Value<bool> unknown28;
    Value<bool> unknown29;
    Value<bool> unknown30;
};
}
}

#endif
