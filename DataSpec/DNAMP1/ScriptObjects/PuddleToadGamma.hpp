#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct PuddleToadGamma : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atUint32> unknown1;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<atVec3f> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<float> unknown8;
    DamageInfo damageInfo1;
    DamageInfo damageInfo2;
    UniqueID32 dcln;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const
    {
        actorParameters.addCMDLRigPairs(pakRouter, charAssoc, patternedInfo.animationParameters);
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (dcln)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(dcln);
            ent->name = name + "_dcln";
        }
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(dcln, pathsOut);
        patternedInfo.depIDs(pathsOut);
        actorParameters.depIDs(pathsOut, lazyOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }
};
}

