#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Geemer : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  PatternedInfo patternedInfo;
  ActorParameters actorParameters;
  Value<float> advanceWpRadius;
  Value<float> unknown2;
  Value<float> alignAngVel;
  Value<float> unknown4;
  Value<float> playerObstructionMinDist;
  Value<float> haltDelay;
  Value<float> forwardMoveWeight;
  Value<atUint32> haltSfx;
  Value<atUint32> getUpSfx;
  Value<atUint32> crouchSfx;

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
