#include "HECL/Runtime.hpp"
#include <LogVisor/LogVisor.hpp>
#if _WIN32
#include <ShlObj.h>
#endif

namespace HECL
{
namespace Runtime
{
static LogVisor::LogModule Log("FileStoreManager");

FileStoreManager::FileStoreManager(const SystemString& domain)
: m_domain(domain)
{
#if _WIN32
    WCHAR home[MAX_PATH];
    if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, home)))
        Log.report(LogVisor::FatalError, _S("unable to locate profile for file store"));

    SystemString path(home);

    path += _S("/.heclrun");

    HECL::MakeDir(path.c_str());
    path += _S('/') + domain;

    HECL::MakeDir(path.c_str());
    m_storeRoot = path;
#else
    const char* home = getenv("HOME");
    if (!home)
        Log.report(LogVisor::FatalError, "unable to locate $HOME for file store");
    std::string path(home);
    path += "/.heclrun";
    if (mkdir(path.c_str(), 0755) && errno != EEXIST)
        Log.report(LogVisor::FatalError, "unable to mkdir at %s", path.c_str());
    path += '/' + domain;
    if (mkdir(path.c_str(), 0755) && errno != EEXIST)
        Log.report(LogVisor::FatalError, "unable to mkdir at %s", path.c_str());
    m_storeRoot = path;
#endif
}

}
}
