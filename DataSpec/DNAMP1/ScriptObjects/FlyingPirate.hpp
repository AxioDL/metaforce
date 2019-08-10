#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct FlyingPirate : IScriptObject {
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
  Value<atUint32> unknown3;
  UniqueID32 wpsc1;
  DamageInfo damageInfo1;
  Value<atUint32> unknown4;
  UniqueID32 wpsc2;
  DamageInfo damageInfo2;
  UniqueID32 wpsc3;
  Value<float> unknown5;
  Value<float> unknown6;
  UniqueID32 particle1;
  DamageInfo damageInfo3;
  Value<float> unknown7;
  Value<float> unknown8;
  Value<float> unknown9;
  Value<float> unknown10;
  Value<atUint32> unknown11;
  Value<atUint32> unknown12;
  Value<float> unknown13;
  Value<float> unknown14;
  Value<float> unknown15;
  UniqueID32 particle2;
  UniqueID32 particle3;
  UniqueID32 particle4;
  Value<atUint32> unknown16;
  Value<atUint32> unknown17;
  Value<float> unknown18;
  Value<float> unknown19;
  Value<float> unknown20;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const override {
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, patternedInfo.animationParameters);
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    if (wpsc1.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc1);
      ent->name = name + "_wpsc1";
    }
    if (wpsc2.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc2);
      ent->name = name + "_wpsc2";
    }
    if (wpsc3.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(wpsc3);
      ent->name = name + "_wpsc3";
    }
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
    patternedInfo.nameIDs(pakRouter, name + "_patterned");
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    g_curSpec->flattenDependencies(wpsc1, pathsOut);
    g_curSpec->flattenDependencies(wpsc2, pathsOut);
    g_curSpec->flattenDependencies(wpsc3, pathsOut);
    g_curSpec->flattenDependencies(particle1, pathsOut);
    g_curSpec->flattenDependencies(particle2, pathsOut);
    g_curSpec->flattenDependencies(particle3, pathsOut);
    g_curSpec->flattenDependencies(particle4, pathsOut);
    patternedInfo.depIDs(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const override { actorParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
