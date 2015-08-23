#ifndef __RETRO_CRESLOADER_HPP__
#define __RETRO_CRESLOADER_HPP__

#include <memory>
#include <string>
#include "RetroTypes.hpp"
#include "CPakFile.hpp"
#include "CBasics.hpp"

namespace Retro
{
class SObjectTag;
class CDvdRequest;

class CResLoader
{
    //std::list<std::unique_ptr<CPakFile>> x4_unusedList;
    std::list<std::unique_ptr<CPakFile>> x1c_pakLoadedList;
    std::list<std::unique_ptr<CPakFile>> x34_pakLoadingList;
    u32 x44_pakLoadingCount = 0;
    u32 x4c_cachedResId = -1;
    const CPakFile::SResInfo* x50_cachedResInfo = nullptr;
public:
    const std::vector<u32>& GetTagListForFile(const std::string&) const;
    void AddPakFileAsync(const std::string&, bool);
    void AddPakFile(const std::string&, bool);
    CInputStream* LoadNewResourcePartSync(const SObjectTag&, int, int, char*);
    void LoadMemResourceSync(const SObjectTag&, char*, int*);
    CInputStream* LoadResourceFromMemorySync(const SObjectTag&, const void*);
    CInputStream* LoadNewResourceSync(const SObjectTag&, char*);

    CDvdRequest* LoadResourcePartAsync(const SObjectTag& tag, int offset, int length, void* buf)
    {
        return FindResourceForLoad(tag.id)->AsyncSeekRead(buf, length,
                                                          OriginBegin, x50_cachedResInfo->offset + offset);
    }

    CDvdRequest* LoadResourceAsync(const SObjectTag& tag, void* buf)
    {
        return FindResourceForLoad(tag.id)->AsyncSeekRead(buf, ROUND_UP_32(x50_cachedResInfo->size),
                                                          OriginBegin, x50_cachedResInfo->offset);
    }

    bool GetResourceCompression(const SObjectTag& tag)
    {
        if (FindResource(tag.id))
            return x50_cachedResInfo->compressed;
        return false;
    }

    u32 ResourceSize(const SObjectTag& tag)
    {
        if (FindResource(tag.id))
            return x50_cachedResInfo->size;
        return false;
    }

    bool ResourceExists(const SObjectTag& tag)
    {
        return FindResource(tag.id);
    }

    FourCC GetResourceTypeById(u32 id)
    {
        if (FindResource(id))
            return x50_cachedResInfo->tag.type;
        return false;
    }

    u32 GetResourceIdByName(const char* name) const
    {
        for (const std::unique_ptr<CPakFile>& file : x1c_pakLoadedList)
        {
            u32 id = file->GetResIdByName(name);
            if (id)
                return id;
        }
        return 0;
    }

    bool AreAllPaksLoaded() const
    {
        return x44_pakLoadingCount == 0;
    }

    void AsyncIdlePakLoading()
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

    bool FindResource(u32 id)
    {
        for (const std::unique_ptr<CPakFile>& file : x1c_pakLoadedList)
            if (CacheFromPak(*file, id))
                return true;
        return false;
    }

    CPakFile* FindResourceForLoad(u32 id)
    {
        for (std::unique_ptr<CPakFile>& file : x1c_pakLoadedList)
            if (CacheFromPakForLoad(*file, id))
                return file.get();
        return nullptr;
    }

    CPakFile* FindResourceForLoad(const SObjectTag& tag)
    {
        return FindResourceForLoad(tag.id);
    }

    bool CacheFromPakForLoad(CPakFile& file, u32 id)
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

    bool CacheFromPak(const CPakFile& file, u32 id)
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

    void MoveToCorrectLoadedList(std::unique_ptr<CPakFile>&& file)
    {
        x1c_pakLoadedList.push_back(std::move(file));
    }
};

}

#endif // __RETRO_CRESLOADER_HPP__
