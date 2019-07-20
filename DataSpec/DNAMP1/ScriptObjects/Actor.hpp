#pragma once

#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Actor : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name SO_NAME_SPECPROP();
  Value<atVec3f> location SO_LOCATION_SPECPROP();
  Value<atVec3f> orientation SO_ORIENTATION_SPECPROP();
  Value<atVec3f> scale SO_SCALE_SPECPROP();
  Value<atVec3f> collisionExtent SO_COLLISION_EXTENT_SPECPROP();
  Value<atVec3f> collisionOffset SO_COLLISION_OFFSET_SPECPROP();
  Value<float> mass;
  Value<float> zMomentum;
  HealthInfo healthInfo;
  DamageVulnerability damageVulnerability;
  UniqueID32 model;
  AnimationParameters animationParameters;
  ActorParameters actorParameters;
  Value<bool> looping;
  Value<bool> immovable;
  Value<bool> solid;
  Value<bool> cameraPassthrough;
  Value<bool> active;
  Value<atUint32> shaderIdx;
  Value<float> xrayAlpha;
  Value<bool> noThermalHotZ;
  Value<bool> castsShadow;
  Value<bool> scaleAdvancementDelta;
  Value<bool> materialFlag54;

  void addCMDLRigPairs(PAKRouter<PAKBridge>& pakRouter, CharacterAssociations<UniqueID32>& charAssoc) const {
    actorParameters.addCMDLRigPairs(pakRouter, charAssoc, animationParameters);
  }

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {
    if (model.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model);
      ent->name = name + "_model";
    }
    animationParameters.nameANCS(pakRouter, name + "_animp");
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
    g_curSpec->flattenDependencies(model, pathsOut);
    animationParameters.depANCS(pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const { actorParameters.scanIDs(scansOut); }

  zeus::CAABox getVISIAABB(hecl::blender::Token& btok) const;
};
} // namespace DataSpec::DNAMP1
