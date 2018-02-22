#ifndef _DNAMP1_THARDUS_HPP_
#define _DNAMP1_THARDUS_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Thardus : IScriptObject
{
    AT_DECL_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    Value<bool> unknown1;
    Value<bool> unknown2;
    UniqueID32 models[14];
    UniqueID32 particles1[3];
    UniqueID32 stateMachine;
    UniqueID32 particles2[6];
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    Value<float> unknown7;
    Value<float> unknown8;
    UniqueID32 texture;
    Value<atUint32> unknown9;
    UniqueID32 particle;
    Value<atUint32> unknown10;
    Value<atUint32> unknown11;
    Value<atUint32> unknown12;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, patternedInfo.animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (models[0])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[0]);
            ent->name = name + "_model1";
        }
        if (models[1])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[1]);
            ent->name = name + "_model2";
        }
        if (models[2])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[2]);
            ent->name = name + "_model3";
        }
        if (models[3])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[3]);
            ent->name = name + "_model4";
        }
        if (models[4])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[4]);
            ent->name = name + "_model5";
        }
        if (models[5])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[5]);
            ent->name = name + "_model6";
        }
        if (models[6])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[6]);
            ent->name = name + "_model7";
        }
        if (models[7])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[7]);
            ent->name = name + "_model8";
        }
        if (models[8])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[8]);
            ent->name = name + "_model9";
        }
        if (models[9])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[9]);
            ent->name = name + "_model10";
        }
        if (models[10])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[10]);
            ent->name = name + "_model11";
        }
        if (models[11])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[11]);
            ent->name = name + "_model12";
        }
        if (models[12])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[12]);
            ent->name = name + "_model13";
        }
        if (models[13])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(models[13]);
            ent->name = name + "_model14";
        }
        if (particles1[0])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particles1[0]);
            ent->name = name + "_part1";
        }
        if (particles1[1])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particles1[1]);
            ent->name = name + "_part2";
        }
        if (particles1[2])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particles1[2]);
            ent->name = name + "_part3";
        }
        if (stateMachine)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(stateMachine);
            ent->name = name + "_fsm";
        }
        if (particles2[0])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particles2[0]);
            ent->name = name + "_part4";
        }
        if (particles2[1])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particles2[1]);
            ent->name = name + "_part5";
        }
        if (particles2[2])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particles2[2]);
            ent->name = name + "_part6";
        }
        if (particles2[3])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particles2[3]);
            ent->name = name + "_part7";
        }
        if (particles2[4])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particles2[4]);
            ent->name = name + "_part8";
        }
        if (particles2[5])
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particles2[5]);
            ent->name = name + "_part9";
        }
        if (particle)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle);
            ent->name = name + "_part10";
        }
        if (texture)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture);
            ent->name = name + "_texture";
        }
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const
    {
        for (int i=0 ; i<14 ; ++i)
            g_curSpec->flattenDependencies(models[i], pathsOut);
        for (int i=0 ; i<3 ; ++i)
            g_curSpec->flattenDependencies(particles1[i], pathsOut);
        g_curSpec->flattenDependencies(stateMachine, pathsOut);
        for (int i=0 ; i<6 ; ++i)
            g_curSpec->flattenDependencies(particles2[i], pathsOut);
        g_curSpec->flattenDependencies(particle, pathsOut);
        g_curSpec->flattenDependencies(texture, pathsOut);
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
