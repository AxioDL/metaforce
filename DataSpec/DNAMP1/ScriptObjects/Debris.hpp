#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Debris : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<atVec3f> scale;
  Value<float> zImpulse;
  Value<atVec3f> velocity;
  DNAColor endsColor;
  Value<float> mass;
  Value<float> restitution;
  Value<float> duration;
  Value<atUint32> scaleType;
  Value<bool> randomAngImpulse;
  UniqueID32 model;
  ActorParameters actorParameters;
  UniqueID32 particle;
  Value<atVec3f> particleScale;
  Value<bool> b1;
  Value<bool> active;

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {
    if (model) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(model);
      ent->name = name + "_model";
    }
    if (particle) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle);
      ent->name = name + "_part";
    }
    actorParameters.nameIDs(pakRouter, name + "_actp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
    g_curSpec->flattenDependencies(model, pathsOut);
    g_curSpec->flattenDependencies(particle, pathsOut);
    actorParameters.depIDs(pathsOut, lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const { actorParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
