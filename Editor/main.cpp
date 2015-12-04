#include <LogVisor/LogVisor.hpp>
#include <boo/boo.hpp>
#include <Specter/Specter.hpp>
#include <HECL/CVarManager.hpp>
#include <Runtime/CGameAllocator.hpp>
#include <functional>

namespace RUDE
{
struct Application : boo::IApplicationCallback
{
    HECL::Runtime::FileStoreManager m_fileMgr;
    Specter::FontCache m_fontCache;
    boo::IWindow* m_mainWindow;
    Specter::ViewResources m_viewResources;
    std::unique_ptr<Specter::RootView> m_rootView;
    HECL::CVarManager m_cvarManager;
    bool m_running = true;

    Application() :
        m_fileMgr(_S("rude")),
        m_fontCache(m_fileMgr),
        m_cvarManager(m_fileMgr) {}

    int appMain(boo::IApplication* app)
    {
        m_mainWindow = app->newWindow(_S("RUDE"));
        m_mainWindow->showWindow();
        m_mainWindow->setWaitCursor(true);
        m_cvarManager.serialize();

        unsigned dpi = m_mainWindow->getVirtualPixelFactor() * 72;
        HECL::CVar* cvDPI = m_cvarManager.newCVar("ed_dpi", "User-selected UI DPI",
                                                  int(dpi), HECL::CVar::EFlags::Editor);

        boo::IGraphicsDataFactory* gf = m_mainWindow->getMainContextDataFactory();
        m_viewResources.init(gf, &m_fontCache, Specter::ThemeData(), dpi);
        m_rootView.reset(new Specter::RootView(m_viewResources, m_mainWindow));

        m_mainWindow->setWaitCursor(false);
        boo::IGraphicsCommandQueue* gfxQ = m_mainWindow->getCommandQueue();
        while (m_running)
        {
            if (m_rootView->isDestroyed())
                break;
            if (cvDPI->isModified())
            {
                dpi = cvDPI->toInteger();
                m_viewResources.resetDPI(dpi);
                m_rootView->resetResources(m_viewResources);
                cvDPI->clearModified();
            }
            m_rootView->dispatchEvents();
            m_rootView->draw(gfxQ);
            gfxQ->execute();
            m_mainWindow->waitForRetrace();
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
        appCb, _S("rude"), _S("RUDE"), argc, argv, false);
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
