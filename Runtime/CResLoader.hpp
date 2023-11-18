#pragma once

#include <functional>
#include <list>
#include <memory>
#include <string>
#include <vector>

#include "Runtime/CPakFile.hpp"
#include "Runtime/Streams/IOStreams.hpp"
#include "Runtime/RetroTypes.hpp"

namespace metaforce {
class IDvdRequest;
struct SObjectTag;

class CResLoader {
  std::string m_loaderPath;
  // std::list<std::unique_ptr<CPakFile>> x0_aramList;
  std::list<std::unique_ptr<CPakFile>> x18_pakLoadedList;
  std::list<std::unique_ptr<CPakFile>> x30_pakLoadingList;
  std::list<std::unique_ptr<CPakFile>>
      m_overridePakList; // URDE Addition, Trilogy has a similar mechanism, need to verify behavior against it
  u32 x44_pakLoadingCount = 0;
  std::list<std::unique_ptr<CPakFile>>::iterator x48_curPak;
  mutable CAssetId x4c_cachedResId;
  mutable const CPakFile::SResInfo* x50_cachedResInfo = nullptr;
  bool x54_forwardSeek = false;

  bool _GetTagListForFile(std::vector<SObjectTag>& out, const std::string& path,
                          const std::unique_ptr<CPakFile>& file) const;

public:
  CResLoader();
  const std::vector<CAssetId>* GetTagListForFile(std::string_view name) const;
  void AddPakFileAsync(std::string_view name, bool buildDepList, bool worldPak, bool override = false);
  void AddPakFile(std::string_view name, bool samusPak, bool worldPak, bool override = false);
  void WaitForPakFileLoadingComplete();
  std::unique_ptr<CInputStream> LoadNewResourcePartSync(const SObjectTag& tag, u32 length, u32 offset, void* extBuf);
  void LoadMemResourceSync(const SObjectTag& tag, std::unique_ptr<u8[]>& bufOut, int* sizeOut);
  std::unique_ptr<CInputStream> LoadResourceFromMemorySync(const SObjectTag& tag, const void* buf);
  std::unique_ptr<CInputStream> LoadNewResourceSync(const SObjectTag& tag, void* extBuf = nullptr);
  std::shared_ptr<IDvdRequest> LoadResourcePartAsync(const SObjectTag& tag, u32 off, u32 size, void* buf);
  std::shared_ptr<IDvdRequest> LoadResourceAsync(const SObjectTag& tag, void* buf);
  std::unique_ptr<u8[]> LoadResourceSync(const metaforce::SObjectTag& tag);
  std::unique_ptr<u8[]> LoadNewResourcePartSync(const metaforce::SObjectTag& tag, u32 off, u32 size);
  void GetTagListForFile(const char* pakName, std::vector<SObjectTag>& out) const;
  bool GetResourceCompression(const SObjectTag& tag) const;
  u32 ResourceSize(const SObjectTag& tag) const;
  bool ResourceExists(const SObjectTag& tag) const;
  FourCC GetResourceTypeById(CAssetId id) const;
  const SObjectTag* GetResourceIdByName(std::string_view name) const;
  bool AreAllPaksLoaded() const;
  void AsyncIdlePakLoading();
  bool FindResource(CAssetId id) const;
  CPakFile* FindResourceForLoad(CAssetId id);
  CPakFile* FindResourceForLoad(const SObjectTag& tag);
  bool CacheFromPakForLoad(CPakFile& file, CAssetId id);
  bool CacheFromPak(const CPakFile& file, CAssetId id) const;
  void MoveToCorrectLoadedList(std::unique_ptr<CPakFile>&& file);
  std::vector<std::pair<std::string, SObjectTag>> GetResourceIdToNameList() const;
  void EnumerateResources(const std::function<bool(const SObjectTag&)>& lambda) const;
  void EnumerateNamedResources(const std::function<bool(std::string_view, const SObjectTag&)>& lambda) const;
  const std::list<std::unique_ptr<CPakFile>>& GetPaks() const { return x18_pakLoadedList; }
};

} // namespace metaforce
