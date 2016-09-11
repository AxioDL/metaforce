#include "logvisor/logvisor.hpp"
#include "boo/boo.hpp"
#include "specter/specter.hpp"
#include "hecl/CVarManager.hpp"
#include "Runtime/CGameAllocator.hpp"
#include <functional>
#include "ViewManager.hpp"

static logvisor::Module AthenaLog("Athena");
static void AthenaExc(athena::error::Level level, const char* file,
                      const char*, int line, const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    AthenaLog.report(logvisor::Level(level), fmt, ap);
    va_end(ap);
}

namespace urde
{
static logvisor::Module Log{"URDE"};

struct Application : boo::IApplicationCallback
{
    hecl::Runtime::FileStoreManager m_fileMgr;
    hecl::CVarManager m_cvarManager;
    std::unique_ptr<ViewManager> m_viewManager;

    bool m_running = true;

    Application() :
        m_fileMgr(_S("urde")),
        m_cvarManager(m_fileMgr)
    {
        m_viewManager.reset(new ViewManager(m_fileMgr, m_cvarManager));
    }

    virtual ~Application() = default;

    int appMain(boo::IApplication* app)
    {
        initialize(app);
        m_viewManager->init(app);
        while (m_running)
        {
            if (!m_viewManager->proc())
                break;
        }
        m_viewManager->stop();
        m_cvarManager.serialize();
        m_viewManager.reset();
        return 0;
    }
    void appQuitting(boo::IApplication*)
    {
        m_running = false;
    }
    void appFilesOpen(boo::IApplication*, const std::vector<boo::SystemString>&);

    void initialize(boo::IApplication* /*app*/)
    {
        zeus::detectCPU();
        hecl::VerbosityLevel = 1;

        const zeus::CPUInfo& cpuInf = zeus::cpuFeatures();
        Log.report(logvisor::Info, "CPU Name: %s", cpuInf.cpuBrand);
        Log.report(logvisor::Info, "CPU Vendor: %s", cpuInf.cpuVendor);
        hecl::SystemString features;
        if (cpuInf.AESNI)
            features += _S("AES-NI");
        if (cpuInf.SSE1)
        {
            if (!features.empty())
                features += _S(", SSE1");
            else
                features += _S("SSE1");
        }
        else
        {
            Log.report(logvisor::Fatal, _S("URDE requires SSE1 minimum"));
            return;
        }
        if (cpuInf.SSE2)
            features += _S(", SSE2");
        else
        {
            Log.report(logvisor::Fatal, _S("URDE requires SSE2 minimum"));
            return;
        }
        if (cpuInf.SSE3)
            features += _S(", SSE3");
        if (cpuInf.SSSE3)
            features += _S(", SSSE3");
        if (cpuInf.SSE4a)
            features += _S(", SSE4a");
        if (cpuInf.SSE41)
            features += _S(", SSE4.1");
        if (cpuInf.SSE42)
            features += _S(", SSE4.2");
        Log.report(logvisor::Info, _S("CPU Features: %s"), features.c_str());
    }
};

/* This is here to prevent a vtable being dumped into every translation unit */
void Application::appFilesOpen(boo::IApplication *, const std::vector<boo::SystemString> &)
{

}

}

#if _WIN32
int wmain(int argc, const boo::SystemChar** argv)
#else
int main(int argc, const boo::SystemChar** argv)
#endif
{
    logvisor::RegisterStandardExceptions();
    logvisor::RegisterConsoleLogger();
    atSetExceptionHandler(AthenaExc);
    urde::Application appCb;
    int ret = boo::ApplicationRun(boo::IApplication::EPlatformType::Auto,
        appCb, _S("urde"), _S("URDE"), argc, argv, false);
    printf("IM DYING!!\n");
    return ret;
}

#if _WIN32
int APIENTRY wWinMain(HINSTANCE hInstance, HINSTANCE, LPWSTR lpCmdLine, int)
{
    int argc = 0;
    const boo::SystemChar** argv = (const wchar_t**)(CommandLineToArgvW(lpCmdLine, &argc));
    static boo::SystemChar selfPath[1024];
    GetModuleFileNameW(nullptr, selfPath, 1024);
    static const boo::SystemChar* booArgv[32] = {};
    booArgv[0] = selfPath;
    for (int i=0 ; i<argc ; ++i)
        booArgv[i+1] = argv[i];

    logvisor::CreateWin32Console();
    return wmain(argc+1, booArgv);
}
#endif
