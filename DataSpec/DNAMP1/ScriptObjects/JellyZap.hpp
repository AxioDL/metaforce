#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct JellyZap : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  PatternedInfo patternedInfo;
  ActorParameters actorParameters;
  DamageInfo damageInfo;
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
  Value<float> unknown12;
  Value<bool> unknown13;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const override {
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, patternedInfo.animationParameters);
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    patternedInfo.nameIDs(pakRouter, name + "_patterned");
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    patternedInfo.depIDs(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const override { actorParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
