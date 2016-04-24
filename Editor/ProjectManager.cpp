#include "ProjectManager.hpp"
#include "ViewManager.hpp"
#include "../DataSpecRegistry.hpp"

namespace urde
{
static logvisor::Module Log("URDE::ProjectManager");
ProjectManager* ProjectManager::g_SharedManager = nullptr;

CToken ProjectResourcePool::GetObj(const char* name)
{
    CToken ret = CSimplePool::GetObj(name);
    if (ret)
        return ret;

    hecl::ProjectPath path(*m_parent.project(), name);
    SObjectTag tag = static_cast<ProjectResourceFactoryBase&>(x30_factory).
                     TagFromPath(path, hecl::SharedBlenderToken);
    if (tag)
        return CSimplePool::GetObj(tag);

    return {};
}

CToken ProjectResourcePool::GetObj(const char* name, const CVParamTransfer& pvxfer)
{
    CToken ret = CSimplePool::GetObj(name, pvxfer);
    if (ret)
        return ret;

    hecl::ProjectPath path(*m_parent.project(), name);
    SObjectTag tag = static_cast<ProjectResourceFactoryBase&>(x30_factory).
                     TagFromPath(path, hecl::SharedBlenderToken);
    if (tag)
        return CSimplePool::GetObj(tag, pvxfer);

    return {};
}

bool ProjectManager::m_registeredSpecs = false;
ProjectManager::ProjectManager(ViewManager &vm)
: m_vm(vm), m_clientProc(1), m_factoryMP1(m_clientProc), m_objStore(m_factoryMP1, *this)
{
    if (!m_registeredSpecs)
    {
        HECLRegisterDataSpecs();
        m_registeredSpecs = true;
    }
    g_SharedManager = this;
}

bool ProjectManager::newProject(const hecl::SystemString& path)
{
    hecl::ProjectRootPath projPath = hecl::SearchForProject(path);
    if (projPath)
    {
        Log.report(logvisor::Warning, _S("project already exists at '%s'"), path.c_str());
        return false;
    }

    hecl::MakeDir(path.c_str());
    m_proj.reset(new hecl::Database::Project(path));
    if (!*m_proj)
    {
        m_proj.reset();
        return false;
    }

    m_vm.ProjectChanged(*m_proj);
    m_vm.SetupEditorView();
    saveProject();

    hecl::SystemString windowTitle(m_proj->getProjectRootPath().getLastComponent());
    windowTitle += _S(" - URDE");
    m_vm.m_mainWindow->setTitle(windowTitle.c_str());
    m_vm.DismissSplash();
    m_vm.FadeInEditors();

    return true;
}

bool ProjectManager::openProject(const hecl::SystemString& path)
{
    hecl::ProjectRootPath projPath = hecl::SearchForProject(path);
    if (!projPath)
    {
        Log.report(logvisor::Warning, _S("project doesn't exist at '%s'"), path.c_str());
        return false;
    }

    m_proj.reset(new hecl::Database::Project(projPath));
    if (!*m_proj)
    {
        m_proj.reset();
        return false;
    }

    hecl::ProjectPath urdeSpacesPath(*m_proj, _S(".hecl/urde_spaces.yaml"));
    FILE* fp = hecl::Fopen(urdeSpacesPath.getAbsolutePath().c_str(), _S("r"));

    bool needsSave = false;
    athena::io::YAMLDocReader r;
    if (!fp)
    {
        needsSave = true;
        goto makeProj;
    }

    yaml_parser_set_input_file(r.getParser(), fp);
    if (!r.ValidateClassType("UrdeSpacesState"))
    {
        needsSave = true;
        fclose(fp);
        goto makeProj;
    }

    r.reset();
    fseek(fp, 0, SEEK_SET);
    yaml_parser_set_input_file(r.getParser(), fp);
    if (!r.parse())
    {
        needsSave = true;
        fclose(fp);
        goto makeProj;
    }
    fclose(fp);

makeProj:
    m_vm.ProjectChanged(*m_proj);

    if (!needsSave)
        m_vm.SetupEditorView(r);
    else
        m_vm.SetupEditorView();

    m_factoryMP1.IndexMP1Resources(*m_proj);
    m_mainMP1.emplace(m_factoryMP1, m_objStore);
    m_vm.InitMP1(*m_mainMP1);
    m_vm.BuildTestPART(m_objStore);

    if (needsSave)
        saveProject();

    {
        hecl::SystemString windowTitle(m_proj->getProjectRootPath().getLastComponent());
        windowTitle += _S(" - URDE");
        m_vm.m_mainWindow->setTitle(windowTitle.c_str());
    }
    m_vm.DismissSplash();
    m_vm.FadeInEditors();
    m_vm.pushRecentProject(m_proj->getProjectRootPath().getAbsolutePath());
    return true;
}

bool ProjectManager::extractGame(const hecl::SystemString& path)
{
    return false;
}

bool ProjectManager::saveProject()
{
    if (!m_proj)
        return false;

    hecl::ProjectPath oldSpacesPath(*m_proj, _S(".hecl/~urde_spaces.yaml"));
    FILE* fp = hecl::Fopen(oldSpacesPath.getAbsolutePath().c_str(), _S("w"));
    if (!fp)
        return false;

    athena::io::YAMLDocWriter w("UrdeSpacesState");
    yaml_emitter_set_output_file(w.getEmitter(), fp);
    m_vm.SaveEditorView(w);
    if (!w.finish())
    {
        fclose(fp);
        return false;
    }
    fclose(fp);

    hecl::ProjectPath newSpacesPath(*m_proj, _S(".hecl/urde_spaces.yaml"));

    hecl::Unlink(newSpacesPath.getAbsolutePath().c_str());
    hecl::Rename(oldSpacesPath.getAbsolutePath().c_str(),
                 newSpacesPath.getAbsolutePath().c_str());

    m_vm.pushRecentProject(m_proj->getProjectRootPath().getAbsolutePath());

    return true;
}

void ProjectManager::mainUpdate()
{
    if (m_mainMP1)
        m_mainMP1->Proc();
}

void ProjectManager::asyncIdle()
{
    m_factoryMP1.AsyncIdle();
}

void ProjectManager::shutdown()
{
    if (m_mainMP1)
        m_mainMP1->Shutdown();
    m_clientProc.shutdown();
    m_factoryMP1.Shutdown();
}

}
