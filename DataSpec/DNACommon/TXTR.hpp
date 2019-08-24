#pragma once

#include <climits>

#include "DataSpec/DNACommon/DNACommon.hpp"

namespace hecl {
class ProjectPath;
}

namespace DataSpec {
class PAKEntryReadStream;

struct TXTR {
  struct PaletteMeta : BigDNAVYaml {
    AT_DECL_EXPLICIT_DNA_YAMLV
    Value<atUint32> format = UINT_MAX;
    Value<atUint32> elementCount = 0;
    Value<atUint64> dolphinHash = 0;
  };
  struct Meta : BigDNAVYaml {
    AT_DECL_EXPLICIT_DNA_YAMLV
    Value<atUint32> format = UINT_MAX;
    Value<atUint32> mips = 0;
    Value<atUint16> width = 0;
    Value<atUint16> height = 0;
    Value<atUint64> dolphinHash = 0;
    Value<bool> hasPalette = false;
    PaletteMeta palette;
  };

  static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
  static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath);
  static bool CookPC(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath);
  static TXTR::Meta GetMetaData(PAKEntryReadStream& rs);
};

} // namespace DataSpec
