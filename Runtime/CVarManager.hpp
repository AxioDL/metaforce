#ifndef CVARMANAGER_HPP
#define CVARMANAGER_HPP

#include <unordered_map>
#include <vector>
#include <fstream>
#include "CVar.hpp"

namespace Retro
{

class CVarManager
{
    template <typename T>
    std::shared_ptr<CVar> _newCVar(const std::string& name, const std::string& help, const T& value, CVar::EFlags flags)
    {
        std::shared_ptr<CVar> ret(new CVar(name, value, help, flags, this));
        if (registerCVar(ret))
            return ret;
        return nullptr;
    }
public:
    CVarManager();
    ~CVarManager();

    void initialize();

    std::shared_ptr<CVar> newCVar(const std::string& name, const std::string& help, const Zeus::CColor& value, CVar::EFlags flags)
    { return _newCVar<Zeus::CColor>(name, help, value, flags); }
    std::shared_ptr<CVar> newCVar(const std::string& name, const std::string& help, const std::string& value, CVar::EFlags flags)
    { return _newCVar<std::string>(name, help, value, flags); }
    std::shared_ptr<CVar> newCVar(const std::string& name, const std::string& help, bool value, CVar::EFlags flags)
    { return _newCVar<bool>(name, help, value, flags); }
    std::shared_ptr<CVar> newCVar(const std::string& name, const std::string& help, float value, CVar::EFlags flags)
    { return _newCVar<float>(name, help, value, flags); }
    std::shared_ptr<CVar> newCVar(const std::string& name, const std::string& help, int value, CVar::EFlags flags)
    { return _newCVar<int>(name, help, value, flags); }

    bool registerCVar(std::shared_ptr<CVar> cvar);

    std::shared_ptr<CVar>findCVar(const std::string& name);

    std::vector<std::shared_ptr<CVar>> archivedCVars() const;
    std::vector<std::shared_ptr<CVar>> cvars() const;

private:
    bool suppressDeveloper();
    void restoreDeveloper(bool oldDeveloper);

    std::unordered_map<std::string, std::shared_ptr<CVar>> m_cvars;
};
}

#endif // CVARMANAGER_HPP
