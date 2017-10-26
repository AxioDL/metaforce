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
    std::list<std::unique_ptr<CPakFile>>::iterator x48_curPak;
    mutable CAssetId x4c_cachedResId;
    mutable const CPakFile::SResInfo* x50_cachedResInfo = nullptr;
    bool x54_forwardSeek = false;
public:
    CResLoader();
    const std::vector<CAssetId>* GetTagListForFile(const std::string& name) const;
    void AddPakFileAsync(const std::string& name, bool samusPak, bool worldPak);
    void AddPakFile(const std::string& name, bool samusPak, bool worldPak);
    std::unique_ptr<CInputStream> LoadNewResourcePartSync(const SObjectTag& tag, int offset, int length, void* extBuf);
    void LoadMemResourceSync(const SObjectTag& tag, std::unique_ptr<u8[]>& bufOut, int* sizeOut);
    std::unique_ptr<CInputStream> LoadResourceFromMemorySync(const SObjectTag& tag, const void* buf);
    std::unique_ptr<CInputStream> LoadNewResourceSync(const SObjectTag& tag, void* extBuf=nullptr);
    std::shared_ptr<IDvdRequest> LoadResourcePartAsync(const SObjectTag& tag, int offset, int length, void* buf);
    std::shared_ptr<IDvdRequest> LoadResourceAsync(const SObjectTag& tag, void* buf);
    bool GetResourceCompression(const SObjectTag& tag);
    u32 ResourceSize(const SObjectTag& tag);
    bool ResourceExists(const SObjectTag& tag);
    FourCC GetResourceTypeById(CAssetId id) const;
    const SObjectTag* GetResourceIdByName(const char* name) const;
    bool AreAllPaksLoaded() const;
    void AsyncIdlePakLoading();
    bool FindResource(CAssetId id) const;
    CPakFile* FindResourceForLoad(CAssetId id);
    CPakFile* FindResourceForLoad(const SObjectTag& tag);
    bool CacheFromPakForLoad(CPakFile& file, CAssetId id);
    bool CacheFromPak(const CPakFile& file, CAssetId id) const;
    void MoveToCorrectLoadedList(std::unique_ptr<CPakFile>&& file);
};

}

#endif // __URDE_CRESLOADER_HPP__
