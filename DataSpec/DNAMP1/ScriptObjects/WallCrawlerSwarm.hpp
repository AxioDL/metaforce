#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct WallCrawlerSwarm : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> volume;
  Value<bool> active;
  ActorParameters actorParameters;
  Value<atUint32> flavor;
  AnimationParameters animationParameters;
  Value<atUint32> launchAnim;
  Value<atUint32> attractAnim;
  UniqueID32 part1;
  UniqueID32 part2;
  UniqueID32 part3;
  UniqueID32 part4;
  DamageInfo crabDamage;
  Value<float> crabDamageCooldown;
  DamageInfo scarabExplodeDamage;
  Value<float> boidRadius;
  Value<float> touchRadius;
  Value<float> playerTouchRadius;
  Value<float> animPlaybackSpeed;
  Value<atUint32> numBoids;
  Value<atUint32> maxCreatedBoids;
  Value<float> separationRadius;
  Value<float> cohesionMagnitude;
  Value<float> alignmentWeight;
  Value<float> separationMagnitude;
  Value<float> moveToWaypointWeight;
  Value<float> attractionMagnitude;
  Value<float> attractionRadius;
  Value<float> boidGenRate;
  Value<atUint32> maxLaunches;
  Value<float> scarabBoxMargin;
  Value<float> scarabScatterXYVelocity;
  Value<float> scarabTimeToExplode;
  HealthInfo healthInfo;
  DamageVulnerability damageVulnerabilty;
  Value<atUint32> launchSfx;
  Value<atUint32> scatterSfx;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const {
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, animationParameters);
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {
    if (part1) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(part1);
      ent->name = name + "_part1";
    }
    if (part2) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(part2);
      ent->name = name + "_part2";
    }
    if (part3) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(part3);
      ent->name = name + "_part3";
    }
    if (part4) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(part4);
      ent->name = name + "_part4";
    }
    animationParameters.nameANCS(pakRouter, name + "_animp");
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
    g_curSpec->flattenDependencies(part1, pathsOut);
    g_curSpec->flattenDependencies(part2, pathsOut);
    g_curSpec->flattenDependencies(part3, pathsOut);
    g_curSpec->flattenDependencies(part4, pathsOut);
    animationParameters.depANCS(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const { actorParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
