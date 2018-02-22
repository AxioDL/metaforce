#ifndef _DNAMP1_WALLCRAWLERSWARM_HPP_
#define _DNAMP1_WALLCRAWLERSWARM_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct WallCrawlerSwarm : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> volume;
    Value<bool> active;
    ActorParameters actorParameters;
    Value<atUint32> unknown1;
    AnimationParameters animationParameters;
    Value<atUint32> unknown2;
    Value<atUint32> unknown3;
    UniqueID32 particle1;
    UniqueID32 particle2;
    Value<atUint32> unknown4; // always FF
    Value<atUint32> unknown5; // always FF
    DamageInfo damageInfo1;
    Value<float> unknown6;
    DamageInfo damageInfo2;
    Value<float> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;
    Value<float> unknown10;
    Value<atUint32> unknown11;
    Value<atUint32> unknown12;
    Value<float> unknown13;
    Value<float> unknown14;
    Value<float> unknown15;
    Value<float> unknown16;
    Value<float> unknown17;
    Value<float> unknown18;
    Value<float> unknown19;
    Value<float> unknown20;
    Value<atUint32> unknown21;
    Value<float> unkown22;
    Value<float> unkown23;
    Value<float> unkown24;
    HealthInfo healthInfo;
    DamageVulnerability damageVulnerabilty;
    Value<atUint32> soundID1; // verification needed
    Value<atUint32> soundID2; // verification needed

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (particle1)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle1);
            ent->name = name + "_part1";
        }
        if (particle2)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle2);
            ent->name = name + "_part2";
        }
        animationParameters.nameANCS(pakRouter, name + "_animp");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(particle1, pathsOut);
        g_curSpec->flattenDependencies(particle2, pathsOut);
        animationParameters.depANCS(pathsOut);
        actorParameters.depIDs(pathsOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }
};
}

#endif
