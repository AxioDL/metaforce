#ifndef _DNAMP1_SEEDLING_HPP_
#define _DNAMP1_SEEDLING_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct Seedling : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    UniqueID32 unknown1;
    UniqueID32 unknown2;
    DamageInfo damageInfo1;
    DamageInfo damageInfo2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, patternedInfo.animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (unknown1)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(unknown1);
            ent->name = name + "_unk1";
        }
        if (unknown2)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(unknown2);
            ent->name = name + "_unk2";
        }
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }
};
}
}

#endif
