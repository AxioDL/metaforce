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
            bool x28_24_samusPak;
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
    IDvdRequest* x34_dvdReq;
    std::vector<std::pair<std::string, SObjectTag>> x4c_nameList;
    std::vector<u32> x5c_depList;
    std::vector<std::pair<u32, SResInfo>> x6c_resList;
public:
    CPakFile(const std::string& filename, bool samusPak, bool worldPak);
    const std::vector<u32>& GetDepList() const { return x5c_depList; }
    const SObjectTag* GetResIdByName(const char* name) const;
    const SResInfo* GetResInfoForLoad(u32 id) { return nullptr; }
    const SResInfo* GetResInfo(u32 id) const { return nullptr; }
    u32 GetFakeStaticSize() const { return 0; }
    void DataLoad() {}
    void InitialHeaderLoad() {}
    void Warmup() {}
    void AsyncIdle();
};

}

#endif // __URDE_CPAKFILE_HPP__
