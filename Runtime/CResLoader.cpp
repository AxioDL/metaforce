#include "CResLoader.hpp"
#include "CPakFile.hpp"

namespace urde
{

CResLoader::CResLoader()
{
    x48_curPak = x18_pakLoadedList.end();
}

const std::vector<CAssetId>* CResLoader::GetTagListForFile(const std::string& name) const
{
    std::string namePak = name + ".pak";
    for (const std::unique_ptr<CPakFile>& pak : x18_pakLoadedList)
        if (!CStringExtras::CompareCaseInsensitive(namePak, pak->x18_path))
            return &pak->GetDepList();
    return nullptr;
}

void CResLoader::AddPakFileAsync(const std::string& name, bool samusPak, bool worldPak)
{
    std::string namePak = name + ".pak";
    if (CDvdFile::FileExists(namePak.c_str()))
    {
        x30_pakLoadingList.emplace_back(new CPakFile(namePak, samusPak, worldPak));
        ++x44_pakLoadingCount;
    }
}

void CResLoader::AddPakFile(const std::string& name, bool samusPak, bool worldPak)
{
    AddPakFileAsync(name, samusPak, worldPak);
    while (x44_pakLoadingCount)
        AsyncIdlePakLoading();
}

std::unique_ptr<CInputStream> CResLoader::LoadNewResourcePartSync(const SObjectTag& tag, int offset, int length, void* extBuf)
{
    void* buf = extBuf;
    CPakFile* file = FindResourceForLoad(tag);
    if (!buf)
        buf = new u8[length];
    file->SyncSeekRead(buf, length, ESeekOrigin::Begin, x50_cachedResInfo->GetOffset() + offset);
    return std::unique_ptr<CInputStream>(new athena::io::MemoryReader((atUint8*)buf, length, !extBuf));
}

void CResLoader::LoadMemResourceSync(const SObjectTag& tag, std::unique_ptr<u8[]>& bufOut, int* sizeOut)
{
    CPakFile* file = FindResourceForLoad(tag);
    bufOut = std::unique_ptr<u8[]>(new u8[x50_cachedResInfo->GetSize()]);
    file->SyncSeekRead(bufOut.get(), x50_cachedResInfo->GetSize(), ESeekOrigin::Begin,
                       x50_cachedResInfo->GetOffset());
    *sizeOut = x50_cachedResInfo->GetSize();
}

std::unique_ptr<CInputStream> CResLoader::LoadResourceFromMemorySync(const SObjectTag& tag, const void* buf)
{
    FindResourceForLoad(tag);
    CInputStream* newStrm = new athena::io::MemoryReader((atUint8*)buf, x50_cachedResInfo->GetSize());
    if (x50_cachedResInfo->IsCompressed())
    {
        newStrm->readUint32Big();
        newStrm = new CZipInputStream(std::unique_ptr<CInputStream>(newStrm));
    }
    return std::unique_ptr<CInputStream>(newStrm);
}

std::unique_ptr<CInputStream> CResLoader::LoadNewResourceSync(const SObjectTag& tag, void* extBuf)
{
    void* buf = extBuf;
    CPakFile* file = FindResourceForLoad(tag);
    size_t resSz = ROUND_UP_32(x50_cachedResInfo->GetSize());
    if (!buf)
        buf = new u8[resSz];
    file->SyncSeekRead(buf, resSz, ESeekOrigin::Begin, x50_cachedResInfo->GetOffset());
    CInputStream* newStrm = new athena::io::MemoryReader((atUint8*)buf, resSz, !extBuf);
    if (x50_cachedResInfo->IsCompressed())
    {
        newStrm->readUint32Big();
        newStrm = new CZipInputStream(std::unique_ptr<CInputStream>(newStrm));
    }
    return std::unique_ptr<CInputStream>(newStrm);
}

std::shared_ptr<IDvdRequest> CResLoader::LoadResourcePartAsync(const SObjectTag& tag, int offset, int length, void* buf)
{
    return FindResourceForLoad(tag.id)->AsyncSeekRead(buf, length,
                                                      ESeekOrigin::Begin, x50_cachedResInfo->GetOffset() + offset);
}

std::shared_ptr<IDvdRequest> CResLoader::LoadResourceAsync(const SObjectTag& tag, void* buf)
{
    return FindResourceForLoad(tag.id)->AsyncSeekRead(buf, ROUND_UP_32(x50_cachedResInfo->GetSize()),
                                                      ESeekOrigin::Begin, x50_cachedResInfo->GetOffset());
}

bool CResLoader::GetResourceCompression(const SObjectTag& tag)
{
    if (FindResource(tag.id))
        return x50_cachedResInfo->IsCompressed();
    return false;
}

u32 CResLoader::ResourceSize(const SObjectTag& tag)
{
    if (FindResource(tag.id))
        return x50_cachedResInfo->GetSize();
    return 0;
}

bool CResLoader::ResourceExists(const SObjectTag& tag)
{
    return FindResource(tag.id);
}

FourCC CResLoader::GetResourceTypeById(CAssetId id) const
{
    if (FindResource(id))
        return x50_cachedResInfo->GetType();
    return FourCC();
}

const SObjectTag* CResLoader::GetResourceIdByName(const char* name) const
{
    for (const std::unique_ptr<CPakFile>& file : x18_pakLoadedList)
    {
        const SObjectTag* id = file->GetResIdByName(name);
        if (id)
            return id;
    }
    return nullptr;
}

bool CResLoader::AreAllPaksLoaded() const
{
    return x44_pakLoadingCount == 0;
}

void CResLoader::AsyncIdlePakLoading()
{
    for (auto it=x30_pakLoadingList.begin();
         it != x30_pakLoadingList.end();
         ++it)
    {
        (*it)->AsyncIdle();
        if ((*it)->x2c_asyncLoadPhase == CPakFile::EAsyncPhase::Loaded)
        {
            MoveToCorrectLoadedList(std::move(*it));
            it = x30_pakLoadingList.erase(it);
            --x44_pakLoadingCount;
        }
    }
}

bool CResLoader::FindResource(CAssetId id) const
{
    if (x4c_cachedResId == id)
        return true;

    if (x48_curPak != x18_pakLoadedList.end())
        if (CacheFromPak(**x48_curPak, id))
            return true;

    for (auto it = x18_pakLoadedList.begin() ; it != x18_pakLoadedList.end() ; ++it)
    {
        if (it == x48_curPak)
            continue;
        if (CacheFromPak(**it, id))
            return true;
    }

    return false;
}

CPakFile* CResLoader::FindResourceForLoad(CAssetId id)
{
    if (x48_curPak != x18_pakLoadedList.end())
        if (CacheFromPakForLoad(**x48_curPak, id))
            return &**x48_curPak;

    for (auto it = x18_pakLoadedList.begin() ; it != x18_pakLoadedList.end() ; ++it)
    {
        if (it == x48_curPak)
            continue;
        if (CacheFromPakForLoad(**it, id))
        {
            x48_curPak = it;
            return &**it;
        }
    }

    return nullptr;
}

CPakFile* CResLoader::FindResourceForLoad(const SObjectTag& tag)
{
    return FindResourceForLoad(tag.id);
}

bool CResLoader::CacheFromPakForLoad(CPakFile& file, CAssetId id)
{
    const CPakFile::SResInfo* info;
    if (x54_forwardSeek)
    {
        info = file.GetResInfoForLoadPreferForward(id);
        x54_forwardSeek = false;
    }
    else
    {
        info = file.GetResInfoForLoadDirectionless(id);
    }
    if (info)
    {
        x4c_cachedResId = id;
        x50_cachedResInfo = info;
        return true;
    }
    return false;
}

bool CResLoader::CacheFromPak(const CPakFile& file, CAssetId id) const
{
    const CPakFile::SResInfo* info = file.GetResInfo(id);
    if (info)
    {
        x4c_cachedResId = id;
        x50_cachedResInfo = info;
        return true;
    }
    return false;
}

void CResLoader::MoveToCorrectLoadedList(std::unique_ptr<CPakFile>&& file)
{
    x18_pakLoadedList.push_back(std::move(file));
}

}
