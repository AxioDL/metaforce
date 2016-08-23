#include "MP1.hpp"
#include "Graphics/Shaders/CModelShaders.hpp"
#include "Graphics/Shaders/CThermalColdFilter.hpp"
#include "Graphics/Shaders/CThermalHotFilter.hpp"
#include "Graphics/Shaders/CSpaceWarpFilter.hpp"
#include "Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Graphics/Shaders/CTexturedQuadFilter.hpp"
#include "Graphics/Shaders/CCameraBlurFilter.hpp"
#include "Graphics/Shaders/CXRayBlurFilter.hpp"
#include "Character/CCharLayoutInfo.hpp"

namespace urde
{
URDE_DECL_SPECIALIZE_SHADER(CThermalColdFilter)
URDE_DECL_SPECIALIZE_SHADER(CThermalHotFilter)
URDE_DECL_SPECIALIZE_SHADER(CSpaceWarpFilter)
URDE_DECL_SPECIALIZE_SHADER(CCameraBlurFilter)
URDE_DECL_SPECIALIZE_SHADER(CXRayBlurFilter)
URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CColoredQuadFilter)
URDE_DECL_SPECIALIZE_MULTI_BLEND_SHADER(CTexturedQuadFilter)

namespace MP1
{

CMain::CMain(IFactory& resFactory, CSimplePool& resStore,
             boo::IGraphicsDataFactory* gfxFactory,
             boo::IGraphicsCommandQueue* cmdQ,
             boo::ITextureR* spareTex)
: m_booSetter(gfxFactory, cmdQ, spareTex), x128_globalObjects(resFactory, resStore)
{
    xe4_gameplayResult = EGameplayResult::Playing;
    g_Main = this;
}

CMain::BooSetter::BooSetter(boo::IGraphicsDataFactory* factory,
                            boo::IGraphicsCommandQueue* cmdQ,
                            boo::ITextureR* spareTex)
{
    CGraphics::InitializeBoo(factory, cmdQ, spareTex);
    TShader<CThermalColdFilter>::Initialize();
    TShader<CThermalHotFilter>::Initialize();
    TShader<CSpaceWarpFilter>::Initialize();
    TShader<CCameraBlurFilter>::Initialize();
    TShader<CXRayBlurFilter>::Initialize();
    TMultiBlendShader<CColoredQuadFilter>::Initialize();
    TMultiBlendShader<CTexturedQuadFilter>::Initialize();
}

void CMain::RegisterResourceTweaks()
{
}
void CMain::ResetGameState()
{
}

void CMain::InitializeSubsystems(const hecl::Runtime::FileStoreManager& storeMgr,
                                 boo::IAudioVoiceEngine* voiceEngine)
{
    CModelShaders::Initialize(storeMgr, CGraphics::g_BooFactory);
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

void CMain::Init(const hecl::Runtime::FileStoreManager& storeMgr,
                 boo::IAudioVoiceEngine* voiceEngine)
{
    InitializeSubsystems(storeMgr, voiceEngine);
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
    TShader<CThermalColdFilter>::Shutdown();
    TShader<CThermalHotFilter>::Shutdown();
    TShader<CSpaceWarpFilter>::Shutdown();
    TShader<CCameraBlurFilter>::Shutdown();
    TShader<CXRayBlurFilter>::Shutdown();
    TMultiBlendShader<CColoredQuadFilter>::Shutdown();
    TMultiBlendShader<CTexturedQuadFilter>::Shutdown();
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
