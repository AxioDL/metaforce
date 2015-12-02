#ifndef CVARMANAGER_HPP
#define CVARMANAGER_HPP

#include <unordered_map>
#include <vector>
#include "CVar.hpp"

namespace HECL
{
namespace Runtime
{
class FileStoreManager;
}

class CVarManager
{
    using CVarContainer = DNACVAR::CVarContainer;
    template <typename T>
    CVar* _newCVar(const std::string& name, const std::string& help, const T& value, CVar::EFlags flags)
    {
        CVar* ret(new CVar(name, value, help, flags, *this));
        if (registerCVar(ret))
        {
            if (ret->isArchive())
                deserialize(ret);
            return ret;
        }
        delete ret;
        return nullptr;
    }

    HECL::Runtime::FileStoreManager& m_store;
    bool m_useBinary;
public:
    CVarManager(HECL::Runtime::FileStoreManager& store, bool useBinary = false);
    ~CVarManager();

    void update();
    CVar* newCVar(const std::string& name, const std::string& help, const atVec4f& value, CVar::EFlags flags)
    { return _newCVar<atVec4f>(name, help, value, flags); }
    CVar* newCVar(const std::string& name, const std::string& help, const std::string& value, CVar::EFlags flags)
    { return _newCVar<std::string>(name, help, value, flags); }
    CVar* newCVar(const std::string& name, const std::string& help, bool value, CVar::EFlags flags)
    { return _newCVar<bool>(name, help, value, flags); }
    CVar* newCVar(const std::string& name, const std::string& help, float value, CVar::EFlags flags)
    { return _newCVar<float>(name, help, value, flags); }
    CVar* newCVar(const std::string& name, const std::string& help, int value, CVar::EFlags flags)
    { return _newCVar<int>(name, help, value, flags); }

    bool registerCVar(CVar* cvar);

    CVar* findCVar(std::string name);

    std::vector<CVar*> archivedCVars() const;
    std::vector<CVar*> cvars() const;

    void deserialize(CVar* cvar);
    void serialize();
private:
    bool suppressDeveloper();
    void restoreDeveloper(bool oldDeveloper);

    std::unordered_map<std::string, CVar*> m_cvars;
};

}

#endif // CVARMANAGER_HPP
