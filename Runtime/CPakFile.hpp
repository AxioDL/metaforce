#ifndef __URDE_CPAKFILE_HPP__
#define __URDE_CPAKFILE_HPP__

#include <vector>
#include "RetroTypes.hpp"
#include "CStringExtras.hpp"
#include "CDvdFile.hpp"
#include "CDvdRequest.hpp"

namespace urde
{

class CPakFile : public CDvdFile
{
    friend class CResLoader;
public:
    struct SResInfo
    {
        FourCC x0_type;
        u32 x4_offset;
        u32 x8_size;
        bool xb_compressed;
    };
private:
    union
    {
        struct
        {
            bool x28_24_buildDepList;
            bool x28_25_aramFile;
            bool x28_26_worldPak;
            bool x28_27_worldPakInitialized;
        };
        u32 _dummy = 0;
    };
    enum class EAsyncPhase
    {
        Warmup = 0,
        InitialHeader = 1,
        DataLoad = 2,
        Loaded = 3
    } x2c_asyncLoadPhase;
    std::shared_ptr<IDvdRequest> x30_dvdReq; // Used to be auto_ptr
    std::vector<u8> x38_headerData;
    u32 x48_resTableOffset = 0;
    u32 x4c_resTableCount = 0;
    int x50_ = -1;
    std::vector<std::pair<std::string, SObjectTag>> x54_nameList;
    std::vector<CAssetId> x64_depList;
    std::vector<std::pair<CAssetId, SResInfo>> x74_resList;
public:
    CPakFile(const std::string& filename, bool buildDepList, bool worldPak);
    const std::vector<CAssetId>& GetDepList() const { return x64_depList; }
    const SObjectTag* GetResIdByName(const char* name) const;
    const SResInfo* GetResInfoForLoad(CAssetId id) { return nullptr; }
    const SResInfo* GetResInfo(CAssetId id) const { return nullptr; }
    u32 GetFakeStaticSize() const { return 0; }
    void DataLoad() {}
    void InitialHeaderLoad() {}
    void Warmup() {}
    void AsyncIdle();
};

}

#endif // __URDE_CPAKFILE_HPP__
