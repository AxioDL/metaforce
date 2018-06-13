#ifndef CVARMANAGER_HPP
#define CVARMANAGER_HPP

#include <unordered_map>
#include <vector>
#include "CVar.hpp"
#include "hecl/SystemChar.hpp"

namespace hecl
{
namespace Runtime
{
class FileStoreManager;
}
extern CVar* com_developer;
extern CVar* com_configfile;
extern CVar* com_enableCheats;
class CVarManager final
{
    using CVarContainer = DNACVAR::CVarContainer;
    template <typename T>
    CVar* _newCVar(std::string_view name, std::string_view help, const T& value, CVar::EFlags flags)
    {
        if (CVar* ret = registerCVar(std::make_unique<CVar>(name, value, help, flags, *this)))
        {
            deserialize(ret);
            return ret;
        }
        return nullptr;
    }

    hecl::Runtime::FileStoreManager& m_store;
    bool m_useBinary;
    static CVarManager* m_instance;
public:
    CVarManager() = delete;
    CVarManager(const CVarManager&) = delete;
    CVarManager& operator=(const CVarManager&) = delete;
    CVarManager& operator=(const CVarManager&&) = delete;
    CVarManager(hecl::Runtime::FileStoreManager& store, bool useBinary = false);
    ~CVarManager();

    CVar* newCVar(std::string_view name, std::string_view help, const atVec4f& value, CVar::EFlags flags)
    { return _newCVar<atVec4f>(name, help, value, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, std::string_view value, CVar::EFlags flags)
    { return _newCVar<std::string_view>(name, help, value, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, bool value, CVar::EFlags flags)
    { return _newCVar<bool>(name, help, value, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, float value, CVar::EFlags flags)
    { return _newCVar<float>(name, help, value, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, int value, CVar::EFlags flags)
    { return _newCVar<int>(name, help, value, flags); }

    CVar* registerCVar(std::unique_ptr<CVar>&& cvar);

    CVar* findCVar(std::string_view name);
    template<class... _Args>
    CVar* findOrMakeCVar(std::string_view name, _Args&&... args)
    {
        if (CVar* cv = findCVar(name))
            return cv;
        return newCVar(name, std::forward<_Args>(args)...);
    }

    std::vector<CVar*> archivedCVars() const;
    std::vector<CVar*> cvars(CVar::EFlags filter = CVar::EFlags::None) const;

    void deserialize(CVar* cvar);
    void serialize();

    static CVarManager* instance();

    void list(class Console* con, const std::vector<std::string>& args);
    void setCVar(class Console* con, const std::vector<std::string>& args);
    void getCVar(class Console* con, const std::vector<std::string>& args);


    void setDeveloperMode(bool v, bool setDeserialized=false);
    bool restartRequired() const;

    void parseCommandLine(const std::vector<SystemString>& args);
private:
    bool suppressDeveloper();
    void restoreDeveloper(bool oldDeveloper);

    std::unordered_map<std::string, std::unique_ptr<CVar>> m_cvars;
};

}

#endif // CVARMANAGER_HPP
