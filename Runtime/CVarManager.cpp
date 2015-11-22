#include "CVarManager.hpp"
#include "CVar.hpp"
#include <Athena/Utility.hpp>
#include <memory>

namespace Retro
{
CVarManager::CVarManager()
{
}

CVarManager::~CVarManager()
{
}

void CVarManager::initialize()
{
}

bool CVarManager::registerCVar(std::shared_ptr<CVar> cvar)
{
    std::string tmp = cvar->name();
    Athena::utility::tolower(tmp);
    if (m_cvars.find(tmp) != m_cvars.end())
        return false;

    m_cvars[tmp] = cvar;
    return true;
}

std::shared_ptr<CVar> CVarManager::findCVar(const std::string &name)
{
    std::string tmp = std::string(name);
    Athena::utility::tolower(tmp);
    if (m_cvars.find(tmp) == m_cvars.end())
        return nullptr;

    return m_cvars[tmp];
}

std::vector<std::shared_ptr<CVar>> CVarManager::archivedCVars() const
{
    std::vector<std::shared_ptr<CVar>> ret;
    for (std::pair<std::string, std::shared_ptr<CVar>> pair : m_cvars)
    {
        if (int(pair.second->flags() & CVar::EFlags::Archive) != 0)
            ret.push_back(pair.second);
    }

    return ret;
}

std::vector<std::shared_ptr<CVar>> CVarManager::cvars() const
{
    std::vector<std::shared_ptr<CVar>> ret;
    for (std::pair<std::string, std::shared_ptr<CVar>> pair : m_cvars)
        ret.push_back(pair.second);

    return ret;
}

bool CVarManager::suppressDeveloper()
{
    /*bool oldDeveloper = com_developer->toBoolean();
    CVarUnlocker unlock(com_developer);
    com_developer->fromBoolean(false);*/

    return false; //oldDeveloper;
}

void CVarManager::restoreDeveloper(bool oldDeveloper)
{
//    CVarUnlocker unlock(com_developer);
//    com_developer->fromBoolean(oldDeveloper);
}

}
