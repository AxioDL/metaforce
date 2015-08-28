#include "CResLoader.hpp"
#include "CPakFile.hpp"
#include "CMemory.hpp"
#include "CCallStack.hpp"

namespace Retro
{

const std::vector<u32>* CResLoader::GetTagListForFile(const std::string& name) const
{
    std::string namePak = name + ".pak";
    for (const std::unique_ptr<CPakFile>& pak : x1c_pakLoadedList)
        if (!CStringExtras::CompareCaseInsensitive(namePak, pak->x18_name))
            return &pak->GetDepList();
    return nullptr;
}

void CResLoader::AddPakFileAsync(const std::string& name, bool flag)
{
    std::string namePak = name + ".pak";
    if (CDvdFile::FileExists(namePak.c_str()))
    {
        x34_pakLoadingList.emplace_back(new CPakFile(namePak, flag));
        ++x44_pakLoadingCount;
    }
}

void CResLoader::AddPakFile(const std::string& name, bool flag)
{
    AddPakFileAsync(name, flag);
    while (x44_pakLoadingCount)
        AsyncIdlePakLoading();
}

CInputStream* CResLoader::LoadNewResourcePartSync(const SObjectTag& tag, int offset, int length, void* extBuf)
{
    void* buf = extBuf;
    CPakFile* file = FindResourceForLoad(tag);
    if (!buf)
    {
        CCallStack cs(AT_PRETTY_FUNCTION, "UnknownType");
        buf = CMemory::Alloc(length,
                             IAllocator::HintLarge,
                             IAllocator::ScopeDefault,
                             IAllocator::TypePrimitive,
                             cs);
    }
    file->SyncSeekRead(buf, length, OriginBegin, x50_cachedResInfo->x4_offset + offset);
    return new CMemoryInStream((atUint8*)buf, length, !extBuf);
}

void CResLoader::LoadMemResourceSync(const SObjectTag& tag, void** bufOut, int* sizeOut)
{
    CPakFile* file = FindResourceForLoad(tag);
    CCallStack cs(AT_PRETTY_FUNCTION, "UnknownType");
    void* buf = CMemory::Alloc(x50_cachedResInfo->x8_size,
                               IAllocator::HintLarge,
                               IAllocator::ScopeDefault,
                               IAllocator::TypePrimitive,
                               cs);
    file->SyncSeekRead(buf, x50_cachedResInfo->x8_size, OriginBegin,
                       x50_cachedResInfo->x4_offset);
    *bufOut = buf;
    *sizeOut = x50_cachedResInfo->x8_size;
}

CInputStream* CResLoader::LoadResourceFromMemorySync(const SObjectTag& tag, const void* buf)
{
    FindResourceForLoad(tag);
    CInputStream* newStrm = new CMemoryInStream((atUint8*)buf, x50_cachedResInfo->x8_size);
    if (x50_cachedResInfo->xb_compressed)
    {
        newStrm->readUint32Big();
        newStrm = new CZipInputStream(std::move(std::unique_ptr<CInputStream>(newStrm)));
    }
    return newStrm;
}

CInputStream* CResLoader::LoadNewResourceSync(const SObjectTag& tag, void* extBuf)
{
    void* buf = extBuf;
    CPakFile* file = FindResourceForLoad(tag);
    size_t resSz = ROUND_UP_32(x50_cachedResInfo->x8_size);
    if (!buf)
    {
        CCallStack cs(AT_PRETTY_FUNCTION, "UnknownType");
        buf = CMemory::Alloc(resSz,
                             IAllocator::HintLarge,
                             IAllocator::ScopeDefault,
                             IAllocator::TypePrimitive,
                             cs);
    }
    file->SyncSeekRead(buf, resSz, OriginBegin, x50_cachedResInfo->x4_offset);
    CInputStream* newStrm = new CMemoryInStream((atUint8*)buf, resSz, !extBuf);
    if (x50_cachedResInfo->xb_compressed)
    {
        newStrm->readUint32Big();
        newStrm = new CZipInputStream(std::move(std::unique_ptr<CInputStream>(newStrm)));
    }
    return newStrm;
}

CDvdRequest* CResLoader::LoadResourcePartAsync(const SObjectTag& tag, int offset, int length, void* buf)
{
    return FindResourceForLoad(tag.id)->AsyncSeekRead(buf, length,
                                                      OriginBegin, x50_cachedResInfo->x4_offset + offset);
}

CDvdRequest* CResLoader::LoadResourceAsync(const SObjectTag& tag, void* buf)
{
    return FindResourceForLoad(tag.id)->AsyncSeekRead(buf, ROUND_UP_32(x50_cachedResInfo->x8_size),
                                                      OriginBegin, x50_cachedResInfo->x4_offset);
}

bool CResLoader::GetResourceCompression(const SObjectTag& tag)
{
    if (FindResource(tag.id))
        return x50_cachedResInfo->xb_compressed;
    return false;
}

u32 CResLoader::ResourceSize(const SObjectTag& tag)
{
    if (FindResource(tag.id))
        return x50_cachedResInfo->x8_size;
    return false;
}

bool CResLoader::ResourceExists(const SObjectTag& tag)
{
    return FindResource(tag.id);
}

FourCC CResLoader::GetResourceTypeById(u32 id)
{
    if (FindResource(id))
        return x50_cachedResInfo->x0_type;
    return false;
}

const SObjectTag* CResLoader::GetResourceIdByName(const char* name) const
{
    for (const std::unique_ptr<CPakFile>& file : x1c_pakLoadedList)
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
    for (auto it=x34_pakLoadingList.begin();
         it != x34_pakLoadingList.end();
         ++it)
    {
        (*it)->AsyncIdle();
        if ((*it)->x2c_asyncLoadPhase == CPakFile::PakAsyncLoaded)
        {
            MoveToCorrectLoadedList(std::move(*it));
            it = x34_pakLoadingList.erase(it);
            --x44_pakLoadingCount;
        }
    }
}

bool CResLoader::FindResource(u32 id)
{
    for (const std::unique_ptr<CPakFile>& file : x1c_pakLoadedList)
        if (CacheFromPak(*file, id))
            return true;
    return false;
}

CPakFile* CResLoader::FindResourceForLoad(u32 id)
{
    for (std::unique_ptr<CPakFile>& file : x1c_pakLoadedList)
        if (CacheFromPakForLoad(*file, id))
            return file.get();
    return nullptr;
}

CPakFile* CResLoader::FindResourceForLoad(const SObjectTag& tag)
{
    return FindResourceForLoad(tag.id);
}

bool CResLoader::CacheFromPakForLoad(CPakFile& file, u32 id)
{
    const CPakFile::SResInfo* info = file.GetResInfoForLoad(id);
    if (info)
    {
        x4c_cachedResId = id;
        x50_cachedResInfo = info;
        return true;
    }
    return false;
}

bool CResLoader::CacheFromPak(const CPakFile& file, u32 id)
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
    x1c_pakLoadedList.push_back(std::move(file));
}

}
