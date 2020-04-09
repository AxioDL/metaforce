#pragma once
#include "DataSpec/DNACommon/PATH.hpp"
#include "DataSpec/DNAMP1/DNAMP1.hpp"

namespace DataSpec::DNAMP1 {
struct PATH : DNAPATH::PATH<4> {
  static bool Extract(const SpecBase& dataSpec, PAKEntryReadStream& rs, const hecl::ProjectPath& outPath,
                      PAKRouter<PAKBridge>& pakRouter, const PAK::Entry& entry, bool force, hecl::blender::Token& btok,
                      std::function<void(const hecl::SystemChar*)> fileChanged);
};
} // namespace DataSpec::DNAMP1