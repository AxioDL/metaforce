#pragma once

namespace DataSpec {
class PAKEntryReadStream;
}

namespace hecl {
class ProjectPath;
}

namespace DataSpec::DNAAudio {

class ATBL {
public:
  static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
  static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath);
};

} // namespace DataSpec::DNAAudio
