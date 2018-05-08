#ifndef _DNAMP1_RIDLEY_HPP_
#define _DNAMP1_RIDLEY_HPP_

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Ridley : IScriptObject
{
    AT_DECL_EXPLICIT_DNA_YAML
    AT_DECL_DNAV
    String<-1> name;
    Value<atVec3f> location;
    Value<atVec3f> orientation;
    Value<atVec3f> scale;
    PatternedInfo patternedInfo;
    ActorParameters actorParameters;
    UniqueID32 model1;
    UniqueID32 model2;
    UniqueID32 model3;
    UniqueID32 model4;
    UniqueID32 model5;
    UniqueID32 model6;
    UniqueID32 model7;
    UniqueID32 model8;
    UniqueID32 model9;
    UniqueID32 model10;
    UniqueID32 model11;
    UniqueID32 model12;
    UniqueID32 particle;
    Value<float> unknown1;
    Value<float> unknown2;
    Value<float> unknown3;
    Value<float> unknown4;
    UniqueID32 wpsc1;
    DamageInfo damageInfo1;
    struct RidleyStruct1 : BigDNA
    {
        AT_DECL_DNA_YAML
        Value<atUint32> unknown1;
        Value<atUint32> unknown2;
        UniqueID32 particle1;
        UniqueID32 particle2;
        UniqueID32 texture1;
        UniqueID32 texture2;
        Value<float> unknown3;
        Value<float> unknown4;
        Value<float> unknown5;
        Value<float> unknown6;
        Value<float> unknown7;
        Value<float> unknown8;
        Value<float> unknown9;
        Value<float> unknown10;
        Value<float> unknown11;
        Value<atVec4f> unknown12; //CColor
        Value<atVec4f> unknown13; //CColor

        void nameIDs(PAKRouter<PAKBridge>& pakRouter, const std::string& name) const
        {
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
            if (texture1)
            {
                PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture1);
                ent->name = name + "_tex1";
            }
            if (texture2)
            {
                PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture2);
                ent->name = name + "_tex2";
            }
        }

        void depIDs(std::vector<hecl::ProjectPath>& pathsOut) const
        {
            g_curSpec->flattenDependencies(particle1, pathsOut);
            g_curSpec->flattenDependencies(particle2, pathsOut);
            g_curSpec->flattenDependencies(texture1, pathsOut);
            g_curSpec->flattenDependencies(texture2, pathsOut);
        }
    } ridleyStruct1;

    Value<atUint32> soundID1;
    UniqueID32 wpsc2;
    UniqueID32 wpsc3;
    DamageInfo damageInfo2;

    struct RidleyStruct2 : BigDNA
    {
        AT_DECL_DNA_YAML
        Value<atUint32> unknown1;
        Value<float> unknown2;
        Value<float> unknown3;
        Value<float> unknown4;
        Value<float> unknown5;
        Value<float> unknown6;
        Value<float> unknown7;
        Value<float> unknown8;
        Value<bool> unknown9;
    } ridleyStruct2_1;

    UniqueID32 wpsc4;
    DamageInfo damageInfo3;
    RidleyStruct2 ridleyStruct2_2;
    Value<atUint32> soundID2;
    DamageInfo damageInfo4;
    RidleyStruct2 ridleyStruct2_3;
    Value<float> unknown5;
    Value<float> unknown6;
    DamageInfo damageInfo5;
    Value<float> unknown7;
    DamageInfo damageInfo6;
    Value<float> unknown8;
    DamageInfo damageInfo7;
    Value<float> unknown9;
    UniqueID32 elsc;
    Value<float> unknown10;
    Value<atUint32> soundID3;
    DamageInfo damageInfo8;

    /* Trilogy addition */
    DamageInfo damageInfo9;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter,
            std::unordered_map<UniqueID32, std::pair<UniqueID32, UniqueID32>>& addTo) const
    {
        actorParameters.addCMDLRigPairs(addTo, patternedInfo.animationParameters.getCINF(pakRouter));
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (particle)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle);
            ent->name = name + "_part";
        }
        if (model1)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model1);
            ent->name = name + "_model1";
        }
        if (model2)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model2);
            ent->name = name + "_model2";
        }
        if (model3)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model3);
            ent->name = name + "_model3";
        }
        if (model4)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model4);
            ent->name = name + "_model4";
        }
        if (model5)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model5);
            ent->name = name + "_model5";
        }
        if (model6)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model6);
            ent->name = name + "_model6";
        }
        if (model7)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model7);
            ent->name = name + "_model7";
        }
        if (model8)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model8);
            ent->name = name + "_model8";
        }
        if (model9)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model9);
            ent->name = name + "_model9";
        }
        if (model10)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model10);
            ent->name = name + "_model10";
        }
        if (model11)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model11);
            ent->name = name + "_model11";
        }
        if (model12)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model12);
            ent->name = name + "_model12";
        }
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
        if (wpsc3)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc3);
            ent->name = name + "_wpsc3";
        }
        if (wpsc4)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc4);
            ent->name = name + "_wpsc4";
        }
        if (elsc)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(elsc);
            ent->name = name + "_elsc";
        }
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
        ridleyStruct1.nameIDs(pakRouter, name + "_ridley1");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(particle, pathsOut);
        g_curSpec->flattenDependencies(model1, pathsOut);
        g_curSpec->flattenDependencies(model2, pathsOut);
        g_curSpec->flattenDependencies(model3, pathsOut);
        g_curSpec->flattenDependencies(model4, pathsOut);
        g_curSpec->flattenDependencies(model5, pathsOut);
        g_curSpec->flattenDependencies(model6, pathsOut);
        g_curSpec->flattenDependencies(model7, pathsOut);
        g_curSpec->flattenDependencies(model8, pathsOut);
        g_curSpec->flattenDependencies(model9, pathsOut);
        g_curSpec->flattenDependencies(model10, pathsOut);
        g_curSpec->flattenDependencies(model11, pathsOut);
        g_curSpec->flattenDependencies(model12, pathsOut);
        g_curSpec->flattenDependencies(wpsc1, pathsOut);
        g_curSpec->flattenDependencies(wpsc2, pathsOut);
        g_curSpec->flattenDependencies(wpsc3, pathsOut);
        g_curSpec->flattenDependencies(wpsc4, pathsOut);
        g_curSpec->flattenDependencies(elsc, pathsOut);
        patternedInfo.depIDs(pathsOut);
        actorParameters.depIDs(pathsOut, lazyOut);
        ridleyStruct1.depIDs(pathsOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }
};
}

#endif
