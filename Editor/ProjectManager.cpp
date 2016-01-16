#include "ProjectManager.hpp"
#include "ViewManager.hpp"
#include "../DataSpecRegistry.hpp"

namespace URDE
{
static LogVisor::LogModule Log("URDE::ProjectManager");

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

    HECL::MakeDir(path.c_str());
    m_proj.reset(new HECL::Database::Project(path));
    if (!*m_proj)
    {
        m_proj.reset();
        return false;
    }

    m_vm.SetupEditorView();
    saveProject();
    m_vm.m_mainWindow->setTitle(m_proj->getProjectRootPath().getLastComponent());
    m_vm.DismissSplash();
    m_vm.FadeInEditors();

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

#ifdef URDE_BINARY_CONFIGS
    HECL::ProjectPath urdeSpacesPath(*m_proj, _S(".hecl/urde_spaces.bin"));
    Athena::io::FileReader r(urdeSpacesPath.getAbsolutePath(), 32 * 1024, false);
    if (r.hasError())
        goto makeDefault;

    m_vm.SetupEditorView(r);

#else
    HECL::ProjectPath urdeSpacesPath(*m_proj, _S(".hecl/urde_spaces.yaml"));
    FILE* fp = HECL::Fopen(urdeSpacesPath.getAbsolutePath().c_str(), _S("r"));

    Athena::io::YAMLDocReader r;
    if (!fp)
        goto makeDefault;

    yaml_parser_set_input_file(r.getParser(), fp);
    if (!r.ValidateClassType(r.getParser(), "UrdeSpacesState"))
    {
        fclose(fp);
        goto makeDefault;
    }

    r.reset();
    fseek(fp, 0, SEEK_SET);
    yaml_parser_set_input_file(r.getParser(), fp);
    if (!r.parse())
    {
        fclose(fp);
        goto makeDefault;
    }
    fclose(fp);

    m_vm.SetupEditorView(r);

#endif

    m_vm.m_mainWindow->setTitle(m_proj->getProjectRootPath().getLastComponent());
    m_vm.DismissSplash();
    m_vm.FadeInEditors();

    m_vm.pushRecentProject(m_proj->getProjectRootPath().getAbsolutePath());

    return true;

makeDefault:
    m_vm.SetupEditorView();
    saveProject();

    m_vm.m_mainWindow->setTitle(m_proj->getProjectRootPath().getLastComponent());
    m_vm.DismissSplash();
    m_vm.FadeInEditors();
    return true;
}

bool ProjectManager::extractGame(const HECL::SystemString& path)
{
    return false;
}

bool ProjectManager::saveProject()
{
    if (!m_proj)
        return false;

#ifdef URDE_BINARY_CONFIGS
    HECL::ProjectPath oldSpacesPath(*m_proj, _S(".hecl/~urde_spaces.bin"));
    Athena::io::FileWriter w(oldSpacesPath.getAbsolutePath(), true, false);
    if (w.hasError())
        return false;

    m_vm.SaveEditorView(w);
    w.close();

    HECL::ProjectPath newSpacesPath(*m_proj, _S(".hecl/urde_spaces.bin"));

#else
    HECL::ProjectPath oldSpacesPath(*m_proj, _S(".hecl/~urde_spaces.yaml"));
    FILE* fp = HECL::Fopen(oldSpacesPath.getAbsolutePath().c_str(), _S("w"));
    if (!fp)
        return false;

    Athena::io::YAMLDocWriter w("UrdeSpacesState");
    yaml_emitter_set_output_file(w.getEmitter(), fp);
    if (!w.open())
    {
        fclose(fp);
        return false;
    }

    m_vm.SaveEditorView(w);
    if (!w.finish())
    {
        fclose(fp);
        return false;
    }

    w.close();
    fclose(fp);

    HECL::ProjectPath newSpacesPath(*m_proj, _S(".hecl/urde_spaces.yaml"));

#endif

    HECL::Unlink(newSpacesPath.getAbsolutePath().c_str());
    HECL::Rename(oldSpacesPath.getAbsolutePath().c_str(),
                 newSpacesPath.getAbsolutePath().c_str());

    m_vm.pushRecentProject(m_proj->getProjectRootPath().getAbsolutePath());

    return true;
}

}
