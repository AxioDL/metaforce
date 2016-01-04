#include "ProjectManager.hpp"
#include "ViewManager.hpp"
#include "../DataSpecRegistry.hpp"

namespace RUDE
{
static LogVisor::LogModule Log("RUDE::ProjectManager");

bool ProjectManager::m_registeredSpecs = false;
ProjectManager::ProjectManager(ViewManager &vm)
: m_vm(vm)
{
    if (!m_registeredSpecs)
    {
        HECLRegisterDataSpecs();
        m_registeredSpecs = true;
    }
}

bool ProjectManager::newProject(const HECL::SystemString& path)
{
    HECL::ProjectRootPath projPath = HECL::SearchForProject(path);
    if (projPath)
    {
        Log.report(LogVisor::Warning, _S("project already exists at '%s'"), path.c_str());
        return false;
    }

    m_proj.reset(new HECL::Database::Project(path));
    if (!*m_proj)
    {
        m_proj.reset();
        return false;
    }

    return true;
}

bool ProjectManager::openProject(const HECL::SystemString& path)
{
    HECL::ProjectRootPath projPath = HECL::SearchForProject(path);
    if (!projPath)
    {
        Log.report(LogVisor::Warning, _S("project doesn't exist at '%s'"), path.c_str());
        return false;
    }

    m_proj.reset(new HECL::Database::Project(projPath));
    if (!*m_proj)
    {
        m_proj.reset();
        return false;
    }

    HECL::ProjectPath rudeSpacesPath(*m_proj, _S(".hecl/rude_spaces.yaml"));
    if (rudeSpacesPath.getPathType() == HECL::ProjectPath::Type::File)
    {

    }

    return true;
}

bool ProjectManager::extractGame(const HECL::SystemString& path)
{
    return false;
}

}
