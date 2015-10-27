#ifndef _DNAMP1_THARDUSROCKPROJECTILE_HPP_
#define _DNAMP1_THARDUSROCKPROJECTILE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace Retro
{
namespace DNAMP1
{
struct ThardusRockProjectile : IScriptObject
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
    Value<float> unknown3;
    UniqueID32 model;
    UniqueID32 stateMachine;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, patternedInfo.animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (model)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model);
            ent->name = name + "_model";
        }
        if (stateMachine)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(stateMachine);
            ent->name = name + "_fsm";
        }
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }
};
}
}

#endif
