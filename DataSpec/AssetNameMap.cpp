#include "AssetNameMap.hpp"
#include "athena/MemoryReader.hpp"

extern "C" const uint8_t ASSET_NAME_MP32[];
extern "C" const size_t ASSET_NAME_MP32_SZ;
extern "C" const uint8_t ASSET_NAME_MP64[];
extern "C" const size_t ASSET_NAME_MP64_SZ;

namespace DataSpec::AssetNameMap {
logvisor::Module Log("AssetNameMap");

struct SAsset {
  std::string name;
  std::string directory;
  hecl::FourCC type;
  SAsset() = default;
  SAsset(const hecl::FourCC& typeIn, athena::io::IStreamReader& in) : type(typeIn) {
    uint32_t nameLen = in.readUint32Big();
    name = in.readString(nameLen);
    uint32_t dirLen = in.readUint32Big();
    directory = in.readString(dirLen);
  }
};

static std::unordered_map<uint64_t, SAsset> g_AssetNameMap;
static bool g_AssetNameMapInit = false;

void LoadAssetMap(athena::io::MemoryReader& ar) {
  if (!ar.hasError()) {
    hecl::FourCC magic;
    if (ar.length() >= 4)
      ar.readBytesToBuf(&magic, 4);
    if (magic != FOURCC('AIDM'))
      Log.report(logvisor::Warning,
                 _SYS_STR("Unable to load asset map; Assets will not have proper filenames for most files."));
    else {
      uint32_t assetCount = ar.readUint32Big();
      g_AssetNameMap.reserve(assetCount);
      for (uint32_t i = 0; i < assetCount; ++i) {
        hecl::FourCC type;
        ar.readBytesToBuf(&type, 4);
        uint64_t id = ar.readUint64Big();
        g_AssetNameMap[id] = SAsset(type, ar);
      }
    }
  }
}

void InitAssetNameMap() {
  if (g_AssetNameMapInit)
    return;

  Log.report(logvisor::Info, "Initializing asset name database...");

  /* First load the 32bit map for MP1/2 */
  {
    athena::io::MemoryReader ar(ASSET_NAME_MP32, ASSET_NAME_MP32_SZ);
    LoadAssetMap(ar);
  }
  /* Now load the 64bit map for MP3 */
  {
    athena::io::MemoryReader ar(ASSET_NAME_MP64, ASSET_NAME_MP64_SZ);
    LoadAssetMap(ar);
  }
  g_AssetNameMapInit = true;
}

const std::string* TranslateIdToName(const UniqueID32& id) {
  if (g_AssetNameMap.find(id.toUint64()) == g_AssetNameMap.end())
    return nullptr;

  return &g_AssetNameMap[id.toUint64()].name;
}

} // namespace DataSpec::AssetNameMap
