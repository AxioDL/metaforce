#ifndef _DNAMP1_OMEGAPIRATE_HPP_
#define _DNAMP1_OMEGAPIRATE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct OmegaPirate : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters1;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<float> unknown8;
    UniqueID32 particle1;
    Value<atUint32> soundID1;
    ActorParameters actorParameters2;
    AnimationParameters animationParameters;
    UniqueID32 particle2;
    Value<atUint32> soundID2;
    UniqueID32 model1;
    DamageInfo damageInfo1;
    Value<float> unknown9;
    UniqueID32 particle3;
    UniqueID32 particle4;
    UniqueID32 particle5;
    UniqueID32 particle6;
    Value<float> unknown10;
    Value<float> unknown11;
    Value<float> unknown12;
    Value<float> unknown13;
    Value<float> unknown14;
    Value<float> unknown15;
    Value<atUint32> unknown16;
    Value<atUint32> soundID3;
    Value<atUint32> soundID4;
    UniqueID32 particle7;
    DamageInfo damageInfo2;
    UniqueID32 elsc;
    Value<atUint32> soundID5;
    Value<bool> unknown17;
    Value<bool> unknown18;
    UniqueID32 model2;
    UniqueID32 skin;
    UniqueID32 rig;
};
}
}

#endif
