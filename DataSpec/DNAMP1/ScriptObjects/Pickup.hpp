#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {

struct Pickup : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  Value<atVec3f> hitboxVolume;
  Value<atVec3f> scanPosition;
  Value<atUint32> pickupType;
  Value<atUint32> capacity;
  Value<atUint32> amount;
  Value<float> possibility;
  Value<float> lifeTime;
  Value<float> fadeInTime;
  UniqueID32 model;
  AnimationParameters animationParameters;
  ActorParameters actorParameters;
  Value<bool> active;
  Value<float> startDelay;
  UniqueID32 pickupParticle;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const override {
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, animationParameters);
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    if (pickupParticle.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(pickupParticle);
      ent->name = name + "_part";
    }
    if (model.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model);
      ent->name = name + "_model";
    }
    animationParameters.nameANCS(pakRouter, name + "_animp");
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    g_curSpec->flattenDependencies(pickupParticle, pathsOut);
    g_curSpec->flattenDependencies(model, pathsOut);
    animationParameters.depANCS(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const override { actorParameters.scanIDs(scansOut); }
};

} // namespace DataSpec::DNAMP1
