#ifndef _DNAMP1_FLAAHGRA_HPP_
#define _DNAMP1_FLAAHGRA_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Flaahgra : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
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
    DamageVulnerability damageVulnerabilty;
    UniqueID32 wpsc1;
    DamageInfo damageInfo1;
    UniqueID32 wpsc2;
    DamageInfo damageInfo2;
    UniqueID32 particle;
    DamageInfo damageInfo3;
    ActorParameters actorParameters2;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    AnimationParameters animationParameters;
    UniqueID32 dependencyGroup;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
                         std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters1.addCMDLRigPairs(addTo, patternedInfo.animationParameters.getCINF(pakRouter));
        actorParameters2.addCMDLRigPairs(addTo, animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (wpsc1)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc1);
            ent->name = name + "_wpsc1";
        }
        if (wpsc2)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc2);
            ent->name = name + "_wpsc2";
        }
        if (particle)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle);
            ent->name = name + "_part";
        }
        if (dependencyGroup)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(dependencyGroup);
            ent->name = name + "_dgrp";
        }
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters1.nameIDs(pakRouter, name + "_actp1");
        actorParameters2.nameIDs(pakRouter, name + "_actp2");
        animationParameters.nameANCS(pakRouter, name + "_animp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(wpsc1, pathsOut);
        g_curSpec->flattenDependencies(wpsc2, pathsOut);
        g_curSpec->flattenDependencies(particle, pathsOut);
        g_curSpec->flattenDependencies(dependencyGroup, pathsOut);
        patternedInfo.depIDs(pathsOut);
        actorParameters1.depIDs(pathsOut);
        actorParameters2.depIDs(pathsOut);
        animationParameters.depANCS(pathsOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters1.scanIDs(scansOut);
        actorParameters2.scanIDs(scansOut);
    }
};
}

#endif
