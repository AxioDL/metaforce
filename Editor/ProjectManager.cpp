#include "ProjectManager.hpp"
#include "ViewManager.hpp"
#include "../DataSpecRegistry.hpp"
#include "hecl/Blender/Connection.hpp"

namespace urde
{
static logvisor::Module Log("URDE::ProjectManager");
ProjectManager* ProjectManager::g_SharedManager = nullptr;

CToken ProjectResourcePool::GetObj(std::string_view name)
{
    CToken ret = CSimplePool::GetObj(name);
    if (ret)
        return ret;

    hecl::ProjectPath path(*m_parent.project(), name);
    SObjectTag tag = static_cast<ProjectResourceFactoryBase&>(x18_factory).
                     TagFromPath(path, hecl::blender::SharedBlenderToken);
    if (tag)
        return CSimplePool::GetObj(tag);

    return {};
}

CToken ProjectResourcePool::GetObj(std::string_view name, const CVParamTransfer& pvxfer)
{
    CToken ret = CSimplePool::GetObj(name, pvxfer);
    if (ret)
        return ret;

    hecl::ProjectPath path(*m_parent.project(), name);
    SObjectTag tag = static_cast<ProjectResourceFactoryBase&>(x18_factory).
                     TagFromPath(path, hecl::blender::SharedBlenderToken);
    if (tag)
        return CSimplePool::GetObj(tag, pvxfer);

    return {};
}

bool ProjectManager::m_registeredSpecs = false;
ProjectManager::ProjectManager(ViewManager &vm)
: m_vm(vm), m_clientProc(nullptr),
  m_factoryMP1(m_clientProc), m_objStore(m_factoryMP1, *this)
{
    if (!m_registeredSpecs)
    {
        HECLRegisterDataSpecs();
        m_registeredSpecs = true;
    }
    g_SharedManager = this;
}

bool ProjectManager::newProject(hecl::SystemStringView path)
{
    hecl::ProjectRootPath projPath = hecl::SearchForProject(path);
    if (projPath)
    {
        Log.report(logvisor::Warning, _SYS_STR("project already exists at '%s'"), path.data());
        return false;
    }

    hecl::MakeDir(path.data());
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
    windowTitle += _SYS_STR(" - URDE [") + hecl::SystemString(m_vm.platformName()) + _SYS_STR("]");
    m_vm.m_mainWindow->setTitle(windowTitle.c_str());
    m_vm.DismissSplash();
    m_vm.FadeInEditors();

    return true;
}

bool ProjectManager::openProject(hecl::SystemStringView path)
{
    hecl::SystemString subPath;
    hecl::ProjectRootPath projPath = hecl::SearchForProject(path, subPath);
    if (!projPath)
    {
        Log.report(logvisor::Warning, _SYS_STR("project doesn't exist at '%s'"), path.data());
        return false;
    }

    m_proj.reset(new hecl::Database::Project(projPath));
    if (!*m_proj)
    {
        m_proj.reset();
        return false;
    }

    hecl::ProjectPath urdeSpacesPath(*m_proj, _SYS_STR(".hecl/urde_spaces.yaml"));
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

    bool doRun = hecl::StringUtils::BeginsWith(subPath, _SYS_STR("out"));
    if (doRun)
    {
        m_mainMP1.emplace(nullptr, nullptr, m_vm.m_mainBooFactory,
                          m_vm.m_mainCommandQueue, m_vm.m_renderTex);
        m_vm.InitMP1(*m_mainMP1);
    }

    if (needsSave)
        saveProject();

    {
        hecl::SystemString windowTitle(m_proj->getProjectRootPath().getLastComponent());
        windowTitle += _SYS_STR(" - URDE [") + hecl::SystemString(m_vm.platformName()) + _SYS_STR("]");
        m_vm.m_mainWindow->setTitle(windowTitle.c_str());
    }
    m_vm.DismissSplash();
    m_vm.FadeInEditors();
    m_vm.pushRecentProject(m_proj->getProjectRootPath().getAbsolutePath());
    return true;
}

bool ProjectManager::extractGame(hecl::SystemStringView path)
{
    return false;
}

bool ProjectManager::saveProject()
{
    if (!m_proj)
        return false;

    hecl::ProjectPath oldSpacesPath(*m_proj, _SYS_STR(".hecl/~urde_spaces.yaml"));
    athena::io::FileWriter writer(oldSpacesPath.getAbsolutePath());
    if (!writer.isOpen())
        return false;

    athena::io::YAMLDocWriter w("UrdeSpacesState");
    m_vm.SaveEditorView(w);
    if (!w.finish(&writer))
        return false;

    hecl::ProjectPath newSpacesPath(*m_proj, _SYS_STR(".hecl/urde_spaces.yaml"));

    hecl::Unlink(newSpacesPath.getAbsolutePath().data());
    hecl::Rename(oldSpacesPath.getAbsolutePath().data(),
                 newSpacesPath.getAbsolutePath().data());

    m_vm.pushRecentProject(m_proj->getProjectRootPath().getAbsolutePath());

    return true;
}

void ProjectManager::mainUpdate()
{
    if (m_precooking)
    {
        if (!m_factoryMP1.IsBusy())
            m_precooking = false;
        else
            return;
    }

    if (m_mainMP1)
    {
        if (m_mainMP1->Proc())
        {
            m_mainMP1->Shutdown();
            m_mainMP1 = std::experimental::nullopt;
        }
    }
}

void ProjectManager::mainDraw()
{
    if (m_precooking)
        return;

    if (m_mainMP1)
        m_mainMP1->Draw();
}

void ProjectManager::shutdown()
{
    if (m_mainMP1)
        m_mainMP1->Shutdown();
    m_clientProc.shutdown();
    m_factoryMP1.Shutdown();
    hecl::blender::Connection::Shutdown();
}

}
