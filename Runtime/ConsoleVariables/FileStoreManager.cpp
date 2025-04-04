#include "Runtime/ConsoleVariables/FileStoreManager.hpp"

#include "Runtime/CBasics.hpp"
#include "Runtime/Logging.hpp"

#include <SDL3/SDL.h>
#if _WIN32
#include <nowide/convert.hpp>
#endif

#if _WIN32
#include <ShlObj.h>
#endif

#if WINDOWS_STORE
using namespace Windows::Storage;
#endif

namespace metaforce {
namespace {
FileStoreManager* g_instance = nullptr;
}

FileStoreManager::FileStoreManager(std::string_view org, std::string_view domain) : m_org(org), m_domain(domain) {
  if (g_instance != nullptr) {
    spdlog::fatal("Attempting to build another FileStoreManager!!");
  }

  auto prefPath = SDL_GetPrefPath(org.data(), domain.data());
  if (prefPath == nullptr) {
#if _WIN32
#if !WINDOWS_STORE
    WCHAR home[MAX_PATH];
    if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, home)))
      spdlog::fatal("unable to locate profile for file store");

    std::string path = nowide::narrow(home);
#else
    StorageFolder ^ cacheFolder = ApplicationData::Current->LocalCacheFolder;
    std::string path(cacheFolder->Path->Data());
#endif
    path += "/." + m_org;

    CBasics::MakeDir(path.c_str());
    path += '/';
    path += domain.data();

    CBasics::MakeDir(path.c_str());
    m_storeRoot = path;
#else
    const char* xdg_data_home = getenv("XDG_DATA_HOME");
    std::string path;
    if (xdg_data_home) {
      if (xdg_data_home[0] != '/')
        spdlog::fatal("invalid $XDG_DATA_HOME for file store (must be absolute)");
      path = xdg_data_home;
    } else {
      const char* home = getenv("HOME");
      if (!home)
        spdlog::fatal("unable to locate $HOME for file store");
      path = home;
      path += "/.local/share";
    }
    path += "/" + m_org + "/" + domain.data();
    if (CBasics::RecursiveMakeDir(path.c_str()) != 0) {
      spdlog::fatal("unable to mkdir at {}", path);
    }
    m_storeRoot = path;
#endif
  } else {
    m_storeRoot = std::string(prefPath);
    SDL_free(prefPath);
  }
  g_instance = this;
}

FileStoreManager* FileStoreManager::instance() {
  if (g_instance == nullptr) {
    spdlog::fatal("Requested FileStoreManager instance before it's built!");
  }
  return g_instance;
}
} // namespace metaforce
