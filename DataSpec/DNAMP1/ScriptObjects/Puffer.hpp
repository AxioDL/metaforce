#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Puffer : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  PatternedInfo patternedInfo;
  ActorParameters actorParameters;
  Value<float> unknown1;
  UniqueID32 particle;
  DamageInfo damageInfo1;
  UniqueID32 texture;
  Value<float> unknown2;
  Value<bool> unknown3;
  Value<bool> unknown4;
  Value<bool> unknown5;
  DamageInfo damageInfo2;
  Value<atUint32> unknown6;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const override {
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, patternedInfo.animationParameters);
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    if (particle.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle);
      ent->name = name + "_part";
    }
    if (texture.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(texture);
      ent->name = name + "_texture";
    }
    patternedInfo.nameIDs(pakRouter, name + "_patterned");
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    g_curSpec->flattenDependencies(particle, pathsOut);
    g_curSpec->flattenDependencies(texture, pathsOut);
    patternedInfo.depIDs(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const override { actorParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
