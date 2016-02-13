#ifndef _DNAMP1_MAGDOLITE_HPP_
#define _DNAMP1_MAGDOLITE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec
{
namespace DNAMP1
{
struct Magdolite : IScriptObject
{
    DECL_YAML
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<float> unknown1;
    Value<float> unknown2;
    DamageInfo damageInfo1;
    DamageInfo damageInfo2;
    DamageVulnerability damageVulnerabilty1;
    DamageVulnerability damageVulnerabilty2;
    UniqueID32 model;
    UniqueID32 skin;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    struct MagdoliteParameters : BigYAML
    {
        DECL_YAML
        Value<atUint32> propertyCount;
        Value<atUint32> unknown1;
        UniqueID32 particle;
        Value<atUint32> unknown2;
        Value<float> unknown3;
        Value<float> unknown4;
        Value<float> unknown5;
    } magdoliteParameters;
    Value<float> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        UniqueID32 cinf = patternedInfo.animationParameters.getCINF(pakRouter);
        actorParameters.addCMDLRigPairs(addTo, cinf);
        if (model && skin)
            addTo[model] = std::make_pair(skin, cinf);
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (model)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model);
            ent->name = name + "_emodel";
        }
        if (skin)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(skin);
            ent->name = name + "_eskin";
        }
        if (magdoliteParameters.particle)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(magdoliteParameters.particle);
            ent->name = name + "_part";
        }
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }
};
}
}

#endif
