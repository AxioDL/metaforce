#ifndef _DNAMP1_ACTORCONTRAPTION_HPP_
#define _DNAMP1_ACTORCONTRAPTION_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct ActorContraption : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1>          name;
    Value<atVec3f>      location;
    Value<atVec3f>      orientation;
    Value<atVec3f>      scale;
    Value<atVec3f>      unknown1;
    Value<atVec3f>      scanOffset;
    Value<float>        unknown2;
    Value<float>        unknown3;
    HealthInfo          healthInfo;
    DamageVulnerability damageVulnerability;
    AnimationParameters animationParameters;
    ActorParameters     actorParameters;
    UniqueID32          particle;
    DamageInfo          damageInfo;
    Value<bool>         active; // needs verification

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (particle)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle);
            ent->name = name + "_part";
        }
        animationParameters.nameANCS(pakRouter, name + "_animp");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(particle, pathsOut);
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
