#include "HECL/Runtime.hpp"
#include <LogVisor/LogVisor.hpp>

namespace HECL
{
namespace Runtime
{
static LogVisor::LogModule Log("FileStoreManager");

FileStoreManager::FileStoreManager(const SystemString& domain)
: m_domain(domain)
{
#if _WIN32
#elif __APPLE__
#else
    const char* home = getenv("HOME");
    if (!home)
        Log.report(LogVisor::FatalError, "unable to locate $HOME for file store");
    std::string path(home);
    path += "/.heclrun";
    if (mkdir(path.c_str(), 0755))
        Log.report(LogVisor::FatalError, "unable to mkdir at %s", path.c_str());
    path += '/' + domain;
    if (mkdir(path.c_str(), 0755))
        Log.report(LogVisor::FatalError, "unable to mkdir at %s", path.c_str());
    m_storeRoot = path;
#endif
}

}
}
