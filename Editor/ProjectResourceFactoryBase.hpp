#ifndef URDE_PROJECT_RESOURCE_FACTORY_BASE_HPP
#define URDE_PROJECT_RESOURCE_FACTORY_BASE_HPP

#include "hecl/ClientProcess.hpp"
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
    hecl::ClientProcess& m_clientProc;
protected:
    std::unordered_map<urde::SObjectTag, hecl::ProjectPath> m_tagToPath;
    std::unordered_map<std::string, urde::SObjectTag> m_catalogNameToTag;
    void Clear();

    const hecl::Database::Project* m_proj = nullptr;
    const hecl::Database::DataSpecEntry* m_origSpec = nullptr;
    const hecl::Database::DataSpecEntry* m_pcSpec = nullptr;
    urde::CFactoryMgr m_factoryMgr;

    std::experimental::optional<hecl::BlenderConnection> m_backgroundBlender;
    std::thread m_backgroundIndexTh;
    std::mutex m_backgroundIndexMutex;
    bool m_backgroundRunning = false;

    std::unordered_map<SObjectTag, CResFactory::SLoadingData> m_asyncLoadList;

    virtual SObjectTag TagFromPath(const hecl::ProjectPath& path) const=0;

    hecl::BlenderConnection& GetBackgroundBlender() const;
    void ReadCatalog(const hecl::ProjectPath& catalogPath);
    void BackgroundIndexRecursiveProc(const hecl::SystemString& path, int level);
    void BackgroundIndexProc();
    void CancelBackgroundIndex();
    void BeginBackgroundIndex(const hecl::Database::Project& proj,
                              const hecl::Database::DataSpecEntry& origSpec,
                              const hecl::Database::DataSpecEntry& pcSpec);

    virtual hecl::ProjectPath GetCookedPath(const SObjectTag& tag,
                                            const hecl::ProjectPath& working,
                                            bool pcTarget) const=0;
    virtual bool DoCook(const SObjectTag& tag, const hecl::ProjectPath& working,
                        const hecl::ProjectPath& cooked,
                        bool pcTarget)=0;
    CFactoryFnReturn MakeObject(const SObjectTag& tag, const hecl::ProjectPath& path,
                                const CVParamTransfer& paramXfer);

public:
    ProjectResourceFactoryBase(hecl::ClientProcess& clientProc) : m_clientProc(clientProc) {}
    std::unique_ptr<urde::IObj> Build(const urde::SObjectTag&, const urde::CVParamTransfer&);
    void BuildAsync(const urde::SObjectTag&, const urde::CVParamTransfer&, urde::IObj**);
    void CancelBuild(const urde::SObjectTag&);
    bool CanBuild(const urde::SObjectTag&);
    const urde::SObjectTag* GetResourceIdByName(const char*) const;

    void AsyncIdle();
};

}

#endif // URDE_PROJECT_RESOURCE_FACTORY_BASE_HPP
