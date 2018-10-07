#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct SpacePirate : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<atUint32> unknown7;
    Value<bool> unknown8;
    UniqueID32 wpsc1;
    DamageInfo damageInfo1;
    Value<atUint32> soundID1;
    DamageInfo damageInfo2;
    Value<float> unknown9;
    UniqueID32 wpsc2;
    DamageInfo damageInfo3;
    Value<float> unknown10;
    Value<atUint32> soundID2;
    Value<float> unknown11;
    Value<float> unknown12;
    Value<atUint32> soundID3;
    Value<float> unknown13;
    Value<atUint32> unknown14;
    Value<float> unknown15;
    Value<float> unknown16;
    Value<float> unknown17;
    Value<float> unknown18;
    Value<atUint32> soundID4;
    Value<atUint32> soundID5;
    Value<float> unknown19;
    Value<float> unknown20;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, patternedInfo.animationParameters.getCINF(pakRouter));
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
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(wpsc1, pathsOut);
        g_curSpec->flattenDependencies(wpsc2, pathsOut);
        patternedInfo.depIDs(pathsOut);
        actorParameters.depIDs(pathsOut, lazyOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }
};
}

