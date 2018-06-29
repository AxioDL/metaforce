#ifndef _DNAMP1_PLATFORM_HPP_
#define _DNAMP1_PLATFORM_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Platform : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<atVec3f> extent;
    Value<atVec3f> collisionCenter;
    UniqueID32 model;
    AnimationParameters animationParameters;
    ActorParameters actorParameters;
    Value<float> speed;
    Value<bool> active;
    UniqueID32 dcln;
    HealthInfo healthInfo;
    DamageVulnerability damageVulnerabilty;
    Value<bool> detectCollision;
    Value<float> xrayAlpha;
    Value<bool> rainSplashes;
    Value<atUint32> maxRainSplashes;
    Value<atUint32> rainGenRate;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (dcln)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(dcln);
            ent->name = name + "_dcln";
        }
        if (model)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model);
            ent->name = name + "_model";
        }
        animationParameters.nameANCS(pakRouter, name + "_animp");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(dcln, pathsOut);
        g_curSpec->flattenDependencies(model, pathsOut);
        animationParameters.depANCS(pathsOut);
        actorParameters.depIDs(pathsOut, lazyOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }

    zeus::CAABox getVISIAABB(hecl::blender::Token& btok) const;
};
}

#endif
