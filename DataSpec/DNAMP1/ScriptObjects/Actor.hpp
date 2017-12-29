#ifndef _DNAMP1_ACTOR_HPP_
#define _DNAMP1_ACTOR_HPP_

#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Actor : IScriptObject
{
    DECL_YAML
    String<-1> name SO_NAME_SPECPROP();
    Value<atVec3f> location SO_LOCATION_SPECPROP();
    Value<atVec3f> orientation SO_ORIENTATION_SPECPROP();
    Value<atVec3f> scale SO_SCALE_SPECPROP();
    Value<atVec3f> collisionExtent SO_COLLISION_EXTENT_SPECPROP();
    Value<atVec3f> collisionOffset SO_COLLISION_OFFSET_SPECPROP();
    Value<float>   unknown2;
    Value<float>   unknown3;
    HealthInfo     healthInfo;
    DamageVulnerability damageVulnerability;
    UniqueID32     model;
    AnimationParameters animationParameters;
    ActorParameters actorParameters;
    Value<bool>     unkown4;
    Value<bool>     unkown5;
    Value<bool>     unkown6;
    Value<bool>     unkown7;
    Value<bool>     unkown8;
    Value<atUint32> unknown9;
    Value<float>    unknown10;
    Value<bool>     unknown11;
    Value<bool>     unknown12;
    Value<bool>     unknown13;
    Value<bool>     unknown14;

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

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(model, pathsOut);
        animationParameters.depANCS(pathsOut);
        actorParameters.depIDs(pathsOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }

    zeus::CAABox getVISIAABB(hecl::blender::Token& btok) const;
};
}

#endif
