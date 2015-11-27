#include <LogVisor/LogVisor.hpp>
#include <boo/boo.hpp>
#include <Specter/Specter.hpp>
#include <Runtime/CVarManager.hpp>
#include <Runtime/CGameAllocator.hpp>
#include <functional>

namespace RUDE
{
struct Application : boo::IApplicationCallback
{
    HECL::Runtime::FileStoreManager m_fileMgr;
    Specter::FontCache m_fontCache;
    boo::IWindow* m_mainWindow;
    Specter::ViewSystem m_viewSystem;
    Retro::CVarManager m_cvarManager;
    Zeus::CColor m_clearColor;
    bool m_running = true;

    void onCVarModified(Retro::CVar* cvar)
    {
        if (cvar == m_cvarManager.findCVar("r_clearColor"))
            m_clearColor = cvar->toColor();
    }

    Application() :
        m_fileMgr(_S("rude")),
        m_fontCache(m_fileMgr),
        m_cvarManager(m_fileMgr) {}

    int appMain(boo::IApplication* app)
    {
        m_mainWindow = app->newWindow(_S("RUDE"));
        m_cvarManager.serialize();
        Retro::CVar* tmp = m_cvarManager.findCVar("r_clearcolor");
        Retro::CVar::ListenerFunc listen = std::bind(&Application::onCVarModified, this, std::placeholders::_1);
        if (tmp)
            tmp->addListener(listen);

        boo::IGraphicsDataFactory* gf = m_mainWindow->getMainContextDataFactory();
        m_viewSystem.init(gf, &m_fontCache);
        Specter::RootView rootView(m_viewSystem, m_mainWindow);

        m_mainWindow->showWindow();
        boo::IGraphicsCommandQueue* gfxQ = m_mainWindow->getCommandQueue();
        while (m_running)
        {
            if (rootView.isDestroyed())
                break;
            m_cvarManager.update();
            m_mainWindow->waitForRetrace();
            rootView.draw(gfxQ);
            gfxQ->flushBufferUpdates();
            gfxQ->execute();
        }

        return 0;
    }
    void appQuitting(boo::IApplication*)
    {
        m_running = false;
    }
    void appFilesOpen(boo::IApplication*, const std::vector<boo::SystemString>&)
    {

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
    RUDE::Application appCb;
    int ret = ApplicationRun(boo::IApplication::EPlatformType::Auto,
        appCb, _S("rude"), _S("RUDE"), argc, argv);
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
