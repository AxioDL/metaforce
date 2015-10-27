#ifndef _DNAMP1_PLAYERACTOR_HPP_
#define _DNAMP1_PLAYERACTOR_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct PlayerActor : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<atVec3f> unknown1;
    Value<atVec3f> scanOffset;
    Value<float> unknown2;
    Value<float> unknown3;
    HealthInfo healthInfo;
    DamageVulnerability damageVulnerability;
    UniqueID32 model;
    AnimationParameters animationParameters;
    ActorParameters actorParameters;
    Value<bool> unknown4;
    Value<bool> unknown5;
    Value<bool> unknown6;
    Value<bool> unknown7;
    struct PlayerParameters : BigYAML
    {
        DECL_YAML
        Value<atUint32> propertyCount;
        Vector<bool, DNA_COUNT(propertyCount)> bools;
    } playerParameters;
    Value<atUint32> unknown8;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (model)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model);
            ent->name = name + "_model";
        }
        animationParameters.nameANCS(pakRouter, name + "_animp");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }
};
}
}

#endif
