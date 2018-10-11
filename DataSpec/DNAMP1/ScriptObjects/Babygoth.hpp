#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Babygoth : IScriptObject
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
    UniqueID32 wpsc1;
    DamageInfo damageInfo1;
    DamageInfo damageInfo2;
    UniqueID32 wpsc2;
    UniqueID32 particle1;
    DamageInfo damageInfo3;
    DamageVulnerability damageVulnerabilty1;
    DamageVulnerability damageVulnerabilty2;
    UniqueID32 cmdlShelless;
    UniqueID32 cskrShelless;
    Value<float> unknown3;
    Value<atUint32> unknown4;
    UniqueID32 particle2;
    UniqueID32 particle3;
    UniqueID32 particle4;
    UniqueID32 particle5;
    Value<atUint32> unknown5;
    Value<atUint32> unknown6;
    Value<atUint32> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;
    Value<float> unknown10;
    UniqueID32 texture;
    Value<atUint32> unknown11;
    UniqueID32 particle6;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const
    {
        UniqueID32 cinf = patternedInfo.animationParameters.getCINF(pakRouter);
        actorParameters.addCMDLRigPairs(pakRouter, charAssoc, patternedInfo.animationParameters);

        if (cmdlShelless && cskrShelless)
        {
            charAssoc.m_cmdlRigs[cmdlShelless] = std::make_pair(cskrShelless, cinf);
            charAssoc.m_cskrCinfToCharacter[cskrShelless] = std::make_pair(
                patternedInfo.animationParameters.animationCharacterSet, "ATTACH.SHELLESS.CSKR");
            charAssoc.addAttachmentRig(patternedInfo.animationParameters.animationCharacterSet,
                                       {}, cmdlShelless, "SHELLESS");
        }
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
        if (particle1)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle1);
            ent->name = name + "_part1";
        }
        if (cmdlShelless)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(cmdlShelless);
            ent->name = name + "_emodel";
        }
        if (cskrShelless)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(cskrShelless);
            ent->name = name + "_eskin";
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
        if (particle4)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle4);
            ent->name = name + "_part4";
        }
        if (particle5)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle5);
            ent->name = name + "_part5";
        }
        if (texture)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture);
            ent->name = name + "_tex";
        }
        if (particle6)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle6);
            ent->name = name + "_part6";
        }
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(wpsc1, pathsOut);
        g_curSpec->flattenDependencies(wpsc2, pathsOut);
        g_curSpec->flattenDependencies(particle1, pathsOut);
        g_curSpec->flattenDependencies(cmdlShelless, pathsOut);
        g_curSpec->flattenDependencies(cskrShelless, pathsOut);
        g_curSpec->flattenDependencies(particle2, pathsOut);
        g_curSpec->flattenDependencies(particle3, pathsOut);
        g_curSpec->flattenDependencies(particle4, pathsOut);
        g_curSpec->flattenDependencies(particle5, pathsOut);
        g_curSpec->flattenDependencies(texture, pathsOut);
        g_curSpec->flattenDependencies(particle6, pathsOut);
        patternedInfo.depIDs(pathsOut);
        actorParameters.depIDs(pathsOut, lazyOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }
};
}

