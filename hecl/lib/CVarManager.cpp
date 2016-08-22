#include "hecl/CVarManager.hpp"
#include "hecl/CVar.hpp"
#include <athena/FileWriter.hpp>
#include <athena/Utility.hpp>
#include <hecl/Runtime.hpp>
#include <memory>

namespace hecl
{

CVar* com_developer = nullptr;
CVar* com_configfile = nullptr;
CVar* com_enableCheats = nullptr;

CVarManager* CVarManager::m_instance = nullptr;

logvisor::Module CVarLog("CVarManager");
CVarManager::CVarManager(hecl::Runtime::FileStoreManager& store, bool useBinary)
    : m_store(store),
      m_useBinary(useBinary)
{
    m_instance = this;
    com_configfile = newCVar("config", "File to store configuration", std::string("config"), CVar::EFlags::System);
    com_developer = newCVar("developer", "Enables developer mode", false, (CVar::EFlags::System | CVar::EFlags::Cheat | CVar::EFlags::ReadOnly));
    com_enableCheats = newCVar("iamaweiner", "Enable cheats", false, (CVar::EFlags::System | CVar::EFlags::ReadOnly | CVar::EFlags::Hidden));
}

CVarManager::~CVarManager()
{
}

void CVarManager::update()
{
    for (const std::pair<std::string, CVar*>& pair : m_cvars)
        if (pair.second->isModified())
        {
            pair.second->dispatch();
            pair.second->clearModified();
        }
}

bool CVarManager::registerCVar(CVar* cvar)
{
    std::string tmp = cvar->name();
    athena::utility::tolower(tmp);
    if (m_cvars.find(tmp) != m_cvars.end())
        return false;

    m_cvars[tmp] = cvar;
    return true;
}

CVar* CVarManager::findCVar(std::string name)
{
    athena::utility::tolower(name);
    if (m_cvars.find(name) == m_cvars.end())
        return nullptr;

    return m_cvars[name];
}

std::vector<CVar*> CVarManager::archivedCVars() const
{
    std::vector<CVar*> ret;
    for (const std::pair<std::string, CVar*>& pair : m_cvars)
        if (pair.second->isArchive())
            ret.push_back(pair.second);

    return ret;
}

std::vector<CVar*> CVarManager::cvars() const
{
    std::vector<CVar*> ret;
    for (const std::pair<std::string, CVar*>& pair : m_cvars)
        ret.push_back(pair.second);

    return ret;
}

void CVarManager::deserialize(CVar* cvar)
{
    if (!cvar || !cvar->isArchive())
        return;

    CVarContainer container;
#if _WIN32
    hecl::SystemString filename = m_store.getStoreRoot() + _S('/') + com_configfile->toWideLiteral();
#else
    hecl::SystemString filename = m_store.getStoreRoot() + _S('/') + com_configfile->toLiteral();
#endif
    hecl::Sstat st;

    if (m_useBinary)
    {
        filename += _S(".bin");
        if (hecl::Stat(filename.c_str(), &st) || !S_ISREG(st.st_mode))
            return;
        athena::io::FileReader reader(filename);
        if (reader.isOpen())
            container.read(reader);
    }
    else
    {
        filename += _S(".yaml");
        if (hecl::Stat(filename.c_str(), &st) || !S_ISREG(st.st_mode))
            return;
        athena::io::FileReader reader(filename);
        if (reader.isOpen())
            container.fromYAMLStream(reader);
    }


    if (container.cvars.size() > 0)
    {
        auto serialized = std::find_if(container.cvars.begin(), container.cvars.end(), [&cvar](const DNACVAR::CVar& c) -> bool
        { return c.m_name == cvar->name(); });

        if (serialized != container.cvars.end())
        {
            DNACVAR::CVar& tmp = *serialized;
            if (tmp.m_type != cvar->type())
            {
                CVarLog.report(logvisor::Error, _S("Stored type for %s does not match actual type!"), tmp.m_name.c_str());
                return;
            }

            if (cvar->m_value != tmp.m_value)
            {
                cvar->m_value = tmp.m_value;
                cvar->m_flags |= CVar::EFlags::Modified;
            }
        }
    }
}

void CVarManager::serialize()
{
    CVarContainer container;
    for (const std::pair<std::string, CVar*>& pair : m_cvars)
        if (pair.second->isArchive())
        {
            CVar tmp = *pair.second;
            container.cvars.push_back(tmp);
        }

    container.cvarCount = container.cvars.size();

#if _WIN32
    hecl::SystemString filename = m_store.getStoreRoot() + _S('/') + com_configfile->toWideLiteral();
#else
    hecl::SystemString filename = m_store.getStoreRoot() + _S('/') + com_configfile->toLiteral();
#endif

    if (m_useBinary)
    {
        filename += _S(".bin");
        athena::io::FileWriter writer(filename);
        if (writer.isOpen())
            container.write(writer);
    }
    else
    {
        filename += _S(".yaml");
        athena::io::FileWriter writer(filename);
        if (writer.isOpen())
            container.toYAMLStream(writer);
    }
}

CVarManager* CVarManager::instance()
{
    return m_instance;
}

bool CVarManager::suppressDeveloper()
{
    bool oldDeveloper = com_developer->toBoolean();
    CVarUnlocker unlock(com_developer);
    com_developer->fromBoolean(false);

    return oldDeveloper;
}

void CVarManager::restoreDeveloper(bool oldDeveloper)
{
    CVarUnlocker unlock(com_developer);
    com_developer->fromBoolean(oldDeveloper);
}

}
