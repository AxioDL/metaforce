#include "CPakFile.hpp"

namespace urde
{

CPakFile::CPakFile(const std::string& filename, bool samusPak, bool worldPak)
: CDvdFile(filename.c_str())
{
    x28_24_samusPak = samusPak;
    x28_26_worldPak = worldPak;
}

const SObjectTag* CPakFile::GetResIdByName(const char* name) const
{
    for (const std::pair<std::string, SObjectTag>& p : x4c_nameList)
        if (!CStringExtras::CompareCaseInsensitive(p.first.c_str(), name))
            return &p.second;
    return nullptr;
}

void CPakFile::AsyncIdle()
{
    if (x2c_asyncLoadPhase == EAsyncPhase::Loaded)
        return;
    if (x34_dvdReq && x34_dvdReq->IsComplete())
        return;
    switch (x2c_asyncLoadPhase)
    {
    case EAsyncPhase::Warmup:
        Warmup();
        break;
    case EAsyncPhase::InitialHeader:
        InitialHeaderLoad();
        break;
    case EAsyncPhase::DataLoad:
        DataLoad();
        break;
    default: break;
    }
}

}
