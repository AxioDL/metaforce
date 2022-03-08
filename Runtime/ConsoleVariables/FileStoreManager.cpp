#include "Runtime/ConsoleVariables/FileStoreManager.hpp"

#include "Runtime/CBasics.hpp"

#include <SDL.h>
#include <logvisor/logvisor.hpp>
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
static logvisor::Module Log("FileStoreManager");
FileStoreManager* g_instance = nullptr;
}

FileStoreManager::FileStoreManager(std::string_view org, std::string_view domain) : m_org(org), m_domain(domain) {
  if (g_instance != nullptr) {
    Log.report(logvisor::Fatal, FMT_STRING("Attempting to build another FileStoreManager!!"));
    return;
  }

  auto prefPath = SDL_GetPrefPath(org.data(), domain.data());
  if (prefPath == nullptr) {
#if _WIN32
#if !WINDOWS_STORE
    WCHAR home[MAX_PATH];
    if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, home)))
      Log.report(logvisor::Fatal, FMT_STRING("unable to locate profile for file store"));

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
        Log.report(logvisor::Fatal, FMT_STRING("invalid $XDG_DATA_HOME for file store (must be absolute)"));
      path = xdg_data_home;
    } else {
      const char* home = getenv("HOME");
      if (!home)
        Log.report(logvisor::Fatal, FMT_STRING("unable to locate $HOME for file store"));
      path = home;
      path += "/.local/share";
    }
    path += "/" + m_org + "/" + domain.data();
    if (CBasics::RecursiveMakeDir(path.c_str()) != 0) {
      Log.report(logvisor::Fatal, FMT_STRING("unable to mkdir at {}"), path);
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
    Log.report(logvisor::Fatal, FMT_STRING("Requested FileStoreManager instance before it's built!"));
    return nullptr;
  }
  return g_instance;
}
} // namespace hecl::Runtime
