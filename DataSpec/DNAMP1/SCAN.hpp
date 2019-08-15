#pragma once

#include <athena/FileWriter.hpp>
#include "DataSpec/DNACommon/DNACommon.hpp"
#include "DNAMP1.hpp"

namespace DataSpec::DNAMP1 {
struct SCAN : BigDNA {
  AT_DECL_DNA_YAML
  Value<atUint32> version;
  Value<atUint32> magic;
  UniqueID32 frame;
  UniqueID32 string;

  enum class ScanSpeed : atUint32 { Normal, Slow };
  Value<ScanSpeed> scanSpeed;

  enum class Category : atUint32 { None, SpacePirateData, ChozoLore, Creatures, Research };

  Value<Category> category;

  Value<bool> isImportant;

  struct Texture : BigDNA {
    AT_DECL_EXPLICIT_DNA_YAML
    UniqueID32 texture;
    Value<float> appearanceRange;
    enum class Position : atInt32 {
      Pane0,
      Pane1,
      Pane2,
      Pane3,
      Pane01,
      Pane12,
      Pane23,
      Pane012,
      Pane123,
      Pane0123,
      Pane4,
      Pane5,
      Pane6,
      Pane7,
      Pane45,
      Pane56,
      Pane67,
      Pane456,
      Pane567,
      Pane4567,
      Invalid = -1
    };
    Value<Position> position;
    Value<atUint32> width;     // width of animation cell
    Value<atUint32> height;    // height of animation cell
    Value<float> interval;     // 0.0 - 1.0
    Value<float> fadeDuration; // 0.0 - 1.0
  };

  Texture textures[4];

  static bool Extract(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath) {
    SCAN scan;
    scan.read(rs);
    athena::io::FileWriter writer(outPath.getAbsolutePath());
    athena::io::ToYAMLStream(scan, writer);
    return true;
  }

  static bool Cook(const SCAN& scan, const hecl::ProjectPath& outPath) {
    athena::io::FileWriter ws(outPath.getAbsolutePath());
    scan.write(ws);
    return true;
  }

  static Category GetCategory(const hecl::ProjectPath& inPath) {
    SCAN scan;
    athena::io::FileReader reader(inPath.getAbsolutePath());
    if (reader.hasError())
      return Category::None;
    if (!athena::io::FromYAMLStream(scan, reader))
      return Category::None;
    return scan.category;
  }

  static void Name(const SpecBase& dataSpec, PAKEntryReadStream& rs, PAKRouter<PAKBridge>& pakRouter,
                   PAK::Entry& entry) {
    SCAN scan;
    scan.read(rs);
    if (scan.string.isValid()) {
      PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(scan.string);
      ent->name = fmt::format(fmt("SCAN_{}_strg"), entry.id);
    }
    for (int i = 0; i < 4; ++i) {
      const Texture& tex = scan.textures[i];
      if (tex.texture.isValid()) {
        PAK::Entry* ent = (PAK::Entry*)pakRouter.lookupEntry(tex.texture);
        ent->name = fmt::format(fmt("SCAN_{}_tex{}"), entry.id, i + 1);
      }
    }
  }

  void gatherDependencies(std::vector<hecl::ProjectPath>& pathsOut) const {
    g_curSpec->flattenDependencies(frame, pathsOut);
    g_curSpec->flattenDependencies(string, pathsOut);
    for (int i = 0; i < 4; ++i)
      g_curSpec->flattenDependencies(textures[i].texture, pathsOut);
  }
};

} // namespace DataSpec::DNAMP1
