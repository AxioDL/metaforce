#include <boo/boo.hpp>
#include <LogVisor/LogVisor.hpp>
#include <Athena/MemoryWriter.hpp>
#include "HECL/Runtime.hpp"
#include "HECL/HMDLMeta.hpp"

#include <math.h>
#include <thread>
#include <mutex>
#include <condition_variable>

struct HECLWindowCallback : boo::IWindowCallback
{
    bool m_sizeDirty = false;
    boo::SWindowRect m_latestSize;
    void resized(const boo::SWindowRect& rect)
    {
        m_sizeDirty = true;
        m_latestSize = rect;
    }

    bool m_destroyed = false;
    void destroyed()
    {
        m_destroyed = true;
    }
};

struct HECLApplicationCallback : boo::IApplicationCallback
{
    HECLWindowCallback m_windowCb;
    boo::IWindow* m_mainWindow = nullptr;
    bool m_running = true;

    int appMain(boo::IApplication* app)
    {
        /* Setup boo window */
        m_mainWindow = app->newWindow(_S("HECL Test"));
        m_mainWindow->setCallback(&m_windowCb);

        boo::ITextureR* renderTex = nullptr;
        boo::IGraphicsBufferD* vubo = nullptr;
        boo::IShaderDataBinding* binding = nullptr;

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

        std::mutex initmt;
        std::condition_variable initcv;
        std::mutex loadmt;
        std::condition_variable loadcv;
        std::unique_lock<std::mutex> outerLk(initmt);
        std::thread loaderThr([&]()
        {
            std::unique_lock<std::mutex> innerLk(initmt);
            boo::IGraphicsDataFactory* gfxF = m_mainWindow->getLoadContextDataFactory();

            boo::SWindowRect mainWindowRect = m_mainWindow->getWindowFrame();
            renderTex = gfxF->newRenderTexture(mainWindowRect.size[0], mainWindowRect.size[1], 1);

            /* HECL managers */
            HECL::Runtime::FileStoreManager fileMgr(app->getUniqueName());
            HECL::Runtime::ShaderCacheManager shaderMgr(fileMgr, gfxF);
            shaderMgr.setRenderTargetHint(renderTex);

            /* Compile HECL shader */
            static std::string testShader = "HECLOpaque(Texture(0, UV(0)))";
            HECL::Runtime::ShaderTag testShaderTag(testShader, 0, 1, 0, 0, 0, false, false, false);
            boo::IShaderPipeline* testShaderObj =
            shaderMgr.buildShader(testShaderTag, testShader, "testShader");

            /* Generate meta structure (usually statically serialized) */
            HECL::HMDLMeta testMeta;
            testMeta.topology = HECL::TopologyTriStrips;
            testMeta.vertStride = 32;
            testMeta.vertCount = 4;
            testMeta.indexCount = 4;
            testMeta.colorCount = 0;
            testMeta.uvCount = 1;
            testMeta.weightCount = 0;

            /* Binary form of meta structure */
            atUint8 testMetaBuf[HECL_HMDL_META_SZ];
            Athena::io::MemoryWriter testMetaWriter(testMetaBuf, HECL_HMDL_META_SZ);
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
            HECL::Runtime::HMDLData testData(gfxF, testMetaBuf, quad, ibo);

            /* Make ramp texture */
            using Pixel = uint8_t[4];
            static Pixel tex[256][256];
            for (int i=0 ; i<256 ; ++i)
                for (int j=0 ; j<256 ; ++j)
                {
                    tex[i][j][0] = i;
                    tex[i][j][1] = j;
                    tex[i][j][2] = 0;
                    tex[i][j][3] = 0xff;
                }
            boo::ITexture* texture =
            gfxF->newStaticTexture(256, 256, 1, boo::TextureFormatRGBA8, tex, 256*256*4);

            /* Make vertex uniform buffer */
            vubo = gfxF->newDynamicBuffer(boo::BufferUseUniform, sizeof(VertexUBO), 1);

            /* Assemble data binding */
            binding = testData.newShaderDataBindng(gfxF, testShaderObj, 1, (boo::IGraphicsBuffer**)&vubo, 1, &texture);

            gfxF->commit();

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
                                          m_windowCb.m_latestSize.size[0],
                                          m_windowCb.m_latestSize.size[1]);
                m_windowCb.m_sizeDirty = false;
            }

            gfxQ->setRenderTarget(renderTex);
            boo::SWindowRect r = m_windowCb.m_latestSize;
            r.location[0] = 0;
            r.location[1] = 0;
            gfxQ->setViewport(r);
            float rgba[] = {sinf(frameIdx / 60.0), cosf(frameIdx / 60.0), 0.0, 1.0};
            gfxQ->setClearColor(rgba);
            gfxQ->clearTarget();
            gfxQ->setDrawPrimitive(boo::PrimitiveTriStrips);

            vuboData.modelview[3][0] = sinf(frameIdx / 60.0) * 0.5;
            vuboData.modelview[3][1] = cosf(frameIdx / 60.0) * 0.5;
            vubo->load(&vuboData, sizeof(vuboData));
            gfxQ->flushBufferUpdates();

            gfxQ->setShaderDataBinding(binding);
            gfxQ->draw(0, 4);
            gfxQ->resolveDisplay(renderTex);
            gfxQ->execute();

            ++frameIdx;
        }

        std::unique_lock<std::mutex> finallk(loadmt);
        finallk.unlock();
        loadcv.notify_one();
        loaderThr.join();
        return 0;
    }
    void appQuitting(boo::IApplication* app)
    {
        m_running = false;
    }
};

void AthenaExcHandler(const Athena::error::Level& level,
                      const char* file, const char* function,
                      int line, const char* fmt, ...)
{}

#if _WIN32
int wmain(int argc, const boo::SystemChar** argv)
#else
int main(int argc, const boo::SystemChar** argv)
#endif
{
    atSetExceptionHandler(AthenaExcHandler);
    LogVisor::RegisterConsoleLogger();
    HECLApplicationCallback appCb;
    int ret = boo::ApplicationRun(boo::IApplication::PLAT_AUTO,
        appCb, _S("heclTest"), _S("HECL Test"), argc, argv);
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
