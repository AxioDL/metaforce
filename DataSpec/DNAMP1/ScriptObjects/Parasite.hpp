#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Parasite : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atUint32> flavor;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  PatternedInfo patternedInfo;
  ActorParameters actorParameters;
  Value<float> maxTelegraphReactDist;
  Value<float> advanceWpRadius;
  Value<float> unknown4;
  Value<float> alignAngVel;
  Value<float> unknown6;
  Value<float> stuckTimeThreshold;
  Value<float> collisionCloseMargin;
  Value<float> parasiteSearchRadius;
  Value<float> parasiteSeparationDist;
  Value<float> parasiteSeparationWeight;
  Value<float> parasiteAlignmentWeight;
  Value<float> parasiteCohesionWeight;
  Value<float> destinationSeekWeight;
  Value<float> forwardMoveWeight;
  Value<float> playerSeparationDist;
  Value<float> playerSeparationWeight;
  Value<float> playerObstructionMinDist;
  Value<bool> disableMove;

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
