#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1
{
struct Magdolite : IScriptObject
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
    DamageInfo damageInfo1;
    DamageInfo damageInfo2;
    DamageVulnerability damageVulnerabilty1;
    DamageVulnerability damageVulnerabilty2;
    UniqueID32 cmdlHeadless;
    UniqueID32 cskrHeadless;
    Value<float> unknown3;
    Value<float> unknown4;
    Value<float> unknown5;
    Value<float> unknown6;
    struct MagdoliteParameters : BigDNA
    {
        AT_DECL_DNA
        Value<atUint32> propertyCount;
        Value<atUint32> unknown1;
        UniqueID32 particle;
        Value<atUint32> unknown2;
        Value<float> unknown3;
        Value<float> unknown4;
        Value<float> unknown5;
    } magdoliteParameters;
    Value<float> unknown7;
    Value<float> unknown8;
    Value<float> unknown9;

    void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const
    {
        UniqueID32 cinf = patternedInfo.animationParameters.getCINF(pakRouter);
        actorParameters.addCMDLRigPairs(pakRouter, charAssoc, patternedInfo.animationParameters);

        if (cmdlHeadless && cskrHeadless)
        {
            charAssoc.m_cmdlRigs[cmdlHeadless] = std::make_pair(cskrHeadless, cinf);
            charAssoc.m_cskrCinfToCharacter[cskrHeadless] = std::make_pair(
                patternedInfo.animationParameters.animationCharacterSet, "ATTACH.HEADLESS.CSKR");
            charAssoc.addAttachmentRig(patternedInfo.animationParameters.animationCharacterSet,
                                       {}, cmdlHeadless, "HEADLESS");
        }
    }

    void nameIDs(PAKRouter<PAKBridge>& pakRouter) const
    {
        if (cmdlHeadless)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(cmdlHeadless);
            ent->name = name + "_emodel";
        }
        if (cskrHeadless)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(cskrHeadless);
            ent->name = name + "_eskin";
        }
        if (magdoliteParameters.particle)
        {
            PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(magdoliteParameters.particle);
            ent->name = name + "_part";
        }
        patternedInfo.nameIDs(pakRouter, name + "_patterned");
        actorParameters.nameIDs(pakRouter, name + "_actp");
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                            std::vector<hecl::ProjectPath>& lazyOut) const
    {
        g_curSpec->flattenDependencies(cmdlHeadless, pathsOut);
        g_curSpec->flattenDependencies(cskrHeadless, pathsOut);
        g_curSpec->flattenDependencies(magdoliteParameters.particle, pathsOut);
        patternedInfo.depIDs(pathsOut);
        actorParameters.depIDs(pathsOut, lazyOut);
    }

    void gatherScans(std::vector<Scan>& scansOut) const
    {
        actorParameters.scanIDs(scansOut);
    }
};
}

