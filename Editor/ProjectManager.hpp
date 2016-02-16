#ifndef URDE_PROJECT_MANAGER_HPP
#define URDE_PROJECT_MANAGER_HPP

#include <HECL/Database.hpp>
#include <Athena/DNAYaml.hpp>
#include "ProjectResourceFactory.hpp"
#include "Runtime/CSimplePool.hpp"

namespace URDE
{
class ViewManager;

#ifdef URDE_BINARY_CONFIGS
using ConfigReader = Athena::io::IStreamReader;
using ConfigWriter = Athena::io::IStreamWriter;
#else
using ConfigReader = Athena::io::YAMLDocReader;
using ConfigWriter = Athena::io::YAMLDocWriter;
#endif

class ProjectManager
{
    ViewManager& m_vm;
    std::unique_ptr<HECL::Database::Project> m_proj;
    static bool m_registeredSpecs;
    ProjectResourceFactory m_factory;
    pshag::CSimplePool m_objStore;

    void IndexMP1Resources();

public:
    ProjectManager(ViewManager& vm);
    operator bool() const {return m_proj.operator bool();}

    HECL::Database::Project* project() {return m_proj.get();}

    bool newProject(const HECL::SystemString& path);
    bool openProject(const HECL::SystemString& path);
    bool extractGame(const HECL::SystemString& path);

    bool saveProject();

};

}

#endif // URDE_PROJECT_MANAGER_HPP
