#include "hecl/CVarManager.hpp"

#include <algorithm>
#include <memory>
#include <regex>

#include "hecl/Console.hpp"
#include "hecl/hecl.hpp"
#include "hecl/Runtime.hpp"

#include <athena/FileWriter.hpp>
#include <athena/Utility.hpp>

namespace hecl {

CVar* com_developer = nullptr;
CVar* com_configfile = nullptr;
CVar* com_enableCheats = nullptr;
CVar* com_cubemaps = nullptr;
CVar* com_variableDt = nullptr;

static const std::regex cmdLineRegex("\\+([\\w\\.]+)=([\\w\\.\\-]+)");
CVarManager* CVarManager::m_instance = nullptr;

static logvisor::Module CVarLog("CVarManager");
CVarManager::CVarManager(hecl::Runtime::FileStoreManager& store, bool useBinary)
: m_store(store), m_useBinary(useBinary) {
  m_instance = this;
  com_configfile =
      newCVar("config", "File to store configuration", std::string("config"),
              CVar::EFlags::System | CVar::EFlags::ReadOnly | CVar::EFlags::NoDeveloper | CVar::EFlags::Hidden);
  com_developer = newCVar("developer", "Enables developer mode", false,
                          (CVar::EFlags::System | CVar::EFlags::ReadOnly | CVar::EFlags::InternalArchivable));
  com_enableCheats = newCVar(
      "cheats", "Enable cheats", false,
      (CVar::EFlags::System | CVar::EFlags::ReadOnly | CVar::EFlags::Hidden | CVar::EFlags::InternalArchivable));
  com_cubemaps = newCVar("cubemaps", "Enable cubemaps", false,
                         (CVar::EFlags::Game | CVar::EFlags::ReadOnly | CVar::EFlags::InternalArchivable));
  com_variableDt = newCVar("variableDt", "Enable variable delta time (experimental)", false,
                           (CVar::EFlags::Game | CVar::EFlags::ReadOnly | CVar::EFlags::InternalArchivable));
}

CVarManager::~CVarManager() {}

CVar* CVarManager::registerCVar(std::unique_ptr<CVar>&& cvar) {
  std::string tmp(cvar->name());
  athena::utility::tolower(tmp);

  if (m_cvars.find(tmp) != m_cvars.end()) {
    return nullptr;
  }

  CVar* ret = cvar.get();
  m_cvars.insert_or_assign(std::move(tmp), std::move(cvar));
  return ret;
}

CVar* CVarManager::findCVar(std::string_view name) {
  std::string lower(name);
  athena::utility::tolower(lower);
  auto search = m_cvars.find(lower);
  if (search == m_cvars.end())
    return nullptr;

  return search->second.get();
}

std::vector<CVar*> CVarManager::archivedCVars() const {
  std::vector<CVar*> ret;
  for (const auto& pair : m_cvars)
    if (pair.second->isArchive())
      ret.push_back(pair.second.get());

  return ret;
}

std::vector<CVar*> CVarManager::cvars(CVar::EFlags filter) const {
  std::vector<CVar*> ret;
  for (const auto& pair : m_cvars)
    if (filter == CVar::EFlags::Any || True(pair.second->flags() & filter))
      ret.push_back(pair.second.get());

  return ret;
}

void CVarManager::deserialize(CVar* cvar) {
  /* Make sure we're not trying to deserialize a CVar that is invalid or not exposed */
  if (!cvar || (!cvar->isArchive() && !cvar->isInternalArchivable()))
    return;

  /* First let's check for a deferred value */
  std::string lowName = cvar->name().data();
  athena::utility::tolower(lowName);
  if (const auto iter = m_deferedCVars.find(lowName); iter != m_deferedCVars.end()) {
    std::string val = std::move(iter->second);
    m_deferedCVars.erase(lowName);
    if (cvar->fromLiteralToType(val))
      return;
  }

  /* We were either unable to find a deferred value or got an invalid value */
#if _WIN32
  hecl::SystemString filename =
      hecl::SystemString(m_store.getStoreRoot()) + _SYS_STR('/') + com_configfile->toWideLiteral();
#else
  hecl::SystemString filename =
      hecl::SystemString(m_store.getStoreRoot()) + _SYS_STR('/') + com_configfile->toLiteral();
#endif
  hecl::Sstat st;

  if (m_useBinary) {
    CVarContainer container;
    filename += _SYS_STR(".bin");
    if (hecl::Stat(filename.c_str(), &st) || !S_ISREG(st.st_mode))
      return;
    athena::io::FileReader reader(filename);
    if (reader.isOpen())
      container.read(reader);

    if (container.cvars.size() > 0) {
      auto serialized = std::find_if(container.cvars.begin(), container.cvars.end(),
                                     [&cvar](const DNACVAR::CVar& c) { return c.m_name == cvar->name(); });

      if (serialized != container.cvars.end()) {
        DNACVAR::CVar& tmp = *serialized;

        if (cvar->m_value != tmp.m_value) {
          CVarUnlocker lc(cvar);
          cvar->fromLiteralToType(tmp.m_value);
          cvar->m_wasDeserialized = true;
        }
      }
    }
  } else {
    filename += _SYS_STR(".yaml");
    if (hecl::Stat(filename.c_str(), &st) || !S_ISREG(st.st_mode))
      return;
    athena::io::FileReader reader(filename);
    if (reader.isOpen()) {
      athena::io::YAMLDocReader docReader;
      if (docReader.parse(&reader)) {
        std::unique_ptr<athena::io::YAMLNode> root = docReader.releaseRootNode();
        auto serialized = std::find_if(root->m_mapChildren.begin(), root->m_mapChildren.end(),
                                       [&cvar](const auto& c) { return c.first == cvar->name(); });

        if (serialized != root->m_mapChildren.end()) {
          const std::unique_ptr<athena::io::YAMLNode>& tmp = serialized->second;

          if (cvar->m_value != tmp->m_scalarString) {
            CVarUnlocker lc(cvar);
            cvar->fromLiteralToType(tmp->m_scalarString);
            cvar->m_wasDeserialized = true;
          }
        }
      }
    }
  }
}

void CVarManager::serialize() {
#if _WIN32
  hecl::SystemString filename =
      hecl::SystemString(m_store.getStoreRoot()) + _SYS_STR('/') + com_configfile->toWideLiteral();
#else
  hecl::SystemString filename =
      hecl::SystemString(m_store.getStoreRoot()) + _SYS_STR('/') + com_configfile->toLiteral();
#endif

  if (m_useBinary) {
    CVarContainer container;
    for (const auto& pair : m_cvars) {
      const auto& cvar = pair.second;

      if (cvar->isArchive() || (cvar->isInternalArchivable() && cvar->wasDeserialized() && !cvar->hasDefaultValue())) {
        container.cvars.push_back(*cvar);
      }
    }
    container.cvarCount = atUint32(container.cvars.size());

    filename += _SYS_STR(".bin");
    athena::io::FileWriter writer(filename);
    if (writer.isOpen())
      container.write(writer);
  } else {
    filename += _SYS_STR(".yaml");

    athena::io::FileReader r(filename);
    athena::io::YAMLDocWriter docWriter(r.isOpen() ? &r : nullptr);
    r.close();

    docWriter.setStyle(athena::io::YAMLNodeStyle::Block);
    for (const auto& pair : m_cvars) {
      const auto& cvar = pair.second;

      if (cvar->isArchive() || (cvar->isInternalArchivable() && cvar->wasDeserialized() && !cvar->hasDefaultValue())) {
        docWriter.writeString(cvar->name().data(), cvar->toLiteral());
      }
    }

    athena::io::FileWriter w(filename);
    if (w.isOpen())
      docWriter.finish(&w);
  }
}

CVarManager* CVarManager::instance() { return m_instance; }

void CVarManager::list(Console* con, const std::vector<std::string>& /*args*/) {
  for (const auto& cvar : m_cvars) {
    if (!cvar.second->isHidden())
      con->report(Console::Level::Info, FMT_STRING("{}: {}"), cvar.second->name(), cvar.second->help());
  }
}

void CVarManager::setCVar(Console* con, const std::vector<std::string>& args) {
  if (args.size() < 2) {
    con->report(Console::Level::Info, FMT_STRING("Usage setCvar <cvar> <value>"));
    return;
  }

  std::string cvName = args[0];
  athena::utility::tolower(cvName);
  const auto iter = m_cvars.find(cvName);
  if (iter == m_cvars.end()) {
    con->report(Console::Level::Error, FMT_STRING("CVar '{}' does not exist"), args[0]);
    return;
  }

  const auto& cv = iter->second;
  std::string oldVal = cv->value();
  std::string value = args[1];
  auto it = args.begin() + 2;
  for (; it != args.end(); ++it)
    value += " " + *it;

  /*  Check to make sure we're not redundantly assigning the value */
  if (cv->value() == value)
    return;

  if (!cv->fromLiteralToType(value))
    con->report(Console::Level::Warning, FMT_STRING("Unable to set cvar '{}' to value '{}'"), cv->name(), value);
  else
    con->report(Console::Level::Info, FMT_STRING("Set '{}' from '{}' to '{}'"), cv->name(), oldVal, value);
}

void CVarManager::getCVar(Console* con, const std::vector<std::string>& args) {
  if (args.empty()) {
    con->report(Console::Level::Info, FMT_STRING("Usage getCVar <cvar>"));
    return;
  }

  std::string cvName = args[0];
  athena::utility::tolower(cvName);
  const auto iter = m_cvars.find(cvName);
  if (iter == m_cvars.end()) {
    con->report(Console::Level::Error, FMT_STRING("CVar '{}' does not exist"), args[0]);
    return;
  }

  const auto& cv = iter->second;
  con->report(Console::Level::Info, FMT_STRING("'{}' = '{}'"), cv->name(), cv->value());
}

void CVarManager::setDeveloperMode(bool v, bool setDeserialized) {
  com_developer->unlock();
  com_developer->fromBoolean(v);
  if (setDeserialized)
    com_developer->m_wasDeserialized = true;
  com_developer->lock();
  com_developer->setModified();
}

void CVarManager::setCheatsEnabled(bool v, bool setDeserialized) {
  com_enableCheats->unlock();
  com_enableCheats->fromBoolean(v);
  if (setDeserialized)
    com_enableCheats->m_wasDeserialized = true;
  com_enableCheats->lock();
  com_enableCheats->setModified();
}

void CVarManager::setVariableDtEnabled(bool v, bool setDeserialized) {
  com_variableDt->unlock();
  com_variableDt->fromBoolean(v);
  if (setDeserialized)
    com_variableDt->m_wasDeserialized = true;
  com_variableDt->lock();
  com_variableDt->setModified();
}

bool CVarManager::restartRequired() const {
  return std::any_of(m_cvars.cbegin(), m_cvars.cend(), [](const auto& entry) {
    return entry.second->isModified() && entry.second->modificationRequiresRestart();
  });
}

void CVarManager::parseCommandLine(const std::vector<SystemString>& args) {
  bool oldDeveloper = suppressDeveloper();
  std::string developerName(com_developer->name());
  athena::utility::tolower(developerName);
  for (const SystemString& arg : args) {
    if (arg[0] != _SYS_STR('+')) {
      continue;
    }

    const std::string tmp(SystemUTF8Conv(arg).str());
    std::smatch matches;
    if (!std::regex_match(tmp, matches, cmdLineRegex)) {
      continue;
    }

    std::string cvarName = matches[1].str();
    std::string cvarValue = matches[2].str();
    if (CVar* cv = findCVar(cvarName)) {
      cv->fromLiteralToType(cvarValue);
      athena::utility::tolower(cvarName);
      if (developerName == cvarName)
        /* Make sure we're not overriding developer mode when we restore */
        oldDeveloper = com_developer->toBoolean();
    } else {
      /* Unable to find an existing CVar, let's defer for the time being 8 */
      athena::utility::tolower(cvarName);
      m_deferedCVars.insert_or_assign(std::move(cvarName), std::move(cvarValue));
    }
  }

  restoreDeveloper(oldDeveloper);
}

bool CVarManager::suppressDeveloper() {
  bool oldDeveloper = com_developer->toBoolean();
  CVarUnlocker unlock(com_developer);
  com_developer->fromBoolean(true);

  return oldDeveloper;
}

void CVarManager::restoreDeveloper(bool oldDeveloper) {
  CVarUnlocker unlock(com_developer);
  com_developer->fromBoolean(oldDeveloper);
}
void CVarManager::proc() {
  for (const auto& [name, cvar] : m_cvars) {
    if (cvar->isModified() && !cvar->modificationRequiresRestart()) {
      cvar->dispatch();
      // Clear the modified flag now that we've informed everyone we've changed
      cvar->clearModified();
    }
  }
}

} // namespace hecl
