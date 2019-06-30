#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Babygoth : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  PatternedInfo patternedInfo;
  ActorParameters actorParameters;
  Value<float> fireballAttackTime;
  Value<float> fireballAttackTimeVariance;
  UniqueID32 fireballWeapon;
  DamageInfo fireballDamage;
  DamageInfo attackContactDamage;
  UniqueID32 fireBreathWeapon;
  UniqueID32 fireBreathRes;
  DamageInfo fireBreathDamage;
  DamageVulnerability mouthVulnerabilities;
  DamageVulnerability shellVulnerabilities;
  UniqueID32 noShellModel;
  UniqueID32 noShellSkin;
  Value<float> shellHitPoints;
  Value<atUint32> shellCrackSfx;
  UniqueID32 intermediateCrackParticle;
  UniqueID32 crackOneParticle;
  UniqueID32 crackTwoParticle;
  UniqueID32 destroyShellParticle;
  Value<atUint32> crackOneSfx;
  Value<atUint32> crackTwoSfx;
  Value<atUint32> destroyShellSfx;
  Value<float> timeUntilAttack;
  Value<float> attackCooldownTime;
  Value<float> interestTime;
  UniqueID32 flamePlayerSteamTxtr;
  Value<atUint32> flamePlayerHitSfx;
  UniqueID32 flamePlayerIceTxtr;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const {
    UniqueID32 cinf = patternedInfo.animationParameters.getCINF(pakRouter);
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, patternedInfo.animationParameters);

    if (noShellModel && noShellSkin) {
      charAssoc.m_cmdlRigs[noShellModel] = std::make_pair(noShellSkin, cinf);
      charAssoc.m_cskrCinfToCharacter[noShellSkin] =
          std::make_pair(patternedInfo.animationParameters.animationCharacterSet, "ATTACH.SHELLESS.CSKR");
      charAssoc.addAttachmentRig(patternedInfo.animationParameters.animationCharacterSet, {}, noShellModel, "SHELLESS");
    }
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {
    if (fireballWeapon) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(fireballWeapon);
      ent->name = name + "_wpsc1";
    }
    if (fireBreathWeapon) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(fireBreathWeapon);
      ent->name = name + "_wpsc2";
    }
    if (fireBreathRes) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(fireBreathRes);
      ent->name = name + "_part1";
    }
    if (noShellModel) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(noShellModel);
      ent->name = name + "_emodel";
    }
    if (noShellSkin) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(noShellSkin);
      ent->name = name + "_eskin";
    }
    if (intermediateCrackParticle) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(intermediateCrackParticle);
      ent->name = name + "_part2";
    }
    if (crackOneParticle) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(crackOneParticle);
      ent->name = name + "_part3";
    }
    if (crackTwoParticle) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(crackTwoParticle);
      ent->name = name + "_part4";
    }
    if (destroyShellParticle) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(destroyShellParticle);
      ent->name = name + "_part5";
    }
    if (flamePlayerSteamTxtr) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(flamePlayerSteamTxtr);
      ent->name = name + "_tex";
    }
    if (flamePlayerIceTxtr) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(flamePlayerIceTxtr);
      ent->name = name + "_part6";
    }
    patternedInfo.nameIDs(pakRouter, name + "_patterned");
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
    g_curSpec->flattenDependencies(fireballWeapon, pathsOut);
    g_curSpec->flattenDependencies(fireBreathWeapon, pathsOut);
    g_curSpec->flattenDependencies(fireBreathRes, pathsOut);
    g_curSpec->flattenDependencies(noShellModel, pathsOut);
    g_curSpec->flattenDependencies(noShellSkin, pathsOut);
    g_curSpec->flattenDependencies(intermediateCrackParticle, pathsOut);
    g_curSpec->flattenDependencies(crackOneParticle, pathsOut);
    g_curSpec->flattenDependencies(crackTwoParticle, pathsOut);
    g_curSpec->flattenDependencies(destroyShellParticle, pathsOut);
    g_curSpec->flattenDependencies(flamePlayerSteamTxtr, pathsOut);
    g_curSpec->flattenDependencies(flamePlayerIceTxtr, pathsOut);
    patternedInfo.depIDs(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const { actorParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
