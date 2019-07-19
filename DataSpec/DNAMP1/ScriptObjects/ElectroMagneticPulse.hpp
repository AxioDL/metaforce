#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct ElectroMagneticPulse : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<bool> active;
  Value<float> initialRadius;
  Value<float> finalRadius;
  Value<float> duration;
  Value<float> interferenceDur;
  Value<float> unknown6;
  Value<float> interferenceMag;
  Value<float> unknown8;
  UniqueID32 particle;

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const {
    if (particle) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(particle);
      ent->name = name + "_part";
    }
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
    g_curSpec->flattenDependencies(particle, pathsOut);
  }
};
} // namespace DataSpec::DNAMP1
