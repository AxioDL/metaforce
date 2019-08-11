#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct PointOfInterest : IScriptObject {
  AT_DECL_DNA_YAMLV
  String<-1> name;
  Value<atVec3f> location;
  Value<atVec3f> orientation;
  Value<bool> active;
  ScannableParameters scannableParameters;
  Value<float> pointSize;

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    scannableParameters.nameIDs(pakRouter, name + "_scanp");
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    scannableParameters.depIDs(lazyOut);
  }

  void gatherScans(std::vector<Scan>& scansOut) const override { scannableParameters.scanIDs(scansOut); }
};
} // namespace DataSpec::DNAMP1
