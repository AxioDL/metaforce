#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct OmegaPirate : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  PatternedInfo patternedInfo;
  ActorParameters actorParameters1;
  Value<float> unknown1;
  Value<float> unknown2;
  Value<float> unknown3;
  Value<float> unknown4;
  Value<float> unknown5;
  Value<float> unknown6;
  Value<float> unknown7;
  Value<float> unknown8;
  UniqueID32 particle1;
  Value<atUint32> soundID1;
  ActorParameters actorParameters2;
  AnimationParameters animationParameters;
  UniqueID32 particle2;
  Value<atUint32> soundID2;
  UniqueID32 model1;
  DamageInfo damageInfo1;
  Value<float> unknown9;
  UniqueID32 particle3;
  UniqueID32 particle4;
  UniqueID32 particle5;
  UniqueID32 particle6;
  Value<float> unknown10;
  Value<float> unknown11;
  Value<float> unknown12;
  Value<float> unknown13;
  Value<float> unknown14;
  Value<float> unknown15;
  Value<atUint32> unknown16;
  Value<atUint32> soundID3;
  Value<atUint32> soundID4;
  UniqueID32 particle7;
  DamageInfo damageInfo2;
  UniqueID32 elsc;
  Value<atUint32> soundID5;
  Value<bool> unknown17;
  Value<bool> unknown18;
  UniqueID32 cmdlPhazonVeins;
  UniqueID32 cskrPhazonVeins;
  UniqueID32 cinfPhazonVeins;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const {
    actorParameters1.addCMDLRigPairs(pakRouter, charAssoc, patternedInfo.animationParameters);
    actorParameters2.addCMDLRigPairs(pakRouter, charAssoc, patternedInfo.animationParameters);
    if (cmdlPhazonVeins.isValid() && cskrPhazonVeins.isValid() && cinfPhazonVeins.isValid()) {
      charAssoc.m_cmdlRigs[cmdlPhazonVeins] = std::make_pair(cskrPhazonVeins, cinfPhazonVeins);
      charAssoc.m_cskrCinfToCharacter[cskrPhazonVeins] =
          std::make_pair(patternedInfo.animationParameters.animationCharacterSet, "ATTACH.VEINS.CSKR");
      charAssoc.m_cskrCinfToCharacter[cinfPhazonVeins] =
          std::make_pair(patternedInfo.animationParameters.animationCharacterSet,
                         fmt::format(fmt("CINF_{}.CINF"), cinfPhazonVeins));
      charAssoc.addAttachmentRig(patternedInfo.animationParameters.animationCharacterSet, cinfPhazonVeins,
                                 cmdlPhazonVeins, "VEINS");
    }
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {
    if (particle1.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle1);
      ent->name = name + "_part1";
    }
    if (particle2.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle2);
      ent->name = name + "_part2";
    }
    if (particle3.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle3);
      ent->name = name + "_part3";
    }
    if (particle4.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle4);
      ent->name = name + "_part4";
    }
    if (particle5.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle5);
      ent->name = name + "_part5";
    }
    if (particle6.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle6);
      ent->name = name + "_part6";
    }
    if (particle7.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle7);
      ent->name = name + "_part7";
    }
    if (elsc.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(elsc);
      ent->name = name + "_elsc";
    }
    if (model1.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model1);
      ent->name = name + "_model1";
    }
    if (cmdlPhazonVeins.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(cmdlPhazonVeins);
      ent->name = name + "_model2";
    }
    if (cskrPhazonVeins.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(cskrPhazonVeins);
      ent->name = name + "_skin";
    }
    if (cinfPhazonVeins.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(cinfPhazonVeins);
      ent->name = name + "_rig";
    }
    patternedInfo.nameIDs(pakRouter, name + "_patterned");
    actorParameters1.nameIDs(pakRouter, name + "_actp1");
    actorParameters2.nameIDs(pakRouter, name + "_actp2");
    animationParameters.nameANCS(pakRouter, name + "_animp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
    g_curSpec->flattenDependencies(particle1, pathsOut);
    g_curSpec->flattenDependencies(particle2, pathsOut);
    g_curSpec->flattenDependencies(particle3, pathsOut);
    g_curSpec->flattenDependencies(particle4, pathsOut);
    g_curSpec->flattenDependencies(particle5, pathsOut);
    g_curSpec->flattenDependencies(particle6, pathsOut);
    g_curSpec->flattenDependencies(particle7, pathsOut);
    g_curSpec->flattenDependencies(elsc, pathsOut);
    g_curSpec->flattenDependencies(model1, pathsOut);
    g_curSpec->flattenDependencies(cmdlPhazonVeins, pathsOut);
    g_curSpec->flattenDependencies(cskrPhazonVeins, pathsOut);
    g_curSpec->flattenDependencies(cinfPhazonVeins, pathsOut);
    patternedInfo.depIDs(pathsOut);
    actorParameters1.depIDs(pathsOut, lazyOut);
    actorParameters2.depIDs(pathsOut, lazyOut);
    animationParameters.depANCS(pathsOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const {
    actorParameters1.scanIDs(scansOut);
    actorParameters2.scanIDs(scansOut);
  }
};
} // namespace DataSpec::DNAMP1
