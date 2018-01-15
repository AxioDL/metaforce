#ifndef CVARMANAGER_HPP
#define CVARMANAGER_HPP

#include <unordered_map>
#include <vector>
#include "CVar.hpp"

namespace hecl
{

extern BoolCVar* com_developer;
extern StringCVar* com_configfile;
extern BoolCVar* com_enableCheats;

namespace Runtime
{
class FileStoreManager;
}
class CVarManager final
{
    template <typename T>
    CVar* _newCVar(std::string_view name, std::string_view help, T& value, CVar::EFlags flags)
    {
        TCVar<T>* ret = new TCVar<T>(value, name, help, flags);
        if (registerCVar(ret))
        {
            deserialize(ret);
            return ret;
        }
        delete ret;
        return nullptr;
    }

    hecl::Runtime::FileStoreManager& m_store;
    std::string m_configFile = "config";
    bool m_useBinary;
    bool m_developerMode = false;
    bool m_enableCheats = false;
    static CVarManager* m_instance;
public:
    CVarManager() = delete;
    CVarManager(const CVarManager&) = delete;
    CVarManager& operator=(const CVarManager&) = delete;
    CVarManager& operator=(const CVarManager&&) = delete;
    CVarManager(hecl::Runtime::FileStoreManager& store, bool useBinary = false);
    ~CVarManager();

    void update();
    CVar* newCVar(std::string_view name, std::string_view help, atVec3f& value, CVar::EFlags flags)
    { return new Vec3fCVar(value, name, help, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, atVec3d& value, CVar::EFlags flags)
    { return new Vec3dCVar(value, name, help, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, atVec4f& value, CVar::EFlags flags)
    { return new Vec4fCVar(value, name, help, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, atVec4d& value, CVar::EFlags flags)
    { return new Vec4dCVar(value, name, help, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, std::string& value, CVar::EFlags flags)
    { return new StringCVar(value, name, help, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, bool& value, CVar::EFlags flags)
    { return _newCVar<bool>(name, help, value, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, float& value, CVar::EFlags flags)
    { return _newCVar<float>(name, help, value, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, double& value, CVar::EFlags flags)
    { return _newCVar<double>(name, help, value, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, int16_t& value, CVar::EFlags flags)
    { return _newCVar<int16_t>(name, help, value, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, uint16_t& value, CVar::EFlags flags)
    { return _newCVar<uint16_t>(name, help, value, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, int32_t& value, CVar::EFlags flags)
    { return _newCVar<int32_t>(name, help, value, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, uint32_t& value, CVar::EFlags flags)
    { return _newCVar<uint32_t>(name, help, value, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, int64_t& value, CVar::EFlags flags)
    { return _newCVar<int64_t>(name, help, value, flags); }
    CVar* newCVar(std::string_view name, std::string_view help, uint64_t& value, CVar::EFlags flags)
    { return _newCVar<uint64_t>(name, help, value, flags); }

    bool registerCVar(CVar* cvar);

    CVar* findCVar(std::string_view name);
    template<class... _Args>
    CVar* findOrMakeCVar(std::string_view name, _Args&&... args)
    {
        if (CVar* cv = findCVar(name))
            return cv;
        return newCVar(name, std::forward<_Args>(args)...);
    }

    std::vector<CVar*> archivedCVars() const;
    std::vector<CVar*> cvars() const;

    void deserialize(CVar* cvar);
    void serialize();

    static CVarManager* instance();

    void list(class Console* con, const std::vector<std::string>& args);
    void setCVar(class Console* con, const std::vector<std::string>& args);
    void getCVar(class Console* con, const std::vector<std::string>& args);


    bool restartRequired() const;
private:
    bool suppressDeveloper();
    void restoreDeveloper(bool oldDeveloper);

    std::unordered_map<std::string, CVar*> m_cvars;
};

}

#endif // CVARMANAGER_HPP
