#ifndef RUDE_PROJECT_MANAGER_HPP
#define RUDE_PROJECT_MANAGER_HPP

#include <HECL/Database.hpp>

namespace RUDE
{
class ViewManager;

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

};

}

#endif // RUDE_PROJECT_MANAGER_HPP
