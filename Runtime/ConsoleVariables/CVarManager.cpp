#include "Runtime/ConsoleVariables/CVarManager.hpp"

#include "Runtime/ConsoleVariables/FileStoreManager.hpp"
#include "Runtime/CBasics.hpp"
#include "Runtime/Streams/CTextInStream.hpp"
#include "Runtime/Streams/CTextOutStream.hpp"
#include "Runtime/Streams/CMemoryInStream.hpp"
#include "Runtime/Streams/CMemoryStreamOut.hpp"
#include "Runtime/CStringExtras.hpp"
#include <logvisor/logvisor.hpp>
#include <algorithm>
#include <memory>
#include <regex>

#if !defined(S_ISREG) && defined(S_IFMT) && defined(S_IFREG)
#define S_ISREG(m) (((m)&S_IFMT) == S_IFREG)
#endif

#if !defined(S_ISDIR) && defined(S_IFMT) && defined(S_IFDIR)
#define S_ISDIR(m) (((m)&S_IFMT) == S_IFDIR)
#endif

namespace metaforce {

CVar* com_developer = nullptr;
CVar* com_configfile = nullptr;
CVar* com_enableCheats = nullptr;
CVar* com_cubemaps = nullptr;

static const std::regex cmdLineRegex(R"(\+([\w\.]+)([=])?([\/\\\s\w\.\-]+)?)");
CVarManager* CVarManager::m_instance = nullptr;

static logvisor::Module CVarLog("CVarManager");
CVarManager::CVarManager(FileStoreManager& store, bool useBinary) : m_store(store), m_useBinary(useBinary) {
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
}

CVarManager::~CVarManager() {}

CVar* CVarManager::registerCVar(std::unique_ptr<CVar>&& cvar) {
  std::string tmp(cvar->name());
  CStringExtras::ToLower(tmp);

  if (m_cvars.find(tmp) != m_cvars.end()) {
    return nullptr;
  }

  CVar* ret = cvar.get();
  m_cvars.insert_or_assign(std::move(tmp), std::move(cvar));
  return ret;
}

CVar* CVarManager::findCVar(std::string_view name) {
  std::string lower(name);
  CStringExtras::ToLower(lower);
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
  /* Make sure we're not trying to deserialize a CVar that is invalid*/
  if (!cvar) {
    return;
  }

  /* First let's check for a deferred value */
  std::string lowName = cvar->name().data();
  CStringExtras::ToLower(lowName);
  if (const auto iter = m_deferedCVars.find(lowName); iter != m_deferedCVars.end()) {
    std::string val = std::move(iter->second);
    m_deferedCVars.erase(lowName);
    if (cvar->isBoolean() && val.empty()) {
      // We were deferred without a value, assume true
      cvar->fromBoolean(true);
      cvar->m_wasDeserialized = true;
      return;
    }
    if (!val.empty() && cvar->fromLiteralToType(val)) {
      cvar->m_wasDeserialized = true;
      return;
    }
  }

  /* Enforce isArchive and isInternalArchivable now that we've checked if it's been deferred */
  if (!cvar->isArchive() && !cvar->isInternalArchivable()) {
    return;
  }
  /* We were either unable to find a deferred value or got an invalid value */
  std::string filename = std::string(m_store.getStoreRoot()) + '/' + com_configfile->toLiteral() + ".yaml";
  auto container = loadCVars(filename);
  auto serialized =
      std::find_if(container.cbegin(), container.cend(), [&cvar](const auto& c) { return c.m_name == cvar->name(); });
  if (serialized != container.cend()) {
    if (cvar->m_value != serialized->m_value) {
      {
        CVarUnlocker lc(cvar);
        cvar->fromLiteralToType(serialized->m_value);
        cvar->m_wasDeserialized = true;
      }
      if (cvar->modificationRequiresRestart()) {
        cvar->dispatch();
        cvar->forceClearModified();
      }
    }
  }
}

void CVarManager::serialize() {
  std::string filename = std::string(m_store.getStoreRoot()) + '/' + com_configfile->toLiteral() + ".yaml";

  /* If we have an existing config load it in, so we can update it */
  auto container = loadCVars(filename);

  u32 minLength = 0;
  bool write = false;
  for (const auto& pair : m_cvars) {
    const auto& cvar = pair.second;

    if (cvar->isArchive() || (cvar->isInternalArchivable() && cvar->wasDeserialized() && !cvar->hasDefaultValue())) {
      write = true;
      /* Look for an existing CVar in the file... */
      auto serialized =
          std::find_if(container.begin(), container.end(), [&cvar](const auto& c) { return c.m_name == cvar->name(); });
      if (serialized != container.end()) {
        /* Found it! Update the value */
        serialized->m_value = cvar->value();
      } else {
        /* Store this value as a new CVar in the config */
        container.emplace_back(StoreCVar::CVar{std::string(cvar->name()), cvar->value()});
      }
    }
  }
  /* Compute length needed for all cvars */
  std::for_each(container.cbegin(), container.cend(),
                [&minLength](const auto& cvar) { minLength += cvar.m_name.length() + cvar.m_value.length() + 2; });
  /* Only write the CVars if any have been modified */
  if (!write) {
    return;
  }

  // Allocate enough space to write all the strings with some space to spare
  const auto requiredLen = minLength + (4 * container.size());
  std::unique_ptr<u8[]> workBuf(new u8[requiredLen]);
  CMemoryStreamOut memOut(workBuf.get(), requiredLen, CMemoryStreamOut::EOwnerShip::NotOwned, 32);
  CTextOutStream textOut(memOut);
  for (const auto& cvar : container) {
    auto str = fmt::format(FMT_STRING("{}: {}"), cvar.m_name, cvar.m_value);
    textOut.WriteString(str);
  }

  auto* file = fopen(filename.c_str(),
#ifdef _MSC_VER
                     "wb"
#else
                     "wbe"
#endif
  );
  if (file != nullptr) {
    u32 writeLen = memOut.GetWritePosition();
    u32 offset = 0;
    while (offset < writeLen) {
      offset += fwrite(workBuf.get() + offset, 1, writeLen - offset, file);
    }
    fflush(file);
  }
  fclose(file);
}

std::vector<StoreCVar::CVar> CVarManager::loadCVars(const std::string& filename) const {
  std::vector<StoreCVar::CVar> ret;
  CBasics::Sstat st;
  if (CBasics::Stat(filename.c_str(), &st) == 0 && S_ISREG(st.st_mode)) {

    auto* file = fopen(filename.c_str(),
#ifdef _MSC_VER
                       "rb"
#else
                       "rbe"
#endif
                       );
    if (file != nullptr) {

      std::unique_ptr<u8[]> inBuf(new u8[st.st_size]);
      fread(inBuf.get(), 1, st.st_size, file);
      fclose(file);
      CMemoryInStream mem(inBuf.get(), st.st_size, CMemoryInStream::EOwnerShip::NotOwned);
      CTextInStream textIn(mem, st.st_size);
      while (!textIn.IsEOF()) {
        auto cvString = textIn.GetNextLine();
        if (cvString.empty()) {
          continue;
        }
        auto parts = CStringExtras::Split(cvString, ':');
        if (parts.size() < 2) {
          continue;
        }
        ret.emplace_back(StoreCVar::CVar{CStringExtras::Trim(parts[0]), CStringExtras::Trim(parts[1])});
      }
    }
  }
  return ret;
}

CVarManager* CVarManager::instance() { return m_instance; }

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

bool CVarManager::restartRequired() const {
  return std::any_of(m_cvars.cbegin(), m_cvars.cend(), [](const auto& entry) {
    return entry.second->isModified() && entry.second->modificationRequiresRestart();
  });
}

void CVarManager::parseCommandLine(const std::vector<std::string>& args) {
  bool oldDeveloper = suppressDeveloper();
  std::string developerName(com_developer->name());
  CStringExtras::ToLower(developerName);
  for (const std::string& arg : args) {
    if (arg[0] != '+') {
      continue;
    }

    std::smatch matches;
    std::string cvarName;
    std::string cvarValue;

    if (std::regex_match(arg, matches, cmdLineRegex)) {
      std::vector<std::string> realMatches;
      for (auto match : matches) {
        if (match.matched) {
          realMatches.push_back(match);
        }
      }
      if (realMatches.size() == 2) {
        cvarName = matches[1].str();
      } else if (realMatches.size() == 4) {
        cvarName = matches[1].str();
        cvarValue = matches[3].str();
      }
    }

    if (CVar* cv = findCVar(cvarName)) {
      if (cvarValue.empty() && cv->isBoolean()) {
        // We were set from the command line with an empty value, assume true
        cv->fromBoolean(true);
      } else if (!cvarValue.empty()) {
        cv->fromLiteralToType(cvarValue);
      }
      cv->m_wasDeserialized = true;
      CStringExtras::ToLower(cvarName);
      if (developerName == cvarName)
        /* Make sure we're not overriding developer mode when we restore */
        oldDeveloper = com_developer->toBoolean();
    } else {
      /* Unable to find an existing CVar, let's defer for the time being 8 */
      CStringExtras::ToLower(cvarName);
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
    if (cvar->isModified()) {
      cvar->dispatch();
    }
    if (cvar->isModified() && !cvar->modificationRequiresRestart()) {
      // Clear the modified flag now that we've informed everyone we've changed
      cvar->clearModified();
    }
  }
}

} // namespace metaforce
