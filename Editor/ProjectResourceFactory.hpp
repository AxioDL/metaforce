#ifndef URDE_PROJECT_RESOURCE_FACTORY_HPP
#define URDE_PROJECT_RESOURCE_FACTORY_HPP

#include "Runtime/IFactory.hpp"
#include "Runtime/CFactoryMgr.hpp"

namespace URDE
{

class ProjectResourceFactory : public pshag::IFactory
{
    std::unordered_map<pshag::SObjectTag, HECL::ProjectPath> m_resPaths;
    pshag::CFactoryMgr m_factoryMgr;
    void RecursiveAddDirObjects(const HECL::ProjectPath& path);
public:
    ProjectResourceFactory();
    void BuildObjectMap(const HECL::Database::Project::ProjectDataSpec& spec);

    std::unique_ptr<pshag::IObj> Build(const pshag::SObjectTag&, const pshag::CVParamTransfer&);
    void BuildAsync(const pshag::SObjectTag&, const pshag::CVParamTransfer&, pshag::IObj**);
    void CancelBuild(const pshag::SObjectTag&);
    bool CanBuild(const pshag::SObjectTag&);
    const pshag::SObjectTag* GetResourceIdByName(const char*) const;
};

}

#endif // URDE_PROJECT_RESOURCE_FACTORY_HPP
