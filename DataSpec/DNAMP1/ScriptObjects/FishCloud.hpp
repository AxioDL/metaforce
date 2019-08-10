#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct FishCloud : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> volume;
  Value<bool> active;
  UniqueID32 model;
  AnimationParameters animationParameters;
  Value<atUint32> numBoids;
  Value<float> speed;
  Value<float> separationRadius;
  Value<float> cohesionMagnitude;
  Value<float> alignmentWeight;
  Value<float> separationMagnitude;
  Value<float> weaponRepelMagnitude;
  Value<float> playerRepelMagnitude;
  Value<float> containmentMagnitude;
  Value<float> scatterVel;
  Value<float> maxScatterAngle;
  Value<float> weaponRepelDampingSpeed;
  Value<float> playerRepelDampingSpeed;
  Value<float> containmentRadius;
  Value<atUint32> updateShift;
  Value<atVec4f> color; // CColor
  Value<bool> killable;
  Value<float> weaponKillRadius;
  UniqueID32 deathParticle1;
  Value<atUint32> deathParticle1Count;
  UniqueID32 deathParticle2;
  Value<atUint32> deathParticle2Count;
  UniqueID32 deathParticle3;
  Value<atUint32> deathParticle3Count;
  UniqueID32 deathParticle4;
  Value<atUint32> deathParticle4Count;
  Value<atUint32> deathSFX;
  Value<bool> repelFromThreats;
  Value<bool> hotInThermal;

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    if (model.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model);
      ent->name = name + "_model";
    }
    animationParameters.nameANCS(pakRouter, name + "_animp");
    if (deathParticle1.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(deathParticle1);
      ent->name = name + "_deathParticle1";
    }
    if (deathParticle2.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(deathParticle2);
      ent->name = name + "_deathParticle2";
    }
    if (deathParticle3.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(deathParticle3);
      ent->name = name + "_deathParticle3";
    }
    if (deathParticle4.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(deathParticle4);
      ent->name = name + "_deathParticle4";
    }
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    g_curSpec->flattenDependencies(model, pathsOut);
    animationParameters.depANCS(pathsOut);
    g_curSpec->flattenDependencies(deathParticle1, pathsOut);
    g_curSpec->flattenDependencies(deathParticle2, pathsOut);
    g_curSpec->flattenDependencies(deathParticle3, pathsOut);
    g_curSpec->flattenDependencies(deathParticle4, pathsOut);
  }
};
} // namespace DataSpec::DNAMP1
