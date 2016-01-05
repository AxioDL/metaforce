#ifndef RUDE_PROJECT_MANAGER_HPP
#define RUDE_PROJECT_MANAGER_HPP

#include <HECL/Database.hpp>
#include <Athena/DNAYaml.hpp>

namespace RUDE
{
class ViewManager;

#ifdef RUDE_BINARY_CONFIGS
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
public:
    ProjectManager(ViewManager& vm);
    operator bool() const {return m_proj.operator bool();}

    bool newProject(const HECL::SystemString& path);
    bool openProject(const HECL::SystemString& path);
    bool extractGame(const HECL::SystemString& path);

    bool saveProject();

};

}

#endif // RUDE_PROJECT_MANAGER_HPP
