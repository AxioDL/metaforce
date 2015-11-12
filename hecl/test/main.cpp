#include <boo/boo.hpp>
#include <LogVisor/LogVisor.hpp>

struct HECLWindowCallback : boo::IWindowCallback
{
    bool m_sizeDirty = false;
    boo::SWindowRect m_latestSize;
    void resized(const boo::SWindowRect& rect)
    {
        m_sizeDirty = true;
        m_latestSize = rect;
    }
};

struct HECLApplicationCallback : boo::IApplicationCallback
{
    HECLWindowCallback m_windowCb;
    boo::IWindow* m_mainWindow = nullptr;
    bool m_running = true;
    int appMain(boo::IApplication* app)
    {
        m_mainWindow = app->newWindow(_S("HECL Test"));
        m_mainWindow->setCallback(&m_windowCb);
        boo::IGraphicsCommandQueue* gfxQ = m_mainWindow->getCommandQueue();
        boo::IGraphicsDataFactory* gfxF = m_mainWindow->getLoadContextDataFactory();
        boo::SWindowRect mainWindowRect = m_mainWindow->getWindowFrame();
        boo::ITextureR* renderTex = gfxF->newRenderTexture(mainWindowRect.size[0], mainWindowRect.size[1], 1);
        gfxF->commit();
        while (m_running)
        {
            m_mainWindow->waitForRetrace();

            if (m_windowCb.m_sizeDirty)
            {
                gfxQ->resizeRenderTexture(renderTex,
                                          m_windowCb.m_latestSize.size[0],
                                          m_windowCb.m_latestSize.size[1]);
                m_windowCb.m_sizeDirty = false;
            }

            gfxQ->setRenderTarget(renderTex);
            gfxQ->clearTarget();
            gfxQ->resolveDisplay(renderTex);
            gfxQ->execute();
        }
        return 0;
    }
    void appQuitting(boo::IApplication* app)
    {
        m_running = false;
    }
};

#if _WIN32
int wmain(int argc, const boo::SystemChar** argv)
#else
int main(int argc, const boo::SystemChar** argv)
#endif
{
    LogVisor::RegisterConsoleLogger();
    HECLApplicationCallback appCb;
    int ret = boo::ApplicationRun(boo::IApplication::PLAT_AUTO,
        appCb, _S("hecl"), _S("HECL"), argc, argv);
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
