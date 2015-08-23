#ifndef __RETRO_CPAKFILE_HPP__
#define __RETRO_CPAKFILE_HPP__

#include <vector>
#include "RetroTypes.hpp"
#include "CStringExtras.hpp"
#include "CDvdFile.hpp"
#include "CDvdRequest.hpp"

namespace Retro
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
    bool x28_b24_ctFlag;
    enum EAsyncPhase
    {
        PakAsyncWarmup = 0,
        PakAsyncInitialHeader = 1,
        PakAsyncDataLoad = 2,
        PakAsyncLoaded = 3
    } x2c_asyncLoadPhase;
    CDvdRequest* x34_dvdReq;
    std::vector<std::pair<std::string, SObjectTag>> x4c_nameList;
    std::vector<u32> x5c_depList;
    std::vector<std::pair<u32, SResInfo>> x6c_resList;
public:
    CPakFile(const std::string& filename, bool flag);
    const std::vector<u32>& GetDepList() const {return x5c_depList;}
    u32 GetResIdByName(const char* name) const
    {
        for (const std::pair<std::string, SObjectTag>& p : x4c_nameList)
            if (!CStringExtras::CompareCaseInsensitive(p.first.c_str(), name))
                return p.second.id;
        return 0;
    }
    const SResInfo* GetResInfoForLoad(u32 id);
    const SResInfo* GetResInfo(u32 id) const;
    u32 GetFakeStaticSize() const;
    void DataLoad();
    void InitialHeaderLoad();
    void Warmup();
    void AsyncIdle()
    {
        if (x2c_asyncLoadPhase == PakAsyncLoaded)
            return;
        if (x34_dvdReq && x34_dvdReq->IsComplete())
            return;
        switch (x2c_asyncLoadPhase)
        {
        case PakAsyncWarmup:
            Warmup();
            break;
        case PakAsyncInitialHeader:
            InitialHeaderLoad();
            break;
        case PakAsyncDataLoad:
            DataLoad();
            break;
        default: break;
        }
    }
};

}

#endif // __RETRO_CPAKFILE_HPP__
