#pragma once

#include "hecl/Database.hpp"
#include "athena/DNAYaml.hpp"
#include "ProjectResourceFactoryMP1.hpp"
#include "Runtime/CSimplePool.hpp"
#include "hecl/Runtime.hpp"
#include "MP1/MP1.hpp"

namespace urde
{
class ViewManager;

using ConfigReader = athena::io::YAMLDocReader;
using ConfigWriter = athena::io::YAMLDocWriter;

class ProjectResourcePool : public CSimplePool
{
    class ProjectManager& m_parent;
public:
    ProjectResourcePool(IFactory& factory, ProjectManager& parent)
    : CSimplePool(factory), m_parent(parent) {}
    CToken GetObj(std::string_view);
    CToken GetObj(std::string_view, const CVParamTransfer&);
};

class ProjectManager
{
    ViewManager& m_vm;
    std::unique_ptr<hecl::Database::Project> m_proj;
    static bool m_registeredSpecs;
    hecl::ClientProcess m_clientProc;
    ProjectResourceFactoryMP1 m_factoryMP1;
    ProjectResourcePool m_objStore;
    std::experimental::optional<MP1::CMain> m_mainMP1;
    bool m_precooking = false;

public:
    static ProjectManager* g_SharedManager;
    ProjectManager(ViewManager& vm);
    operator bool() const {return m_proj.operator bool();}

    hecl::Database::Project* project() {return m_proj.get();}
    ProjectResourcePool& objectStore() {return m_objStore;}
    ProjectResourceFactoryMP1& resourceFactoryMP1() {return m_factoryMP1;}
    MP1::CMain* gameMain() {return m_mainMP1 ? &*m_mainMP1 : nullptr;}
    SObjectTag TagFromPath(hecl::SystemStringView path) const
    { return m_factoryMP1.ProjectResourceFactoryBase::TagFromPath(path); }

    bool newProject(hecl::SystemStringView path);
    bool openProject(hecl::SystemStringView path);
    bool extractGame(hecl::SystemStringView path);
    bool saveProject();

    void mainUpdate();
    void mainDraw();
    void shutdown();
};

}

