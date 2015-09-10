#ifndef _DNAMP1_PICKUP_HPP_
#define _DNAMP1_PICKUP_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct Pickup : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<atVec3f> hitboxVolume;
    Value<atVec3f> scanPosition;
    Value<atUint32> pickupType;
    Value<atUint32> capacity;
    Value<atUint32> amount;
    Value<float> dropRate;
    Value<float> lifetime;
    Value<float> spawnDelay;
    UniqueID32 model;
    AnimationParameters animationParameters;
    ActorParameters actorParameters;
    Value<bool> active;
    Value<float> unknown1;
    UniqueID32 particle;
};
}
}

#endif
