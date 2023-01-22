#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include <map>
#include <vector>

#include "Runtime/ConsoleVariables/CVar.hpp"

namespace metaforce {
class FileStoreManager;
extern CVar* com_developer;
extern CVar* com_configfile;
extern CVar* com_enableCheats;
extern CVar* com_cubemaps;
class CVarManager final {
  using CVarContainer = StoreCVar::CVarContainer;
  template <typename T>
  CVar* _newCVar(std::string_view name, std::string_view help, const T& value, CVar::EFlags flags) {
    if (CVar* ret = registerCVar(std::make_unique<CVar>(name, value, help, flags))) {
      deserialize(ret);
      return ret;
    }
    return nullptr;
  }

  FileStoreManager& m_store;
  bool m_useBinary;
  static CVarManager* m_instance;

public:
  CVarManager() = delete;
  CVarManager(const CVarManager&) = delete;
  CVarManager& operator=(const CVarManager&) = delete;
  CVarManager& operator=(const CVarManager&&) = delete;
  CVarManager(FileStoreManager& store, bool useBinary = false);
  ~CVarManager();

  CVar* newCVar(std::string_view name, std::string_view help, const  zeus::CVector2f& value, CVar::EFlags flags) {
    return _newCVar<const zeus::CVector2f>(name, help, value, flags);
  }
  CVar* newCVar(std::string_view name, std::string_view help, const  zeus::CVector2d& value, CVar::EFlags flags) {
    return _newCVar<const zeus::CVector2d>(name, help, value, flags);
  }
  CVar* newCVar(std::string_view name, std::string_view help, const  zeus::CVector3f& value, CVar::EFlags flags) {
    return _newCVar<const zeus::CVector3f>(name, help, value, flags);
  }
  CVar* newCVar(std::string_view name, std::string_view help, const  zeus::CVector3d& value, CVar::EFlags flags) {
    return _newCVar<const zeus::CVector3d>(name, help, value, flags);
  }
  CVar* newCVar(std::string_view name, std::string_view help, const  zeus::CVector4f& value, CVar::EFlags flags) {
    return _newCVar<const zeus::CVector4f>(name, help, value, flags);
  }
  CVar* newCVar(std::string_view name, std::string_view help, const  zeus::CVector4d& value, CVar::EFlags flags) {
    return _newCVar<const zeus::CVector4d>(name, help, value, flags);
  }
  CVar* newCVar(std::string_view name, std::string_view help, std::string_view value, CVar::EFlags flags) {
    return _newCVar<std::string_view>(name, help, value, flags);
  }
  CVar* newCVar(std::string_view name, std::string_view help, bool value, CVar::EFlags flags) {
    return _newCVar<bool>(name, help, value, flags);
  }
  // Float and double are internally identical, all floating point values are stored as `double`
  CVar* newCVar(std::string_view name, std::string_view help, float value, CVar::EFlags flags) {
    return _newCVar<double>(name, help, static_cast<double>(value), flags);
  }
  CVar* newCVar(std::string_view name, std::string_view help, double value, CVar::EFlags flags) {
    return _newCVar<double>(name, help, value, flags);
  }
  // Integer CVars can be seamlessly converted between either type, the distinction is to make usage absolutely clear
  CVar* newCVar(std::string_view name, std::string_view help, int32_t value, CVar::EFlags flags) {
    return _newCVar<int32_t>(name, help, value, flags);
  }

  CVar* newCVar(std::string_view name, std::string_view help, uint32_t value, CVar::EFlags flags) {
    return _newCVar<uint32_t>(name, help, value, flags);
  }

  CVar* registerCVar(std::unique_ptr<CVar>&& cvar);

  CVar* findCVar(std::string_view name);
  template <class... _Args>
  CVar* findOrMakeCVar(std::string_view name, _Args&&... args) {
    if (CVar* cv = findCVar(name))
      return cv;
    return newCVar(name, std::forward<_Args>(args)...);
  }

  std::vector<CVar*> archivedCVars() const;
  std::vector<CVar*> cvars(CVar::EFlags filter = CVar::EFlags::Any) const;

  void deserialize(CVar* cvar);
  void serialize();

  static CVarManager* instance();

  void proc();

  void setDeveloperMode(bool v, bool setDeserialized = false);
  void setCheatsEnabled(bool v, bool setDeserialized = false);
  bool restartRequired() const;

  void parseCommandLine(const std::vector<std::string>& args);

  FileStoreManager& fileStoreManager() { return m_store; }

private:
  bool suppressDeveloper();
  void restoreDeveloper(bool oldDeveloper);

  std::unordered_map<std::string, std::unique_ptr<CVar>> m_cvars;
  std::map<std::string, std::string> m_deferedCVars;
  std::vector<StoreCVar::CVar> loadCVars(const std::string& filename) const;
};

} // namespace hecl
