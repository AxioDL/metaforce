#pragma once

#include <functional>
#include <memory>
#include <vector>

#include "Runtime/RetroTypes.hpp"

namespace urde {
class CFactoryMgr;
class CObjectReference;
class CResLoader;
class CSimplePool;
class CVParamTransfer;
class IDvdRequest;
class IObj;

using CFactoryFnReturn = std::unique_ptr<IObj>;
using FFactoryFunc = std::function<CFactoryFnReturn(const urde::SObjectTag& tag, urde::CInputStream& in,
                                                    const urde::CVParamTransfer& vparms, CObjectReference* selfRef)>;
using FMemFactoryFunc = std::function<CFactoryFnReturn(const urde::SObjectTag& tag, std::unique_ptr<u8[]>&& in, u32 len,
                                                       const urde::CVParamTransfer& vparms, CObjectReference* selfRef)>;

class IFactory {
public:
  virtual ~IFactory() = default;
  virtual CFactoryFnReturn Build(const SObjectTag&, const CVParamTransfer&, CObjectReference*) = 0;
  virtual void BuildAsync(const SObjectTag&, const CVParamTransfer&, std::unique_ptr<IObj>*, CObjectReference*) = 0;
  virtual void CancelBuild(const SObjectTag&) = 0;
  virtual bool CanBuild(const SObjectTag&) = 0;
  virtual const SObjectTag* GetResourceIdByName(std::string_view) const = 0;
  virtual FourCC GetResourceTypeById(CAssetId id) const = 0;
  virtual void EnumerateResources(const std::function<bool(const SObjectTag&)>& lambda) const = 0;
  virtual void
  EnumerateNamedResources(const std::function<bool(std::string_view, const SObjectTag&)>& lambda) const = 0;
  virtual CResLoader* GetResLoader() { return nullptr; }
  virtual CFactoryMgr* GetFactoryMgr() { return nullptr; }
  virtual void LoadOriginalIDs(CSimplePool& sp) {}
  virtual void AsyncIdle() {}

  /* Non-factory versions, replaces CResLoader */
  virtual u32 ResourceSize(const urde::SObjectTag& tag) = 0;
  virtual std::shared_ptr<IDvdRequest> LoadResourceAsync(const urde::SObjectTag& tag, void* target) = 0;
  virtual std::shared_ptr<IDvdRequest> LoadResourcePartAsync(const urde::SObjectTag& tag, u32 off, u32 size,
                                                             void* target) = 0;
  virtual std::unique_ptr<u8[]> LoadResourceSync(const urde::SObjectTag& tag) = 0;
  virtual std::unique_ptr<u8[]> LoadNewResourcePartSync(const urde::SObjectTag& tag, u32 off, u32 size) = 0;
  virtual void GetTagListForFile(const char* pakName, std::vector<SObjectTag>& out) const {}

  virtual CAssetId TranslateOriginalToNew(CAssetId id) const { return {}; }
  virtual CAssetId TranslateNewToOriginal(CAssetId id) const { return {}; }
};

} // namespace urde
