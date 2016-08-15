#include "MP1.hpp"
#include "Graphics/Shaders/CModelShaders.hpp"
#include "Graphics/Shaders/CThermalColdFilter.hpp"
#include "Graphics/Shaders/CThermalHotFilter.hpp"
#include "Graphics/Shaders/CSpaceWarpFilter.hpp"

namespace urde
{
URDE_DECL_SPECIALIZE_SHADER(CThermalColdFilter)
URDE_DECL_SPECIALIZE_SHADER(CThermalHotFilter)
URDE_DECL_SPECIALIZE_SHADER(CSpaceWarpFilter)

namespace MP1
{

CMain::CMain(IFactory& resFactory, CSimplePool& resStore)
: x128_globalObjects(resFactory, resStore)
{
    xe4_gameplayResult = EGameplayResult::Playing;
    g_Main = this;
}

void CMain::RegisterResourceTweaks()
{
}
void CMain::ResetGameState()
{
}

void CMain::InitializeSubsystems(boo::IGraphicsDataFactory* factory,
                                 boo::IGraphicsCommandQueue* cc,
                                 boo::ITextureR* renderTex,
                                 const hecl::Runtime::FileStoreManager& storeMgr,
                                 boo::IAudioVoiceEngine* voiceEngine)
{
    CGraphics::InitializeBoo(factory, cc, renderTex);
    CModelShaders::Initialize(storeMgr, factory);
    TShader<CThermalColdFilter>::Initialize();
    TShader<CThermalHotFilter>::Initialize();
    TShader<CSpaceWarpFilter>::Initialize();
    CMoviePlayer::Initialize();
    CLineRenderer::Initialize();
    CElementGen::Initialize();
    CAnimData::InitializeCache();
    CDecalManager::Initialize();
}

void CMain::FillInAssetIDs()
{
}
void CMain::LoadAudio()
{
}

void CMain::Init(boo::IGraphicsDataFactory* factory,
                 boo::IGraphicsCommandQueue* cc,
                 boo::ITextureR* renderTex,
                 const hecl::Runtime::FileStoreManager& storeMgr,
                 boo::IAudioVoiceEngine* voiceEngine)
{
    InitializeSubsystems(factory, cc, renderTex, storeMgr, voiceEngine);
    x128_globalObjects.PostInitialize();
    x70_tweaks.RegisterTweaks();
    //g_TweakManager->ReadFromMemoryCard("AudioTweaks");
    FillInAssetIDs();
}

bool CMain::Proc()
{
    xe8_b24_finished = m_archSupport.Update();
    return xe8_b24_finished;
}

void CMain::Shutdown()
{
}

#if MP1_USE_BOO

int CMain::appMain(boo::IApplication* app)
{
    zeus::detectCPU();
    mainWindow = app->newWindow(_S("Metroid Prime 1 Reimplementation vZygote"), 1);
    mainWindow->showWindow();
    TOneStatic<CGameGlobalObjects> globalObjs;
    InitializeSubsystems();
    globalObjs->PostInitialize();
    x70_tweaks.RegisterTweaks();
    AddWorldPaks();
    g_TweakManager->ReadFromMemoryCard("AudioTweaks");
    FillInAssetIDs();
    TOneStatic<CGameArchitectureSupport> archSupport;
    mainWindow->setCallback(archSupport.GetAllocSpace());

    boo::IGraphicsCommandQueue* gfxQ = mainWindow->getCommandQueue();
    boo::SWindowRect windowRect = mainWindow->getWindowFrame();
    boo::ITextureR* renderTex;
    boo::GraphicsDataToken data = mainWindow->getMainContextDataFactory()->commitTransaction(
    [&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        renderTex = ctx.newRenderTexture(windowRect.size[0], windowRect.size[1], true, true);
        return true;
    });
    float rgba[4] = { 0.2f, 0.2f, 0.2f, 1.0f};
    gfxQ->setClearColor(rgba);

    while (!xe8_b24_finished)
    {
        xe8_b24_finished = archSupport->Update();

        if (archSupport->isRectDirty())
        {
            const boo::SWindowRect& windowRect = archSupport->getWindowRect();
            gfxQ->resizeRenderTexture(renderTex,
                                      windowRect.size[0],
                                      windowRect.size[1]);
        }

        gfxQ->setRenderTarget(renderTex);
        gfxQ->clearTarget();
        gfxQ->resolveDisplay(renderTex);
        gfxQ->execute();
        mainWindow->waitForRetrace();
    }
    return 0;
}

#endif

}
}
