#ifndef __URDE_CRESLOADER_HPP__
#define __URDE_CRESLOADER_HPP__

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
    //std::list<std::unique_ptr<CPakFile>> x0_aramList;
    std::list<std::unique_ptr<CPakFile>> x18_pakLoadedList;
    std::list<std::unique_ptr<CPakFile>> x30_pakLoadingList;
    u32 x44_pakLoadingCount = 0;
    u32 x4c_cachedResId = -1;
    const CPakFile::SResInfo* x50_cachedResInfo = nullptr;
public:
    const std::vector<ResId>* GetTagListForFile(const std::string& name) const;
    void AddPakFileAsync(const std::string& name, bool samusPak, bool worldPak);
    void AddPakFile(const std::string& name, bool samusPak, bool worldPak);
    CInputStream* LoadNewResourcePartSync(const SObjectTag& tag, int offset, int length, void* extBuf);
    void LoadMemResourceSync(const SObjectTag& tag, void** bufOut, int* sizeOut);
    CInputStream* LoadResourceFromMemorySync(const SObjectTag& tag, const void* buf);
    CInputStream* LoadNewResourceSync(const SObjectTag& tag, void* extBuf=nullptr);
    std::shared_ptr<IDvdRequest> LoadResourcePartAsync(const SObjectTag& tag, int offset, int length, void* buf);
    std::shared_ptr<IDvdRequest> LoadResourceAsync(const SObjectTag& tag, void* buf);
    bool GetResourceCompression(const SObjectTag& tag);
    u32 ResourceSize(const SObjectTag& tag);
    bool ResourceExists(const SObjectTag& tag);
    FourCC GetResourceTypeById(u32 id) const;
    const SObjectTag* GetResourceIdByName(const char* name) const;
    bool AreAllPaksLoaded() const;
    void AsyncIdlePakLoading();
    bool FindResource(u32 id) const;
    CPakFile* FindResourceForLoad(u32 id);
    CPakFile* FindResourceForLoad(const SObjectTag& tag);
    bool CacheFromPakForLoad(CPakFile& file, u32 id);
    bool CacheFromPak(const CPakFile& file, u32 id);
    void MoveToCorrectLoadedList(std::unique_ptr<CPakFile>&& file);
};

}

#endif // __URDE_CRESLOADER_HPP__
