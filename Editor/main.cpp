#include <LogVisor/LogVisor.hpp>
#include <boo/boo.hpp>
#include <Specter/Specter.hpp>
#include <HECL/CVarManager.hpp>
#include <Runtime/CGameAllocator.hpp>
#include <functional>
#include "ViewManager.hpp"
#include <Runtime/Particle/CElementGen.hpp>

namespace URDE
{
LogVisor::LogModule Log{"URDE"};

struct Application : boo::IApplicationCallback
{
    HECL::Runtime::FileStoreManager m_fileMgr;
    HECL::CVarManager m_cvarManager;
    ViewManager m_viewManager;

    bool m_running = true;

    Application() :
        m_fileMgr(_S("urde")),
        m_cvarManager(m_fileMgr),
        m_viewManager(m_fileMgr, m_cvarManager) {}

    int appMain(boo::IApplication* app)
    {
        initialize(app);
        m_viewManager.init(app);
        while (m_running)
        {
            if (!m_viewManager.proc())
                break;
        }
        m_viewManager.stop();
        m_cvarManager.serialize();
        return 0;
    }
    void appQuitting(boo::IApplication*)
    {
        m_running = false;
    }
    void appFilesOpen(boo::IApplication*, const std::vector<boo::SystemString>&)
    {

    }

    void initialize(boo::IApplication* app)
    {
        Zeus::detectCPU();

        const Zeus::CPUInfo& cpuInf = Zeus::cpuFeatures();
        Log.report(LogVisor::Info, "CPU Name: %s", cpuInf.cpuBrand);
        Log.report(LogVisor::Info, "CPU Vendor: %s", cpuInf.cpuVendor);
        HECL::SystemString features;
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
            Log.report(LogVisor::FatalError, _S("URDE requires SSE1 minimum"));
            return;
        }
        if (cpuInf.SSE2)
            features += _S(", SSE2");
        else
        {
            Log.report(LogVisor::FatalError, _S("URDE requires SSE2 minimum"));
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
        Log.report(LogVisor::Info, _S("CPU Features: %s"), features.c_str());
    }
};

}

#if _WIN32
int wmain(int argc, const boo::SystemChar** argv)
#else
int main(int argc, const boo::SystemChar** argv)
#endif
{
    LogVisor::RegisterConsoleLogger();
    URDE::Application appCb;
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

    LogVisor::CreateWin32Console();
    return wmain(argc+1, booArgv);
}
#endif
