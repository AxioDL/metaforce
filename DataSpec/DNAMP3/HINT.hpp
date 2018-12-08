#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "PAK.hpp"

namespace DataSpec::DNAMP3 {
struct HINT : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> magic;
  Value<atUint32> version;

  struct Hint : BigDNA {
    AT_DECL_DNA_YAML
    String<-1> name;
    Value<float> unknown1;
    Value<float> fadeInTime;
    UniqueID64 stringID;
    Value<atUint32> unknown2;
    struct Location : BigDNA {
      AT_DECL_DNA_YAML
      UniqueID64 worldAssetID;
      UniqueID64 areaAssetID;
      Value<atUint32> areaID;
      UniqueID64 stringID;
      Value<atUint32> unknown[3];
    };

    Value<atUint32> locationCount;
    Vector<Location, AT_DNA_COUNT(locationCount)> locations;
  };
  Value<atUint32> hintCount;
  Vector<Hint, AT_DNA_COUNT(hintCount)> hints;

  static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath) {
    HINT hint;
    hint.read(rs);
    athena::io::FileWriter writer(outPath.getAbsolutePath());
    athena::io::ToYAMLStream(hint, writer);
    return true;
  }
};
} // namespace DataSpec::DNAMP3
