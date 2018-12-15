#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "../DNAMP1.hpp"
#include "../SAVW.hpp"
#include "specter/genie.hpp"

namespace DataSpec::DNAMP1 {

enum class EPickupType : atUint32 {
  PowerBeam = 0,
  IceBeam = 1,
  WaveBeam = 2,
  PlasmaBeam = 3,
  Missile = 4,
  ScanVisor = 5,
  MorphBallBomb = 6,
  PowerBomb = 7,
  Flamethrower = 8,
  ThermalVisor = 9,
  ChargeBeam = 10,
  SuperMissile = 11,
  GrappleBeam = 12,
  XRayVisor = 13,
  IceSpreader = 14,
  SpaceJump = 15,
  MorphBall = 16,
  CombatVisor = 17,
  BoostBall = 18,
  SpiderBall = 19,
  PowerSuit = 20,
  GravitySuit = 21,
  VariaSuit = 22,
  PhazonSuit = 23,
  EnergyTank = 24,
  UnknownItem1 = 25,
  HealthRefill = 26,
  UnknownItem2 = 27,
  WaveBuster = 28,
  Truth = 29,
  Strength = 30,
  Elder = 31,
  Wild = 32,
  LifeGiver = 33,
  Warrior = 34,
  Chozo = 35,
  Nature = 36,
  Sun = 37,
  World = 38,
  Spirit = 39,
  Newborn = 40
} SPECTER_ENUM("Pickup Type", "", EPickupType);

enum class ESpecialFunctionType : atUint32 {
  What,
  PlayerFollowLocator,
  SpinnerController,
  ObjectFollowLocator,
  Function4,
  InventoryActivator,
  MapStation,
  SaveStation,
  IntroBossRingController,
  ViewFrustumTest,
  ShotSpinnerController,
  EscapeSequence,
  BossEnergyBar,
  EndGame,
  HUDFadeIn,
  CinematicSkip,
  ScriptLayerController,
  RainSimulator,
  AreaDamage,
  ObjectFollowObject,
  HintSystem,
  DropBomb,
  Function22,
  MissileStation,
  Billboard,
  PlayerInAreaRelay,
  HUDTarget,
  FogFader,
  EnterLogbook,
  PowerBombStation,
  Ending,
  FusionRelay,
  WeaponSwitch // PAL Only
} SPECTER_ENUM("Special Function", "", EPickupType);

struct AnimationParameters : BigDNA {
  AT_DECL_DNA_YAML
  UniqueID32 animationCharacterSet;
  Value<atUint32> character;
  Value<atUint32> defaultAnimation;

  UniqueID32 getCINF(PAKRouter<PAKBridge>& pakRouter) const;

  void nameANCS(PAKRouter<PAKBridge>& pakRouter, const std::string& name) const {
    if (!animationCharacterSet)
      return;
    PAK::Entry* ancsEnt = (PAK::Entry*)pakRouter.lookupEntry(animationCharacterSet);
    if (ancsEnt->name.empty())
      ancsEnt->name = name;
  }

  void depANCS(std::vector<hecl::ProjectPath>& pathsOut) const {
    g_curSpec->flattenDependencies(animationCharacterSet, pathsOut, character);
  }

  void depANCSAll(std::vector<hecl::ProjectPath>& pathsOut) const {
    g_curSpec->flattenDependencies(animationCharacterSet, pathsOut);
  }
};

struct BehaveChance : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  Value<float> unknown1;
  Value<float> unknown2;
  Value<float> unknown3;
  Value<float> unknown4;
  Value<float> unknown5;
  Value<float> unknown6;
  Value<float> unknown7;
};

struct DamageInfo : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  Value<atUint32> weaponType;
  Value<float> damage;
  Value<float> radius;
  Value<float> knockbackPower;
};

struct DamageVulnerability : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  Value<atUint32> power;
  Value<atUint32> ice;
  Value<atUint32> wave;
  Value<atUint32> plasma;
  Value<atUint32> bomb;
  Value<atUint32> powerBomb;
  Value<atUint32> missile;
  Value<atUint32> boostBall;
  Value<atUint32> phazon;
  Value<atUint32> enemyWeapon1;
  Value<atUint32> enemyWeapon2Poison;
  Value<atUint32> enemyWeapon3Lava;
  Value<atUint32> enemyWeapon4;
  Value<atUint32> unkownWeapon1;
  Value<atUint32> unkownWeapon2;
  Value<atUint32> deflected;
  struct ChargedBeams : BigDNA {
    AT_DECL_DNA_YAML
    Value<atUint32> propertyCount;
    Value<atUint32> power;
    Value<atUint32> ice;
    Value<atUint32> wave;
    Value<atUint32> plasma;
    Value<atUint32> deflected;
  } chargedBeams;

  struct BeamCombos : BigDNA {
    AT_DECL_DNA_YAML
    Value<atUint32> propertyCount;
    Value<atUint32> superMissiles;
    Value<atUint32> iceSpreader;
    Value<atUint32> wavebuster;
    Value<atUint32> flameThrower;
    Value<atUint32> deflected;
  } beamCombos;
};

struct FlareDefinition : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  UniqueID32 texture;
  Value<float> unknown1;
  Value<float> unknown2;
  Value<atVec4f> unknown4; // CColor

  void nameIDs(PAKRouter<PAKBridge>& pakRouter, const std::string& name) const {
    if (texture) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture);
      ent->name = name + "_texture";
    }
  }

  void depIDs(std::vector<hecl::ProjectPath>& pathsOut) const { g_curSpec->flattenDependencies(texture, pathsOut); }
};

struct GrappleParameters : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  Value<float> unknown1;
  Value<float> unknown2;
  Value<float> unknown3;
  Value<float> unknown4;
  Value<float> unknown5;
  Value<float> unknown6;
  Value<float> unknown7;
  Value<float> unknown8;
  Value<float> unknown9;
  Value<float> unknown10;
  Value<float> unknown11;
  Value<bool> disableTurning;
};

struct HealthInfo : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  Value<float> health SPECTER_PROPERTY("Health", "Base health for object");
  Value<float> knockbackResistance SPECTER_PROPERTY("Knockback Resistance", "");
} SPECTER_PROPERTY("Health Info", "");

struct LightParameters : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  Value<bool> unknown1;
  Value<float> unknown2;
  Value<atUint32> shadowTesselation;
  Value<float> unknown3;
  Value<float> unknown4;
  Value<atVec4f> noLightsAmbient; // CColor
  Value<bool> makeLights;
  Value<atUint32> worldLightingOptions;
  Value<atUint32> lightRecalculationOptions;
  Value<atVec3f> actorPosBias;
  Value<atUint32> maxDynamicLights;
  Value<atUint32> maxAreaLights;
  Value<bool> ambientChannelOverflow;
  Value<atUint32> layerIndex;
};

struct PatternedInfo : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  Value<float> mass;
  Value<float> speed;
  Value<float> turnSpeed;
  Value<float> detectionRange;
  Value<float> detectionHeightRange;
  Value<float> dectectionAngle;
  Value<float> minAttackRange;
  Value<float> maxAttackRange;
  Value<float> averageAttackTime;
  Value<float> attackTimeVariation;
  Value<float> leashRadius;
  Value<float> playerLeashRadius;
  Value<float> playerLeashTime;
  DamageInfo contactDamage;
  Value<float> damageWaitTime;
  HealthInfo healthInfo;
  DamageVulnerability damageVulnerability;
  Value<float> halfExtent;
  Value<float> height;
  Value<atVec3f> bodyOrigin;
  Value<float> stepUpHeight;
  Value<float> xDamage;
  Value<float> frozenXDamage;
  Value<float> xDamageDelay;
  Value<atUint32> deathSfx;
  AnimationParameters animationParameters;
  Value<bool> active;
  UniqueID32 stateMachine;
  Value<float> intoFreezeDur;
  Value<float> outOfFreezeDur;
  Value<float> unknown10;
  Value<atUint32> pathfindingIndex;
  Value<atVec3f> particle1Scale;
  UniqueID32 particle1;
  UniqueID32 electric;
  Value<atVec3f> particle2Scale;
  UniqueID32 particle2;
  Value<atUint32> iceShatterSfx;

  void nameIDs(PAKRouter<PAKBridge>& pakRouter, const std::string& name) const {
    animationParameters.nameANCS(pakRouter, name + "_animp");
    if (stateMachine) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(stateMachine);
      ent->name = name + "_fsm";
    }
    if (particle1) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle1);
      ent->name = name + "_part1";
    }
    if (electric) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(electric);
      ent->name = name + "_elsc";
    }
    if (particle2) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle2);
      ent->name = name + "_part2";
    }
  }

  void depIDs(std::vector<hecl::ProjectPath>& pathsOut) const {
    animationParameters.depANCS(pathsOut);
    g_curSpec->flattenDependencies(stateMachine, pathsOut);
    g_curSpec->flattenDependencies(particle1, pathsOut);
    g_curSpec->flattenDependencies(electric, pathsOut);
    g_curSpec->flattenDependencies(particle2, pathsOut);
  }
};

struct PlayerHintParameters : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  Value<bool> unknown1;
  Value<bool> unknown2;
  Value<bool> extendTargetDistance;
  Value<bool> unknown4;
  Value<bool> unknown5;
  Value<bool> unknown6;
  Value<bool> unknown7;
  Value<bool> unknown8;
  Value<bool> unknown9;
  Value<bool> unknown10;
  Value<bool> unknown11;
  Value<bool> unknown12;
  Value<bool> unknown13;
  Value<bool> unknown14;
  Value<bool> unknown15;
};

struct ScannableParameters : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  UniqueID32 scanId;

  void nameIDs(PAKRouter<PAKBridge>& pakRouter, const std::string& name) const {
    if (scanId) {
      PAK::Entry* scanEnt = (PAK::Entry*)pakRouter.lookupEntry(scanId);
      scanEnt->name = name + "_scan";
    }
  }

  void depIDs(std::vector<hecl::ProjectPath>& pathsOut) const { g_curSpec->flattenDependencies(scanId, pathsOut); }

  void scanIDs(std::vector<Scan>& scansOut) const { scansOut.emplace_back(scanId); }
};

struct VisorParameters : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  Value<bool> unknown1;
  Value<bool> scanPassthrough;
  Value<atUint32> visorMask;
};

struct PlayerParameters : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  Vector<bool, AT_DNA_COUNT(propertyCount)> bools;
};

struct ActorParameters : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  LightParameters lightParameters;
  ScannableParameters scannableParameters;
  UniqueID32 cmdlXray;
  UniqueID32 cskrXray;
  UniqueID32 cmdlThermal;
  UniqueID32 cskrThermal;
  Value<bool> globalTimeProvider;
  Value<float> fadeInTime;
  Value<float> fadeOutTime;
  VisorParameters visorParameters;
  Value<bool> thermalHeat;
  Value<bool> renderUnsorted;
  Value<bool> noSortThermal;
  Value<float> thermalMag;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc,
                       const AnimationParameters& animParms) const {
    auto cinf = animParms.getCINF(pakRouter);
    if (cmdlXray && cskrXray) {
      charAssoc.m_cmdlRigs[cmdlXray] = std::make_pair(cskrXray, cinf);
      charAssoc.m_cskrCinfToCharacter[cskrXray] = std::make_pair(animParms.animationCharacterSet, "ATTACH.XRAY.CSKR");
      charAssoc.addAttachmentRig(animParms.animationCharacterSet, {}, cmdlXray, "XRAY");
    }
    if (cmdlThermal && cskrThermal) {
      charAssoc.m_cmdlRigs[cmdlThermal] = std::make_pair(cskrThermal, cinf);
      charAssoc.m_cskrCinfToCharacter[cskrThermal] =
          std::make_pair(animParms.animationCharacterSet, "ATTACH.THERMAL.CSKR");
      charAssoc.addAttachmentRig(animParms.animationCharacterSet, {}, cmdlThermal, "THERMAL");
    }
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter, const std::string& name) const {
    scannableParameters.nameIDs(pakRouter, name);
    if (cmdlXray) {
      PAK::Entry* xmEnt = (PAK::Entry*)pakRouter.lookupEntry(cmdlXray);
      xmEnt->name = name + "_xraymodel";
    }
    if (cskrXray) {
      PAK::Entry* xsEnt = (PAK::Entry*)pakRouter.lookupEntry(cskrXray);
      xsEnt->name = name + "_xrayskin";
    }
    if (cmdlThermal) {
      PAK::Entry* xmEnt = (PAK::Entry*)pakRouter.lookupEntry(cmdlThermal);
      xmEnt->name = name + "_thermalmodel";
    }
    if (cskrThermal) {
      PAK::Entry* xsEnt = (PAK::Entry*)pakRouter.lookupEntry(cskrThermal);
      xsEnt->name = name + "_thermalskin";
    }
  }

  void depIDs(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
    scannableParameters.depIDs(lazyOut);
    g_curSpec->flattenDependencies(cmdlXray, pathsOut);
    g_curSpec->flattenDependencies(cskrXray, pathsOut);
    g_curSpec->flattenDependencies(cmdlThermal, pathsOut);
    g_curSpec->flattenDependencies(cskrThermal, pathsOut);
  }

  void scanIDs(std::vector<Scan>& scansOut) const { scannableParameters.scanIDs(scansOut); }
};

struct BeamInfo : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> propertyCount;
  Value<atUint32> unknown1;
  UniqueID32 particle1;
  UniqueID32 particle2;
  UniqueID32 texture1;
  UniqueID32 texture2;
  Value<float> unknown2;
  Value<float> unknown3;
  Value<float> unknown4;
  Value<float> unknown5;
  Value<float> unknown6;
  Value<float> unknown7;
  Value<float> unknown8;
  Value<float> unknown9;
  Value<float> unknown10;
  DNAColor unknown11;
  DNAColor unknown12;

  void nameIDs(PAKRouter<PAKBridge>& pakRouter, const std::string& name) const {
    if (particle1) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle1);
      ent->name = name + "_part1";
    }
    if (particle2) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle2);
      ent->name = name + "_part2";
    }
    if (texture1) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture1);
      ent->name = name + "_tex1";
    }
    if (texture2) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture2);
      ent->name = name + "_tex2";
    }
  }

  void depIDs(std::vector<hecl::ProjectPath>& pathsOut) const {
    g_curSpec->flattenDependencies(particle1, pathsOut);
    g_curSpec->flattenDependencies(particle2, pathsOut);
    g_curSpec->flattenDependencies(texture1, pathsOut);
    g_curSpec->flattenDependencies(texture2, pathsOut);
  }
};

} // namespace DataSpec::DNAMP1
