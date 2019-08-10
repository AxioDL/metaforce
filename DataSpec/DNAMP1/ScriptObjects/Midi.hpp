#pragma once

#include "../../DNACommon/DNACommon.hpp"
#include "IScriptObject.hpp"
#include "Parameters.hpp"

namespace DataSpec::DNAMP1 {
struct Midi : IScriptObject {
  AT_DECL_DNA_YAML
  AT_DECL_DNAV
  String<-1> name;
  Value<bool> active;
  UniqueID32 song;
  Value<float> fadeInTime;
  Value<float> fadeOutTime;
  Value<atUint32> volume;

  void nameIDs(PAKRouter<PAKBridge>& pakRouter) const override {
    if (song.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(song);
      ent->name = name + "_song";
    }
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut,
                          std::vector<hecl::ProjectPath>& lazyOut) const override {
    // Dedicated PAK for this
    // g_curSpec->flattenDependencies(song, pathsOut);
  }
};
} // namespace DataSpec::DNAMP1
