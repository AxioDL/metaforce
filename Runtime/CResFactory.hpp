#pragma once

#include <list>
#include <memory>
#include <unordered_map>
#include <vector>

#include "Runtime/CResLoader.hpp"
#include "Runtime/CToken.hpp"
#include "Runtime/IFactory.hpp"
#include "Runtime/IVParamObj.hpp"
#include "Runtime/MP1/MP1OriginalIDs.hpp"

namespace urde {
class IDvdRequest;
class CSimplePool;

class CResFactory : public IFactory {
  CResLoader x4_loader;
  CFactoryMgr x5c_factoryMgr;

public:
  struct SLoadingData {
    SObjectTag x0_tag;
    std::shared_ptr<IDvdRequest> x8_dvdReq;
    std::unique_ptr<IObj>* xc_targetPtr = nullptr;
    std::unique_ptr<u8[]> x10_loadBuffer;
    u32 x14_resSize = 0;
    CVParamTransfer x18_cvXfer;
    bool m_compressed = false;
    CObjectReference* m_selfRef = nullptr;

    SLoadingData() = default;
    SLoadingData(const SObjectTag& tag, std::unique_ptr<IObj>* ptr, const CVParamTransfer& xfer, bool compressed,
                 CObjectReference* selfRef)
    : x0_tag(tag), xc_targetPtr(ptr), x18_cvXfer(xfer), m_compressed(compressed), m_selfRef(selfRef) {}
  };

private:
  std::list<SLoadingData> m_loadList;
  std::unordered_map<SObjectTag, std::list<SLoadingData>::iterator> m_loadMap;
  std::vector<CToken> m_nonWorldTokens; /* URDE: always keep non-world resources resident */
  void AddToLoadList(SLoadingData&& data);
  CFactoryFnReturn BuildSync(const SObjectTag&, const CVParamTransfer&, CObjectReference* selfRef);
  bool PumpResource(SLoadingData& data);

public:
  CResLoader& GetLoader() { return x4_loader; }
  std::unique_ptr<IObj> Build(const SObjectTag&, const CVParamTransfer&, CObjectReference* selfRef) override;
  void BuildAsync(const SObjectTag&, const CVParamTransfer&, std::unique_ptr<IObj>*,
                  CObjectReference* selfRef) override;
  void AsyncIdle() override;
  void CancelBuild(const SObjectTag&) override;

  bool CanBuild(const SObjectTag& tag) override { return x4_loader.ResourceExists(tag); }

  u32 ResourceSize(const urde::SObjectTag& tag) override { return x4_loader.ResourceSize(tag); }

  std::unique_ptr<u8[]> LoadResourceSync(const urde::SObjectTag& tag) override {
    return x4_loader.LoadResourceSync(tag);
  }

  std::unique_ptr<u8[]> LoadNewResourcePartSync(const urde::SObjectTag& tag, u32 off, u32 size) override {
    return x4_loader.LoadNewResourcePartSync(tag, off, size);
  }

  void GetTagListForFile(const char* pakName, std::vector<SObjectTag>& out) const override {
    return x4_loader.GetTagListForFile(pakName, out);
  }

  std::shared_ptr<IDvdRequest> LoadResourceAsync(const urde::SObjectTag& tag, void* target) override {
    return x4_loader.LoadResourceAsync(tag, target);
  }

  std::shared_ptr<IDvdRequest> LoadResourcePartAsync(const urde::SObjectTag& tag, u32 off, u32 size,
                                                     void* target) override {
    return x4_loader.LoadResourcePartAsync(tag, off, size, target);
  }

  const SObjectTag* GetResourceIdByName(std::string_view name) const override {
    return x4_loader.GetResourceIdByName(name);
  }

  FourCC GetResourceTypeById(CAssetId id) const override { return x4_loader.GetResourceTypeById(id); }

  std::vector<std::pair<std::string, SObjectTag>> GetResourceIdToNameList() const {
    return x4_loader.GetResourceIdToNameList();
  }

  void EnumerateResources(const std::function<bool(const SObjectTag&)>& lambda) const override {
    return x4_loader.EnumerateResources(lambda);
  }

  void EnumerateNamedResources(const std::function<bool(std::string_view, const SObjectTag&)>& lambda) const override {
    return x4_loader.EnumerateNamedResources(lambda);
  }

  void LoadPersistentResources(CSimplePool& sp);
  void UnloadPersistentResources() { m_nonWorldTokens.clear(); }

  CResLoader* GetResLoader() override { return &x4_loader; }
  CFactoryMgr* GetFactoryMgr() override { return &x5c_factoryMgr; }
};

} // namespace urde
