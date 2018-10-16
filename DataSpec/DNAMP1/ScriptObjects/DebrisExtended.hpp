#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct DebrisExtended : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<float> linConeAngle;
    Value<float> linMinMag;
    Value<float> linMaxMag;
    Value<float> angMinMag;
    Value<float> angMaxMag;
    Value<float> minDuration;
    Value<float> maxDuration;
    Value<float> colorInT;
    Value<float> colorOutT;
    DNAColor color;
    DNAColor endsColor;
    Value<float> scaleOutT;
    Value<atVec3f> endScale;
    Value<float> restitution;
    Value<float> downwardSpeed;
    Value<atVec3f> localOffset;
    UniqueID32 model;
    ActorParameters actorParameters;
    UniqueID32 particle1;
    Value<atVec3f> particle1Scale;
    Value<bool> particle1GlobalTranslation;
    Value<bool> deferDeleteTillParticle1Done;
    Value<atUint32> particle1Or;
    UniqueID32 particle2;
    Value<atVec3f> particle2Scale;
    Value<bool> particle2GlobalTranslation;
    Value<bool> deferDeleteTillParticle2Done;
    Value<atUint32> particle2Or;
    UniqueID32 particle3;
    Value<atVec3f> particle3Scale;
    Value<atUint32> particle3Or;
    Value<bool> solid;
    Value<bool> dieOnProjectile;
    Value<bool> noBounce;
    Value<bool> active;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (model)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model);
            ent->name = name + "_model";
        }
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
        if (particle3)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle3);
            ent->name = name + "_part3";
        }
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(model, pathsOut);
        g_curSpec->flattenDependencies(particle1, pathsOut);
        g_curSpec->flattenDependencies(particle2, pathsOut);
        g_curSpec->flattenDependencies(particle3, pathsOut);
        actorParameters.depIDs(pathsOut, lazyOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }
};
}

