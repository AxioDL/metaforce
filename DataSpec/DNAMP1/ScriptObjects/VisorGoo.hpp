#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct VisorGoo : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> position;
    UniqueID32 particle;
    UniqueID32 electric;
    Value<float> minDist;
    Value<float> maxDist;
    Value<float> nearProb;
    Value<float> farProb;
    DNAColor color;
    Value<atUint32> sfx;
    Value<bool> skipAngleTest;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (particle)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle);
            ent->name = name + "_part";
        }
        if (electric)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(electric);
            ent->name = name + "_elsc";
        }
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(particle, pathsOut);
        g_curSpec->flattenDependencies(electric, pathsOut);
    }
};
}

