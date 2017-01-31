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
    SObjectTag tag = static_cast<ProjectResourceFactoryBase&>(x18_factory).
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
    SObjectTag tag = static_cast<ProjectResourceFactoryBase&>(x18_factory).
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
    windowTitle += _S(" - URDE [") + hecl::SystemString(m_vm.platformName()) + _S("]");
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
    athena::io::FileReader reader(urdeSpacesPath.getAbsolutePath());

    bool needsSave = false;
    athena::io::YAMLDocReader r;
    if (!reader.isOpen())
    {
        needsSave = true;
        goto makeProj;
    }

    yaml_parser_set_input(r.getParser(), (yaml_read_handler_t*)athena::io::YAMLAthenaReader, &reader);
    if (!r.ValidateClassType("UrdeSpacesState"))
    {
        needsSave = true;
        goto makeProj;
    }

    r.reset();
    reader.seek(0, athena::Begin);
    if (!r.parse(&reader))
    {
        needsSave = true;
        goto makeProj;
    }

makeProj:
    m_vm.ProjectChanged(*m_proj);

    if (!needsSave)
        m_vm.SetupEditorView(r);
    else
        m_vm.SetupEditorView();

    m_factoryMP1.IndexMP1Resources(*m_proj, m_objStore);
    m_mainMP1.emplace(m_factoryMP1, m_objStore, m_vm.m_mainBooFactory,
                      m_vm.m_mainCommandQueue, m_vm.m_renderTex);
    m_vm.InitMP1(*m_mainMP1);
    m_vm.BuildTestPART(m_objStore);

    if (needsSave)
        saveProject();

    {
        hecl::SystemString windowTitle(m_proj->getProjectRootPath().getLastComponent());
        windowTitle += _S(" - URDE [") + hecl::SystemString(m_vm.platformName()) + _S("]");
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
    athena::io::FileWriter writer(oldSpacesPath.getAbsolutePath());
    if (!writer.isOpen())
        return false;

    athena::io::YAMLDocWriter w("UrdeSpacesState");
    m_vm.SaveEditorView(w);
    if (!w.finish(&writer))
        return false;

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

void ProjectManager::mainDraw()
{
    if (m_mainMP1)
        m_mainMP1->Draw();
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
    hecl::BlenderConnection::Shutdown();
}

}
