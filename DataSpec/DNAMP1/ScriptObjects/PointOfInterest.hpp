#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct PointOfInterest : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<bool> unknown1;
  ScannableParameters scannableParameters;
  Value<float> unknown2;

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const { scannableParameters.nameIDs(pakRouter, name + "_scanp"); }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut, std::vector<hecl::ProjectPath>& lazyOut) const {
    scannableParameters.depIDs(lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const { scannableParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
