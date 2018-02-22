#ifndef _DNAMP1_DRONE_HPP_
#define _DNAMP1_DRONE_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Drone : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atUint32> unknown1;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    Value<float> unknown2;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    DamageInfo damageInfo1;
    Value<atUint32> unknown3;
    DamageInfo damageInfo2;
    Value<atUint32> unknown4;
    Value<atUint32> unknown5;
    Value<atUint32> unknown6;
    FlareDefinition flareDefinition1;
    FlareDefinition flareDefinition2;
    FlareDefinition flareDefinition3;
    FlareDefinition flareDefinition4;
    FlareDefinition flareDefinition5;
    Value<float> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;
    Value<float> unknown10;
    Value<float> unknown11;
    Value<float> unknown12;
    Value<float> unknown13;
    Value<float> unknown14;
    Value<float> unknown15;
    Value<float> unknown16;
    Value<float> unknown17;
    Value<float> unknown18;
    Value<float> unknown19;
    Value<float> unknown20;
    Value<float> unknown21;
    Value<float> unknown22;
    Value<float> unknown23;
    Value<float> unknown24;
    Value<float> unknown25;
    UniqueID32 crsc;
    Value<float> unknon26;
    Value<float> unknon27;
    Value<float> unknon28;
    Value<float> unknon29;
    Value<atUint32> sound; // verification needed
    Value<bool> unknown30;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, patternedInfo.animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (crsc)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(crsc);
            ent->name = name + "_crsc";
        }
        flareDefinition1.nameIDs(pakRouter, name + "_flare1");
        flareDefinition2.nameIDs(pakRouter, name + "_flare2");
        flareDefinition3.nameIDs(pakRouter, name + "_flare3");
        flareDefinition4.nameIDs(pakRouter, name + "_flare4");
        flareDefinition5.nameIDs(pakRouter, name + "_flare5");
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        g_curSpec->flattenDependencies(crsc, pathsOut);
        flareDefinition1.depIDs(pathsOut);
        flareDefinition2.depIDs(pathsOut);
        flareDefinition3.depIDs(pathsOut);
        flareDefinition4.depIDs(pathsOut);
        flareDefinition5.depIDs(pathsOut);
        patternedInfo.depIDs(pathsOut);
        actorParameters.depIDs(pathsOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }
};
}

#endif
