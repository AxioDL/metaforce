#pragma once
#include "DataSpec/DNACommon/PATH.hpp"
#include "DataSpec/DNAMP2/DNAMP2.hpp"

namespace DataSpec::DNAMP2 {
struct PATH : DNAPATH::PATH<6> {
  static bool Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                      PAKRouter<PAKBridge>& pakRouter, const PAK::Entry& entry, bool force, hecl::blender::Token& btok,
                      std::function<void(const hecl::SystemChar*)> fileChanged);

  static bool Cook(const hecl::ProjectPath& outPath, const hecl::ProjectPath& inPath,
                              const PathMesh& mesh, hecl::blender::Token& btok);
};
}