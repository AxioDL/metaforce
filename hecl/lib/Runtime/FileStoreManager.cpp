#include "hecl/Runtime.hpp"
#include "logvisor/logvisor.hpp"
#if _WIN32
#include <ShlObj.h>
#endif

namespace hecl
{
namespace Runtime
{
static logvisor::Module Log("FileStoreManager");

FileStoreManager::FileStoreManager(const SystemString& domain)
: m_domain(domain)
{
#if _WIN32
    WCHAR home[MAX_PATH];
    if (!SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, home)))
        Log.report(logvisor::Fatal, _S("unable to locate profile for file store"));

    SystemString path(home);

    path += _S("/.heclrun");

    hecl::MakeDir(path.c_str());
    path += _S('/') + domain;

    hecl::MakeDir(path.c_str());
    m_storeRoot = path;
#else
    const char* home = getenv("HOME");
    if (!home)
        Log.report(logvisor::Fatal, "unable to locate $HOME for file store");
    std::string path(home);
    path += "/.heclrun";
    if (mkdir(path.c_str(), 0755) && errno != EEXIST)
        Log.report(logvisor::Fatal, "unable to mkdir at %s", path.c_str());
    path += '/' + domain;
    if (mkdir(path.c_str(), 0755) && errno != EEXIST)
        Log.report(logvisor::Fatal, "unable to mkdir at %s", path.c_str());
    m_storeRoot = path;
#endif
}

}
}
