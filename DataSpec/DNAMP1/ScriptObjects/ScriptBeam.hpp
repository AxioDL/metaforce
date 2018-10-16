#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct ScriptBeam : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<bool> unknown1;
    UniqueID32 wpsc;
    BeamInfo beamInfo;
    DamageInfo damageInfo;

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (wpsc)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc);
            ent->name = name + "_wpsc";
        }
        beamInfo.nameIDs(pakRouter, name + "_beamInfo");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(wpsc, pathsOut);
        beamInfo.depIDs(pathsOut);
    }
};
}

