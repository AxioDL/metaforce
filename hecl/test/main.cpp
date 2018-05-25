#include <boo/boo.hpp>
#include "logvisor/logvisor.hpp"
#include "hecl/Console.hpp"
#include "hecl/CVarManager.hpp"
#include <athena/MemoryWriter.hpp>
#include "hecl/Runtime.hpp"
#include "hecl/HMDLMeta.hpp"
#include <cmath>
#include <thread>
#include <mutex>
#include <condition_variable>

using namespace std::literals;

struct HECLWindowCallback : boo::IWindowCallback
{
    bool m_sizeDirty = false;
    boo::SWindowRect m_latestSize;
    virtual ~HECLWindowCallback();
    void resized(const boo::SWindowRect& rect, bool /*sync*/)
    {
        m_sizeDirty = true;
        m_latestSize = rect;
    }

    bool m_destroyed = false;
    void destroyed()
    {
        m_destroyed = true;
    }

    void charKeyDown(unsigned long charCode, boo::EModifierKey mods, bool isRepeat)
    {
        hecl::Console::instance()->handleCharCode(charCode, mods, isRepeat);
    }
    void specialKeyDown(boo::ESpecialKey key, boo::EModifierKey mods, bool isRepeat)
    {
        hecl::Console::instance()->handleSpecialKeyDown(key, mods, isRepeat);
    }
    void specialKeyUp(boo::ESpecialKey key, boo::EModifierKey mods)
    {
        hecl::Console::instance()->hecl::Console::handleSpecialKeyUp(key, mods);
    }
};

HECLWindowCallback::~HECLWindowCallback()
{

}

struct HECLApplicationCallback : boo::IApplicationCallback
{
    HECLWindowCallback m_windowCb;
    hecl::Runtime::FileStoreManager m_fileStoreMgr;
    hecl::CVarManager m_cvarManager;
    hecl::Console m_console;
    std::shared_ptr<boo::IWindow> m_mainWindow;
    bool m_running = true;

    HECLApplicationCallback()
        : m_fileStoreMgr(_S("heclTest")),
          m_cvarManager(m_fileStoreMgr),
          m_console(&m_cvarManager)
    {
        m_console.registerCommand("quit"sv, "Quits application"sv, "", std::bind(&HECLApplicationCallback::quit, this, std::placeholders::_1, std::placeholders::_2));
    }

    virtual ~HECLApplicationCallback();

    int appMain(boo::IApplication* app)
    {
        hecl::VerbosityLevel = 2;

        /* Setup boo window */
        m_mainWindow = app->newWindow(_S("HECL Test"));
        m_mainWindow->setCallback(&m_windowCb);

        boo::ObjToken<boo::ITextureR> renderTex;
        boo::ObjToken<boo::IGraphicsBuffer> vubo;
        boo::ObjToken<boo::IShaderDataBinding> binding;

        struct VertexUBO
        {
            float modelview[4][4] = {};
            float modelviewInv[4][4] = {};
            float projection[4][4] = {};
            VertexUBO()
            {
                modelview[0][0] = 1.0;
                modelview[1][1] = 1.0;
                modelview[2][2] = 1.0;
                modelview[3][3] = 1.0;
                modelviewInv[0][0] = 1.0;
                modelviewInv[1][1] = 1.0;
                modelviewInv[2][2] = 1.0;
                modelviewInv[3][3] = 1.0;
                projection[0][0] = 1.0;
                projection[1][1] = 1.0;
                projection[2][2] = 1.0;
                projection[3][3] = 1.0;
            }
        } vuboData;

        /* Make ramp texture */
        using Pixel = uint8_t[4];
        static Pixel tex[256][256];
        for (int i=0 ; i<256 ; ++i)
            for (int j=0 ; j<256 ; ++j)
            {
                tex[i][j][0] = uint8_t(i);
                tex[i][j][1] = uint8_t(i);
                tex[i][j][2] = 0;
                tex[i][j][3] = 0xff;
            }

        std::mutex initmt;
        std::condition_variable initcv;
        std::mutex loadmt;
        std::condition_variable loadcv;
        std::unique_lock<std::mutex> outerLk(initmt);
        std::thread loaderThr([&]()
        {
            std::unique_lock<std::mutex> innerLk(initmt);
            boo::IGraphicsDataFactory* gfxF = m_mainWindow->getLoadContextDataFactory();

            /* HECL managers */
            hecl::Runtime::FileStoreManager fileMgr(app->getUniqueName());
            hecl::Runtime::ShaderCacheManager shaderMgr(fileMgr, gfxF);

            /* Compile HECL shader */
            static std::string testShader = "HECLOpaque(Texture(0, UV(0)))";
            //static std::string testShader = "HECLOpaque(vec3(1.0,1.0,1.0),1.0)";
            hecl::Runtime::ShaderTag testShaderTag(testShader, 0, 1, 0, 0, boo::Primitive::TriStrips,
                                                   hecl::Backend::ReflectionType::None, false, false, false);
            std::shared_ptr<hecl::Runtime::ShaderPipelines> testShaderObj =
            shaderMgr.buildShader(testShaderTag, testShader, "testShader", *gfxF);

            gfxF->commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx)
            {
                boo::SWindowRect mainWindowRect = m_mainWindow->getWindowFrame();
                renderTex = ctx.newRenderTexture(size_t(mainWindowRect.size[0]), size_t(mainWindowRect.size[1]),
                                                 boo::TextureClampMode::Repeat, 0, 0);

                /* Generate meta structure (usually statically serialized) */
                hecl::HMDLMeta testMeta;
                testMeta.topology = hecl::HMDLTopology::TriStrips;
                testMeta.vertStride = 32;
                testMeta.vertCount = 4;
                testMeta.indexCount = 4;
                testMeta.colorCount = 0;
                testMeta.uvCount = 1;
                testMeta.weightCount = 0;
                testMeta.bankCount = 0;

                /* Binary form of meta structure */
                atUint8 testMetaBuf[HECL_HMDL_META_SZ];
                athena::io::MemoryWriter testMetaWriter(testMetaBuf, HECL_HMDL_META_SZ);
                testMeta.write(testMetaWriter);

                /* Make Tri-strip VBO */
                struct Vert
                {
                    float pos[3];
                    float norm[3];
                    float uv[2];
                };
                static const Vert quad[4] =
                {
                    {{0.5,0.5},{},{1.0,1.0}},
                    {{-0.5,0.5},{},{0.0,1.0}},
                    {{0.5,-0.5},{},{1.0,0.0}},
                    {{-0.5,-0.5},{},{0.0,0.0}}
                };

                /* Now simple IBO */
                static const uint32_t ibo[4] = {0,1,2,3};

                /* Construct quad mesh against boo factory */
                hecl::Runtime::HMDLData testData(ctx, testMetaBuf, quad, ibo);

                boo::ObjToken<boo::ITexture> texture =
                ctx.newStaticTexture(256, 256, 1, boo::TextureFormat::RGBA8, boo::TextureClampMode::Repeat, tex, 256*256*4).get();

                /* Make vertex uniform buffer */
                vubo = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(VertexUBO), 1).get();

                /* Assemble data binding */
                binding = testData.newShaderDataBindng(ctx, testShaderObj->m_pipelines[0], 1, &vubo, nullptr, 1, &texture);
                return true;
            } BooTrace);

            /* Return control to main thread */
            innerLk.unlock();
            initcv.notify_one();

            /* Wait for exit */
            std::unique_lock<std::mutex> lk(loadmt);
            while (m_running)
            {
                loadcv.wait(lk);
            }
        });
        initcv.wait(outerLk);

        m_mainWindow->showWindow();
        m_windowCb.m_latestSize = m_mainWindow->getWindowFrame();
        boo::IGraphicsCommandQueue* gfxQ = m_mainWindow->getCommandQueue();
        m_mainWindow->getMainContextDataFactory();

        size_t frameIdx = 0;
        while (m_running)
        {
            m_mainWindow->waitForRetrace();

            if (m_windowCb.m_destroyed)
            {
                m_running = false;
                break;
            }

            if (m_windowCb.m_sizeDirty)
            {
                gfxQ->resizeRenderTexture(renderTex,
                                          size_t(m_windowCb.m_latestSize.size[0]),
                                          size_t(m_windowCb.m_latestSize.size[1]));
                m_windowCb.m_sizeDirty = false;
            }

            m_console.proc();

            gfxQ->setRenderTarget(renderTex);
            boo::SWindowRect r = m_windowCb.m_latestSize;
            r.location[0] = 0;
            r.location[1] = 0;
            gfxQ->setViewport(r);
            gfxQ->setScissor(r);
            float rgba[] = {sinf(frameIdx / 60.0f), cosf(frameIdx / 60.0f), 0.0f, 1.0f};
            gfxQ->setClearColor(rgba);
            gfxQ->clearTarget();

            vuboData.modelview[3][0] = sinf(frameIdx / 60.0f) * 0.5f;
            vuboData.modelview[3][1] = cosf(frameIdx / 60.0f) * 0.5f;
            vubo.cast<boo::IGraphicsBufferD>()->load(&vuboData, sizeof(vuboData));

            gfxQ->setShaderDataBinding(binding);
            gfxQ->drawIndexed(0, 4);
            gfxQ->resolveDisplay(renderTex);
            m_console.draw(gfxQ);
            gfxQ->execute();

            ++frameIdx;
        }

        std::unique_lock<std::mutex> finallk(loadmt);
        m_cvarManager.serialize();
        finallk.unlock();
        gfxQ->stopRenderer();
        loadcv.notify_one();
        loaderThr.join();
        return 0;
    }
    void appQuitting(boo::IApplication* /*app*/)
    {
        m_running = false;
    }

    void quit(hecl::Console* /*con*/, const std::vector<std::string>& /*args*/)
    {
        m_running = false;
    }
};

void AthenaExcHandler(athena::error::Level level,
                      const char* file, const char* /*function*/,
                      int line, const char* fmt, ...)
{
    static logvisor::Module Log("heclTest::AthenaExcHandler");
    va_list ap;
    va_start(ap, fmt);
    Log.reportSource(logvisor::Level(level), file, uint32_t(line), fmt, ap);
    va_end(ap);
}

#if !WINDOWS_STORE
#if _WIN32
int wmain(int argc, const boo::SystemChar** argv)
#else
int main(int argc, const boo::SystemChar** argv)
#endif
{
    atSetExceptionHandler(AthenaExcHandler);
    logvisor::RegisterStandardExceptions();
    logvisor::RegisterConsoleLogger();
    HECLApplicationCallback appCb;
    int ret = boo::ApplicationRun(boo::IApplication::EPlatformType::Auto,
        appCb, _S("heclTest"), _S("HECL Test"), argc, argv);
    printf("IM DYING!!\n");
    return ret;
}
#else
using namespace Windows::ApplicationModel::Core;

[Platform::MTAThread]
int WINAPIV main(Platform::Array<Platform::String^>^ params)
{
    logvisor::RegisterStandardExceptions();
    logvisor::RegisterConsoleLogger();
    HECLApplicationCallback appCb;
    boo::ViewProvider^ viewProvider =
        ref new boo::ViewProvider(appCb, _S("heclTest"), _S("HECL Test"), _S("heclTest"), params, false);
    CoreApplication::Run(viewProvider);
    return 0;
}
#endif

#if _WIN32 && !WINDOWS_STORE
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

HECLApplicationCallback::~HECLApplicationCallback()
{

}
