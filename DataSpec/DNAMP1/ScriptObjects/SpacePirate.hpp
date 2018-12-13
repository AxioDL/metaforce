#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct SpacePirate : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  PatternedInfo patternedInfo;
  ActorParameters actorParameters;
  Value<float> AggressionCheck;
  Value<float> CoverCheck;
  Value<float> SearchRadius;
  Value<float> FallBackCheck;
  Value<float> FallBackRadius;
  Value<float> HearingRadius;
  /*
   * 0x1: pendingAmbush
   * 0x2: ceilingAmbush
   * 0x4: nonAggressive
   * 0x8: melee
   * 0x10: noShuffleCloseCheck
   * 0x20: onlyAttackInRange
   * 0x40: unk
   * 0x80: noKnockbackImpulseReset
   * 0x200: noMeleeAttack
   * 0x400: breakAttack
   * 0x1000: seated
   * 0x2000: shadowPirate
   * 0x4000: alertBeforeCloak
   * 0x8000: noBreakDodge
   * 0x10000: floatingCorpse
   * 0x20000: ragdollNoAiCollision
   * 0x40000: trooper
   */
  Value<atUint32> flags;
  Value<bool> unknown8;
  UniqueID32 Projectile;
  DamageInfo ProjectileDamage;
  Value<atUint32> Sound_Projectile;
  DamageInfo BladeDamage;
  Value<float> KneelAttackChance;
  UniqueID32 KneelAttackShot;
  DamageInfo KneelAttackDamage;
  Value<float> DodgeCheck;
  Value<atUint32> Sound_Impact;
  Value<float> averageNextShotTime;
  Value<float> nextShotTimeVariation;
  Value<atUint32> Sound_Alert;
  Value<float> GunTrackDelay;
  Value<atUint32> firstBurstCount;
  Value<float> CloakOpacity;
  Value<float> MaxCloakOpacity;
  Value<float> dodgeDelayTimeMin;
  Value<float> dodgeDelayTimeMax;
  Value<atUint32> Sound_Hurled;
  Value<atUint32> Sound_Death;
  Value<float> unknown19;
  Value<float> AvoidDistance;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const {
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, patternedInfo.animationParameters);
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {
    if (Projectile) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(Projectile);
      ent->name = name + "_Projectile";
    }
    if (KneelAttackShot) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(KneelAttackShot);
      ent->name = name + "_KneelAttackShot";
    }
    patternedInfo.nameIDs(pakRouter, name + "_patterned");
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
    g_curSpec->flattenDependencies(Projectile, pathsOut);
    g_curSpec->flattenDependencies(KneelAttackShot, pathsOut);
    patternedInfo.depIDs(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const { actorParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
