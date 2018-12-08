#pragma once

#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DataSpec/DNACommon/PAK.hpp"

namespace DataSpec::DNAMP1 {
struct HINT : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> magic;
  Value<atUint32> version;

  struct Hint : BigDNA {
    AT_DECL_DNA_YAML
    String<-1> name;
    Value<float> immediateTime;
    Value<float> normalTime;
    UniqueID32 stringID;
    Value<atUint32> textPageCount;
    struct Location : BigDNA {
      AT_DECL_DNA_YAML
      UniqueID32 worldAssetID;
      UniqueID32 areaAssetID;
      Value<atUint32> areaID;
      UniqueID32 stringID;
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

  static bool Cook(const hecl::ProjectPath& inPath, const hecl::ProjectPath& outPath) {
    HINT hint;
    athena::io::FileReader reader(inPath.getAbsolutePath());
    athena::io::FromYAMLStream(hint, reader);
    athena::io::FileWriter ws(outPath.getAbsolutePath());
    hint.write(ws);
    return true;
  }
};
} // namespace DataSpec::DNAMP1
