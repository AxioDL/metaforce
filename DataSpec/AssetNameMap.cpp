#include "AssetNameMap.hpp"
#include "athena/MemoryReader.hpp"

extern "C" uint8_t ASSET_NAME_MP32[];
extern "C" uint32_t ASSET_NAME_MP32_SZ;
extern "C" uint8_t ASSET_NAME_MP64[];
extern "C" uint32_t ASSET_NAME_MP64_SZ;

namespace DataSpec
{
namespace AssetNameMap
{
logvisor::Module Log("AssetNameMap");

struct SAsset
{
    std::string name;
    std::string directory;
    hecl::FourCC type;
    SAsset() = default;
    SAsset(athena::io::IStreamReader& in)
    {
        uint32_t nameLen = in.readUint32Big();
        name = in.readString(nameLen);
        uint32_t dirLen = in.readUint32Big();
        directory = in.readString(dirLen);
        type = in.readUint32Big();
    }
};

static std::unordered_map<uint64_t, SAsset> g_AssetNameMap;
static bool g_AssetNameMapInit = false;

void LoadAssetMap(athena::io::MemoryReader ar)
{
    if (!ar.hasError())
    {
        hecl::FourCC magic = ar.readUint32Big();
        if (magic != FOURCC('AIDM'))
            Log.report(logvisor::Error, _S("Unable to load asset map; Assets will not have proper filenames for most files."));
        else
        {
            uint32_t assetCount = ar.readUint32Big();
            for (uint32_t i = 0 ; i<assetCount ; ++i)
            {
                uint64_t id = ar.readUint64Big();
                g_AssetNameMap[id] = SAsset(ar);
            }
        }
    }
}

void InitAssetNameMap()
{
    if (g_AssetNameMapInit)
        return;

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

const std::string* TranslateIdToName(const UniqueID32& id)
{
    if (g_AssetNameMap.find(id.toUint64()) == g_AssetNameMap.end())
        return nullptr;

    return &g_AssetNameMap[id.toUint64()].name;
}

}
}
