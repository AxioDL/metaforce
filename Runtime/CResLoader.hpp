#ifndef __PSHAG_CRESLOADER_HPP__
#define __PSHAG_CRESLOADER_HPP__

#include <memory>
#include <string>
#include "RetroTypes.hpp"
#include "CPakFile.hpp"
#include "IOStreams.hpp"

namespace urde
{
struct SObjectTag;
class IDvdRequest;

class CResLoader
{
    std::string m_loaderPath;
    //std::list<std::unique_ptr<CPakFile>> x4_unusedList;
    std::list<std::unique_ptr<CPakFile>> x1c_pakLoadedList;
    std::list<std::unique_ptr<CPakFile>> x34_pakLoadingList;
    u32 x44_pakLoadingCount = 0;
    u32 x4c_cachedResId = -1;
    const CPakFile::SResInfo* x50_cachedResInfo = nullptr;
public:
    const std::vector<u32>* GetTagListForFile(const std::string& name) const;
    void AddPakFileAsync(const std::string& name, bool flag);
    void AddPakFile(const std::string& name, bool flag);
    CInputStream* LoadNewResourcePartSync(const SObjectTag& tag, int offset, int length, void* extBuf);
    void LoadMemResourceSync(const SObjectTag& tag, void** bufOut, int* sizeOut);
    CInputStream* LoadResourceFromMemorySync(const SObjectTag& tag, const void* buf);
    CInputStream* LoadNewResourceSync(const SObjectTag& tag, void* extBuf=nullptr);
    std::shared_ptr<IDvdRequest> LoadResourcePartAsync(const SObjectTag& tag, int offset, int length, void* buf);
    std::shared_ptr<IDvdRequest> LoadResourceAsync(const SObjectTag& tag, void* buf);
    bool GetResourceCompression(const SObjectTag& tag);
    u32 ResourceSize(const SObjectTag& tag);
    bool ResourceExists(const SObjectTag& tag);
    FourCC GetResourceTypeById(u32 id);
    const SObjectTag* GetResourceIdByName(const char* name) const;
    bool AreAllPaksLoaded() const;
    void AsyncIdlePakLoading();
    bool FindResource(u32 id);
    CPakFile* FindResourceForLoad(u32 id);
    CPakFile* FindResourceForLoad(const SObjectTag& tag);
    bool CacheFromPakForLoad(CPakFile& file, u32 id);
    bool CacheFromPak(const CPakFile& file, u32 id);
    void MoveToCorrectLoadedList(std::unique_ptr<CPakFile>&& file);
};

}

#endif // __PSHAG_CRESLOADER_HPP__
