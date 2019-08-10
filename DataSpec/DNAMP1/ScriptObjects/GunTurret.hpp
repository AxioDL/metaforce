#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct GunTurret : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atUint32> unknown1;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  Value<atVec3f> collisionExtent;
  Value<atVec3f> collisionOffset;
  AnimationParameters animationParameters;
  ActorParameters actorParameters;
  HealthInfo healthInfo;
  DamageVulnerability damageVulnerabilty;
  Value<float> intoDeactivateDelay;
  Value<float> reloadTime;
  Value<float> reloadTimeVariance;
  Value<float> panStartTime;
  Value<float> panHoldTime;
  Value<float> totalPanSearchTime;
  Value<float> leftMaxAngle;
  Value<float> rightMaxAngle;
  Value<float> downMaxAngle;
  Value<float> turnSpeed;
  Value<float> detectionRange;
  Value<float> detectionZRange;
  Value<float> freezeDuration;
  Value<float> freezeVariance;
  Value<bool> freezeTimeout;
  UniqueID32 projectileRes;
  DamageInfo projectileDamage;
  UniqueID32 idleLightRes;
  UniqueID32 deactivateLightRes;
  UniqueID32 targettingLightRes;
  UniqueID32 frozenEffectRes;
  UniqueID32 chargingEffectRes;
  UniqueID32 panningEffectRes;
  UniqueID32 visorEffectRes;
  Value<atUint32> trackingSoundId;
  Value<atUint32> lockOnSoundId;
  Value<atUint32> unfreezeSoundId;
  Value<atUint32> stopClankSoundId;
  Value<atUint32> chargingSoundId;
  Value<atUint32> visorSoundId;
  UniqueID32 extensionModelResId;
  Value<float> extensionDropDownDist;
  Value<atUint32> numInitialShots;
  Value<atUint32> initialShotTableIndex;
  Value<atUint32> numSubsequentShots;
  Value<float> frenzyDuration;
  Value<bool> scriptedStartOnly;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const override {
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, animationParameters);
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    if (projectileRes.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(projectileRes);
      ent->name = name + "_projectileRes";
    }
    if (idleLightRes.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(idleLightRes);
      ent->name = name + "_idleLightRes";
    }
    if (deactivateLightRes.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(deactivateLightRes);
      ent->name = name + "_deactivateLightRes";
    }
    if (targettingLightRes.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(targettingLightRes);
      ent->name = name + "_targettingLightRes";
    }
    if (frozenEffectRes.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(frozenEffectRes);
      ent->name = name + "_frozenEffectRes";
    }
    if (chargingEffectRes.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(chargingEffectRes);
      ent->name = name + "_chargingEffectRes";
    }
    if (panningEffectRes.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(panningEffectRes);
      ent->name = name + "_panningEffectRes";
    }
    if (visorEffectRes.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(visorEffectRes);
      ent->name = name + "_visorEffectRes";
    }
    if (extensionModelResId.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(extensionModelResId);
      ent->name = name + "_extensionModelResId";
    }
    animationParameters.nameANCS(pakRouter, name + "_animp");
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    g_curSpec->flattenDependencies(projectileRes, pathsOut);
    g_curSpec->flattenDependencies(idleLightRes, pathsOut);
    g_curSpec->flattenDependencies(deactivateLightRes, pathsOut);
    g_curSpec->flattenDependencies(targettingLightRes, pathsOut);
    g_curSpec->flattenDependencies(frozenEffectRes, pathsOut);
    g_curSpec->flattenDependencies(chargingEffectRes, pathsOut);
    g_curSpec->flattenDependencies(panningEffectRes, pathsOut);
    g_curSpec->flattenDependencies(visorEffectRes, pathsOut);
    g_curSpec->flattenDependencies(extensionModelResId, pathsOut);
    animationParameters.depANCS(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const override { actorParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
