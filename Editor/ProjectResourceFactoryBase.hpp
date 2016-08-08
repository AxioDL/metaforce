#ifndef URDE_PROJECT_RESOURCE_FACTORY_BASE_HPP
#define URDE_PROJECT_RESOURCE_FACTORY_BASE_HPP

#include "hecl/ClientProcess.hpp"
#include "hecl/Database.hpp"
#include "Runtime/IFactory.hpp"
#include "Runtime/CFactoryMgr.hpp"
#include "Runtime/CResFactory.hpp"
#include "optional.hpp"

#include <thread>
#include <mutex>

namespace urde
{

class ProjectResourceFactoryBase : public urde::IFactory
{
    friend class ProjectResourcePool;
    hecl::ClientProcess& m_clientProc;

public:
    struct AsyncTask
    {
        ProjectResourceFactoryBase& m_parent;

        SObjectTag x0_tag;
        //IDvdRequest* x8_dvdReq = nullptr;
        std::unique_ptr<u8[]>* xc_targetDataPtr = nullptr;
        IObj** xc_targetObjPtr = nullptr;
        std::unique_ptr<u8[]> x10_loadBuffer;
        u32 x14_resSize = UINT32_MAX;
        u32 x14_resOffset = 0;
        CVParamTransfer x18_cvXfer;

        hecl::ProjectPath m_workingPath;
        hecl::ProjectPath m_cookedPath;
        std::shared_ptr<const hecl::ClientProcess::CookTransaction> m_cookTransaction;
        std::shared_ptr<const hecl::ClientProcess::BufferTransaction> m_bufTransaction;
        bool m_failed = false;
        bool m_complete = false;

        AsyncTask(ProjectResourceFactoryBase& parent, const SObjectTag& tag,
                  std::unique_ptr<u8[]>& ptr)
        : m_parent(parent), x0_tag(tag), xc_targetDataPtr(&ptr) {}

        AsyncTask(ProjectResourceFactoryBase& parent, const SObjectTag& tag,
                  std::unique_ptr<u8[]>& ptr, u32 size, u32 off)
        : m_parent(parent), x0_tag(tag), xc_targetDataPtr(&ptr), x14_resSize(size),
          x14_resOffset(off) {}

        AsyncTask(ProjectResourceFactoryBase& parent, const SObjectTag& tag,
                  IObj** ptr, const CVParamTransfer& xfer)
        : m_parent(parent), x0_tag(tag), xc_targetObjPtr(ptr), x18_cvXfer(xfer) {}

        void EnsurePath(const urde::SObjectTag& tag,
                        const hecl::ProjectPath& path);
        void CookComplete();
        bool AsyncPump();
    };

protected:
    std::unordered_map<urde::SObjectTag, hecl::ProjectPath> m_tagToPath;
    std::unordered_map<std::string, urde::SObjectTag> m_catalogNameToTag;
    void Clear();

    const hecl::Database::Project* m_proj = nullptr;
    const hecl::Database::DataSpecEntry* m_origSpec = nullptr;
    const hecl::Database::DataSpecEntry* m_pcSpec = nullptr;
    /* Used to resolve cooked paths */
    std::unique_ptr<hecl::Database::IDataSpec> m_cookSpec;
    urde::CFactoryMgr m_factoryMgr;

    hecl::BlenderToken m_backgroundBlender;
    std::thread m_backgroundIndexTh;
    std::mutex m_backgroundIndexMutex;
    bool m_backgroundRunning = false;

    std::unordered_map<SObjectTag, std::shared_ptr<AsyncTask>> m_asyncLoadList;

    bool WaitForTagReady(const urde::SObjectTag& tag, const hecl::ProjectPath*& pathOut);
    bool
    PrepForReadSync(const SObjectTag& tag,
                    const hecl::ProjectPath& path,
                    std::experimental::optional<athena::io::FileReader>& fr);

    virtual SObjectTag TagFromPath(const hecl::ProjectPath& path, hecl::BlenderToken& btok) const=0;

    void ReadCatalog(const hecl::ProjectPath& catalogPath,
                     athena::io::YAMLDocWriter& nameWriter);
    void BackgroundIndexRecursiveProc(const hecl::ProjectPath& path,
                                      athena::io::YAMLDocWriter& cacheWriter,
                                      athena::io::YAMLDocWriter& nameWriter,
                                      int level);
    void BackgroundIndexRecursiveCatalogs(const hecl::ProjectPath& path,
                                          athena::io::YAMLDocWriter& nameWriter,
                                          int level);
    void BackgroundIndexProc();
    void CancelBackgroundIndex();
    void BeginBackgroundIndex(hecl::Database::Project& proj,
                              const hecl::Database::DataSpecEntry& origSpec,
                              const hecl::Database::DataSpecEntry& pcSpec);

    hecl::ProjectPath GetCookedPath(const hecl::ProjectPath& working, bool pcTarget) const;
    bool SyncCook(const hecl::ProjectPath& working);
    CFactoryFnReturn BuildSync(const SObjectTag& tag, const hecl::ProjectPath& path,
                               const CVParamTransfer& paramXfer);

public:
    ProjectResourceFactoryBase(hecl::ClientProcess& clientProc) : m_clientProc(clientProc) {}
    std::unique_ptr<urde::IObj> Build(const urde::SObjectTag&, const urde::CVParamTransfer&);
    void BuildAsync(const urde::SObjectTag&, const urde::CVParamTransfer&, urde::IObj**);
    std::shared_ptr<AsyncTask> BuildAsyncInternal(const urde::SObjectTag&, const urde::CVParamTransfer&, urde::IObj**);
    void CancelBuild(const urde::SObjectTag&);
    bool CanBuild(const urde::SObjectTag&);
    const urde::SObjectTag* GetResourceIdByName(const char*) const;
    FourCC GetResourceTypeById(ResId id) const;

    u32 ResourceSize(const SObjectTag& tag);
    std::shared_ptr<AsyncTask> LoadResourceAsync(const urde::SObjectTag& tag, std::unique_ptr<u8[]>& target);
    std::shared_ptr<AsyncTask> LoadResourcePartAsync(const urde::SObjectTag& tag, u32 size, u32 off, std::unique_ptr<u8[]>& target);
    std::unique_ptr<u8[]> LoadResourceSync(const urde::SObjectTag& tag);
    std::unique_ptr<u8[]> LoadResourcePartSync(const urde::SObjectTag& tag, u32 size, u32 off);

    void AsyncIdle();
    void Shutdown() {CancelBackgroundIndex();}

    SObjectTag TagFromPath(const hecl::SystemChar* path) const
    {
        return TagFromPath(hecl::ProjectPath(*(hecl::Database::Project*)m_proj, path),
                           hecl::SharedBlenderToken);
    }

    ~ProjectResourceFactoryBase() {Shutdown();}
};

}

#endif // URDE_PROJECT_RESOURCE_FACTORY_BASE_HPP
