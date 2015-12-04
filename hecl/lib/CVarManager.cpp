#include "HECL/CVarManager.hpp"
#include "HECL/CVar.hpp"
#include <Athena/FileWriter.hpp>
#include <Athena/Utility.hpp>
#include <HECL/Runtime.hpp>
#include <memory>

namespace HECL
{

CVar* com_developer = nullptr;
CVar* com_configfile = nullptr;
CVar* com_enableCheats = nullptr;

LogVisor::LogModule CVarLog("CVarManager");
CVarManager::CVarManager(HECL::Runtime::FileStoreManager& store, bool useBinary)
    : m_store(store),
      m_useBinary(useBinary)
{
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
    Athena::utility::tolower(tmp);
    if (m_cvars.find(tmp) != m_cvars.end())
        return false;

    m_cvars[tmp] = cvar;
    return true;
}

CVar* CVarManager::findCVar(std::string name)
{
    Athena::utility::tolower(name);
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
    HECL::SystemString filename = m_store.getStoreRoot() + _S('/') + com_configfile->toWideLiteral();
#else
    HECL::SystemString filename = m_store.getStoreRoot() + _S('/') + com_configfile->toLiteral();
#endif
    HECL::Sstat st;

    if (m_useBinary)
    {
        filename += _S(".bin");
        if (HECL::Stat(filename.c_str(), &st) || !S_ISREG(st.st_mode))
            return;
        Athena::io::FileReader reader(filename);
        if (reader.isOpen())
            container.read(reader);
    }
    else
    {
        filename += _S(".yaml");
        if (HECL::Stat(filename.c_str(), &st) || !S_ISREG(st.st_mode))
            return;
        FILE* f = HECL::Fopen(filename.c_str(), _S("rb"));
        if (f)
            container.fromYAMLFile(f);
        fclose(f);
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
                CVarLog.report(LogVisor::Error, _S("Stored type for %s does not match actual type!"), tmp.m_name.c_str());
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
    HECL::SystemString filename = m_store.getStoreRoot() + _S('/') + com_configfile->toWideLiteral();
#else
    HECL::SystemString filename = m_store.getStoreRoot() + _S('/') + com_configfile->toLiteral();
#endif

    if (m_useBinary)
    {
        filename += _S(".bin");
        Athena::io::FileWriter writer(filename);
        if (writer.isOpen())
            container.write(writer);
    }
    else
    {
        filename += _S(".yaml");
        FILE* f = HECL::Fopen(filename.c_str(), _S("wb"));
        if (f)
            container.toYAMLFile(f);
        fclose(f);
    }
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
