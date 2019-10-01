#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <optional>
#include <thread>
#include <unordered_map>

#include "DataSpec/SpecBase.hpp"

#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/CResFactory.hpp"
#include "Runtime/IFactory.hpp"

#include <hecl/ClientProcess.hpp>
#include <hecl/Database.hpp>

namespace urde {

class ProjectResourceFactoryBase : public IFactory {
  friend class ProjectResourcePool;
  hecl::ClientProcess& m_clientProc;

public:
  struct AsyncTask : urde::IDvdRequest {
    ProjectResourceFactoryBase& m_parent;

    SObjectTag x0_tag;
    // IDvdRequest* x8_dvdReq = nullptr;
    std::unique_ptr<u8[]>* xc_targetDataPtr = nullptr;
    u8* xc_targetDataRawPtr = nullptr;
    std::unique_ptr<IObj>* xc_targetObjPtr = nullptr;
    std::unique_ptr<u8[]> x10_loadBuffer;
    u32 x14_resSize = UINT32_MAX;
    u32 x14_resOffset = 0;
    CVParamTransfer x18_cvXfer;
    CObjectReference* m_selfRef = nullptr;

    hecl::ProjectPath m_workingPath;
    hecl::ProjectPath m_cookedPath;
    std::shared_ptr<const hecl::ClientProcess::CookTransaction> m_cookTransaction;
    std::shared_ptr<const hecl::ClientProcess::BufferTransaction> m_bufTransaction;
    bool m_failed = false;
    bool m_complete = false;

    AsyncTask(ProjectResourceFactoryBase& parent, const SObjectTag& tag, std::unique_ptr<u8[]>& ptr)
    : m_parent(parent), x0_tag(tag), xc_targetDataPtr(&ptr) {}

    AsyncTask(ProjectResourceFactoryBase& parent, const SObjectTag& tag, std::unique_ptr<u8[]>& ptr, u32 size, u32 off)
    : m_parent(parent), x0_tag(tag), xc_targetDataPtr(&ptr), x14_resSize(size), x14_resOffset(off) {}

    AsyncTask(ProjectResourceFactoryBase& parent, const SObjectTag& tag, u8* ptr)
    : m_parent(parent), x0_tag(tag), xc_targetDataRawPtr(ptr) {}

    AsyncTask(ProjectResourceFactoryBase& parent, const SObjectTag& tag, u8* ptr, u32 size, u32 off)
    : m_parent(parent), x0_tag(tag), xc_targetDataRawPtr(ptr), x14_resSize(size), x14_resOffset(off) {}

    AsyncTask(ProjectResourceFactoryBase& parent, const SObjectTag& tag, std::unique_ptr<IObj>* ptr,
              const CVParamTransfer& xfer, CObjectReference* selfRef)
    : m_parent(parent), x0_tag(tag), xc_targetObjPtr(ptr), x18_cvXfer(xfer), m_selfRef(selfRef) {}

    /* Cook only */
    AsyncTask(ProjectResourceFactoryBase& parent, const SObjectTag& tag) : m_parent(parent), x0_tag(tag) {}

    void EnsurePath(const urde::SObjectTag& tag, const hecl::ProjectPath& path);
    void CookComplete();
    bool AsyncPump();
    void WaitUntilComplete() override;
    bool IsComplete() override { return m_complete; }
    void PostCancelRequest() override {}
    EMediaType GetMediaType() const override { return EMediaType::Real; }
  };

protected:
  const hecl::Database::Project* m_proj = nullptr;
  const hecl::Database::DataSpecEntry* m_origSpec = nullptr;
  const hecl::Database::DataSpecEntry* m_pcSpec = nullptr;
  /* Used to resolve cooked paths */
  std::unique_ptr<hecl::Database::IDataSpec> m_cookSpec;
  urde::CFactoryMgr m_factoryMgr;

  std::list<std::shared_ptr<AsyncTask>> m_asyncLoadList;
  std::unordered_map<SObjectTag, std::list<std::shared_ptr<AsyncTask>>::iterator> m_asyncLoadMap;
  std::shared_ptr<AsyncTask> _AddTask(const std::shared_ptr<AsyncTask>& ptr);
  std::list<std::shared_ptr<AsyncTask>>::iterator _RemoveTask(std::list<std::shared_ptr<AsyncTask>>::iterator it);
  std::unordered_map<SObjectTag, std::list<std::shared_ptr<AsyncTask>>::iterator>::iterator
  _RemoveTask(std::unordered_map<SObjectTag, std::list<std::shared_ptr<AsyncTask>>::iterator>::iterator it);
  std::unordered_map<SObjectTag, std::list<std::shared_ptr<AsyncTask>>::iterator>::iterator
  _RemoveTask(const SObjectTag& tag);

  static AsyncTask& _GetAsyncTask(std::list<std::shared_ptr<AsyncTask>>::iterator it) { return **it; }
  static AsyncTask&
  _GetAsyncTask(std::unordered_map<SObjectTag, std::list<std::shared_ptr<AsyncTask>>::iterator>::iterator it) {
    return **it->second;
  }

  bool PrepForReadSync(const SObjectTag& tag, const hecl::ProjectPath& path,
                       std::optional<athena::io::FileReader>& fr);

  bool WaitForTagReady(const urde::SObjectTag& tag, const hecl::ProjectPath*& pathOut) {
    return static_cast<DataSpec::SpecBase&>(*m_cookSpec).waitForTagReady(tag, pathOut);
  }
  SObjectTag TagFromPath(const hecl::ProjectPath& path) const {
    return static_cast<DataSpec::SpecBase&>(*m_cookSpec).tagFromPath(path);
  }
  SObjectTag BuildTagFromPath(const hecl::ProjectPath& path) const {
    return static_cast<DataSpec::SpecBase&>(*m_cookSpec).buildTagFromPath(path);
  }
  void GetTagListForFile(const char* pakName, std::vector<SObjectTag>& out) const override {
    return static_cast<DataSpec::SpecBase&>(*m_cookSpec).getTagListForFile(pakName, out);
  }
  void CancelBackgroundIndex() {
    if (m_cookSpec)
      return static_cast<DataSpec::SpecBase&>(*m_cookSpec).cancelBackgroundIndex();
  }
  void BeginBackgroundIndex(hecl::Database::Project& proj, const hecl::Database::DataSpecEntry& origSpec,
                            const hecl::Database::DataSpecEntry& pcSpec);

  bool SyncCook(const hecl::ProjectPath& working);
  CFactoryFnReturn BuildSync(const SObjectTag& tag, const hecl::ProjectPath& path, const CVParamTransfer& paramXfer,
                             CObjectReference* selfRef);
  std::shared_ptr<AsyncTask> BuildAsyncInternal(const urde::SObjectTag&, const urde::CVParamTransfer&,
                                                std::unique_ptr<urde::IObj>*, CObjectReference* selfRef);

public:
  ProjectResourceFactoryBase(hecl::ClientProcess& clientProc) : m_clientProc(clientProc) {}
  std::unique_ptr<urde::IObj> Build(const urde::SObjectTag&, const urde::CVParamTransfer&,
                                    CObjectReference* selfRef) override;
  void BuildAsync(const urde::SObjectTag&, const urde::CVParamTransfer&, std::unique_ptr<urde::IObj>*,
                  CObjectReference* selfRef) override;
  void CancelBuild(const urde::SObjectTag&) override;
  bool CanBuild(const urde::SObjectTag&) override;
  const urde::SObjectTag* GetResourceIdByName(std::string_view) const override;
  FourCC GetResourceTypeById(CAssetId id) const override;
  hecl::ProjectPath GetCookedPath(const hecl::ProjectPath& working, bool pcTarget) const {
    return static_cast<DataSpec::SpecBase&>(*m_cookSpec).getCookedPath(working, pcTarget);
  }

  void EnumerateResources(const std::function<bool(const SObjectTag&)>& lambda) const override;
  void EnumerateNamedResources(const std::function<bool(std::string_view, const SObjectTag&)>& lambda) const override;

  u32 ResourceSize(const SObjectTag& tag) override;
  std::shared_ptr<urde::IDvdRequest> LoadResourceAsync(const urde::SObjectTag& tag, void* target) override;
  std::shared_ptr<urde::IDvdRequest> LoadResourcePartAsync(const urde::SObjectTag& tag, u32 off, u32 size,
                                                           void* target) override;
  std::unique_ptr<u8[]> LoadResourceSync(const urde::SObjectTag& tag) override;
  std::unique_ptr<u8[]> LoadNewResourcePartSync(const urde::SObjectTag& tag, u32 off, u32 size) override;

  std::shared_ptr<AsyncTask> CookResourceAsync(const urde::SObjectTag& tag);

  template <typename ItType>
  bool AsyncPumpTask(ItType& it);
  void AsyncIdle() override;
  void Shutdown() { CancelBackgroundIndex(); }
  bool IsBusy() const { return m_asyncLoadMap.size() != 0; }

  SObjectTag TagFromPath(hecl::SystemStringView path) const {
    return TagFromPath(hecl::ProjectPath(*(hecl::Database::Project*)m_proj, path));
  }

  ~ProjectResourceFactoryBase() override { Shutdown(); }
};

} // namespace urde
