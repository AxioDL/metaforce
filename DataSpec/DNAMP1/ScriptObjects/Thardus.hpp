#ifndef _DNAMP1_THARDUS_HPP_
#define _DNAMP1_THARDUS_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Thardus : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<bool> unknown1;
    Value<bool> unknown2;
    UniqueID32 models[14];
    UniqueID32 particles1[3];
    UniqueID32 stateMachine;
    UniqueID32 particles2[6];
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<float> unknown8;
    UniqueID32 texture;
    Value<atUint32> unknown9;
    UniqueID32 particle;
    Value<atUint32> unknown10;
    Value<atUint32> unknown11;
    Value<atUint32> unknown12;
};
}
}

#endif
