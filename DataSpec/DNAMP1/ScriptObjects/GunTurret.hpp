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
  Value<atVec3f> unknown2;
  Value<atVec3f> scanOffset;
  AnimationParameters animationParameters;
  ActorParameters actorParameters;
  HealthInfo healthInfo;
  DamageVulnerability damageVulnerabilty;
  Value<float> unknown3;
  Value<float> unknown4;
  Value<float> unknown5;
  Value<float> unknown6;
  Value<float> unknown7;
  Value<float> unknown8;
  Value<float> unknown9;
  Value<float> unknown10;
  Value<float> unknown11;
  Value<float> unknown12;
  Value<float> unknown13;
  Value<float> unknown14;
  Value<float> unknown15;
  Value<float> unknown16;
  Value<bool> unknown17;
  UniqueID32 unknown18;
  DamageInfo damageInfo;
  UniqueID32 particle1;
  UniqueID32 particle2;
  UniqueID32 particle3;
  UniqueID32 particle4;
  UniqueID32 particle5;
  UniqueID32 particle6;
  UniqueID32 particle7;
  Value<atUint32> unknown19;
  Value<atUint32> unknown20;
  Value<atUint32> unknown21;
  Value<atUint32> unknown22;
  Value<atUint32> unknown23;
  Value<atUint32> unknown24;
  UniqueID32 model;
  Value<float> unknown25;
  Value<atUint32> unknown26;
  Value<atUint32> unknown27;
  Value<atUint32> unknown28;
  Value<float> unknown29;
  Value<bool> unknown30;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const {
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, animationParameters);
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {
    if (unknown18) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(unknown18);
      ent->name = name + "_unknown18";
    }
    if (model) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model);
      ent->name = name + "_model";
    }
    if (particle1) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle1);
      ent->name = name + "_part1";
    }
    if (particle2) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle2);
      ent->name = name + "_part2";
    }
    if (particle3) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle3);
      ent->name = name + "_part3";
    }
    if (particle4) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle4);
      ent->name = name + "_part4";
    }
    if (particle5) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle5);
      ent->name = name + "_part5";
    }
    if (particle6) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle6);
      ent->name = name + "_part6";
    }
    if (particle7) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle7);
      ent->name = name + "_part7";
    }
    animationParameters.nameANCS(pakRouter, name + "_animp");
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
    g_curSpec->flattenDependencies(unknown18, pathsOut);
    g_curSpec->flattenDependencies(model, pathsOut);
    g_curSpec->flattenDependencies(particle1, pathsOut);
    g_curSpec->flattenDependencies(particle2, pathsOut);
    g_curSpec->flattenDependencies(particle3, pathsOut);
    g_curSpec->flattenDependencies(particle4, pathsOut);
    g_curSpec->flattenDependencies(particle5, pathsOut);
    g_curSpec->flattenDependencies(particle6, pathsOut);
    g_curSpec->flattenDependencies(particle7, pathsOut);
    animationParameters.depANCS(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const { actorParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
