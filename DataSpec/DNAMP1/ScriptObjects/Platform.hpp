#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Platform : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  Value<atVec3f> extent;
  Value<atVec3f> collisionCenter;
  UniqueID32 model;
  AnimationParameters animationParameters;
  ActorParameters actorParameters;
  Value<float> speed;
  Value<bool> active;
  UniqueID32 dcln;
  HealthInfo healthInfo;
  DamageVulnerability damageVulnerabilty;
  Value<bool> detectCollision;
  Value<float> xrayAlpha;
  Value<bool> rainSplashes;
  Value<atUint32> maxRainSplashes;
  Value<atUint32> rainGenRate;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const override {
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, animationParameters);
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    if (dcln.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(dcln);
      ent->name = name + "_dcln";
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
    g_curSpec->flattenDependencies(dcln, pathsOut);
    g_curSpec->flattenDependencies(model, pathsOut);
    animationParameters.depANCS(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const override { actorParameters.scanIDs(scansOut); }

  zeus::CAABox getVISIAABB(hecl::blender::Token& btok) const override;
};
} // namespace DataSpec::DNAMP1
