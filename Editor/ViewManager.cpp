#include "ViewManager.hpp"
#include "specter/Control.hpp"
#include "specter/Space.hpp"
#include "specter/Menu.hpp"
#include "SplashScreen.hpp"
#include "locale/locale.hpp"
#include "ResourceBrowser.hpp"
#include "icons/icons.hpp"
#include "badging/Badging.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"
#include "Runtime/Graphics/CModel.hpp"
#include "Runtime/Graphics/CGraphics.hpp"
#include "Runtime/Character/CSkinRules.hpp"
#include <cstdio>

using YAMLNode = athena::io::YAMLNode;

namespace urde
{

URDE_DECL_SPECIALIZE_SHADER(CThermalColdFilter)
URDE_DECL_SPECIALIZE_SHADER(CSpaceWarpFilter)

void ViewManager::BuildTestPART(urde::IObjectStore& objStore)
{
    m_modelTest = objStore.GetObj("gun_cmdl");
    //m_modelTest = objStore.GetObj("CMDL_GameCube");
    m_modelTest.Lock();

    //m_partGenDesc = objStore.GetObj({hecl::FOURCC('PART'), 0x972A5CD2});
    //m_partGenDesc = objStore.GetObj("PowerCharge");
    //m_partGenDesc.Lock();
    //m_partGen.reset(new urde::CElementGen(m_partGenDesc,
    //                                       urde::CElementGen::EModelOrientationType::Normal,
    //                                       urde::CElementGen::EOptionalSystemFlags::None));
    //m_partGen->SetGlobalScale({5.f, 5.f, 5.f});
    m_lineRenderer.reset(new urde::CLineRenderer(urde::CLineRenderer::EPrimitiveMode::LineStrip, 4, nullptr, true));

    m_particleView.reset(new ParticleView(*this, m_viewResources, *m_rootView));

    //m_moviePlayer.reset(new CMoviePlayer("Video/SpecialEnding.thp", 1.f, false, true));
    //m_moviePlayer->SetFrame({-1.0f, 1.0f, 0.f}, {-1.0f, -1.0f, 0.f}, {1.0f, -1.0f, 0.f}, {1.0f, 1.0f, 0.f});
    /*
    CDvdFile testRSF("Audio/frontend_1.rsf");
    u64 rsfLen = testRSF.Length();
    m_rsfBuf.reset(new u8[rsfLen]);
    testRSF.SyncRead(m_rsfBuf.get(), rsfLen);
    */
    //CMoviePlayer::SetStaticAudio(m_rsfBuf.get(), rsfLen, 416480, 1973664);

    m_videoVoice = m_voiceEngine->allocateNewStereoVoice(32000, &m_voiceCallback);
    m_videoVoice->start();

    //m_rootView->accessContentViews().clear();
    m_rootView->accessContentViews().push_back(m_particleView.get());
    m_rootView->updateSize();
}

void ViewManager::InitMP1(MP1::CMain& main)
{
    main.Init(m_mainBooFactory, m_mainCommandQueue, m_renderTex, m_fileStoreManager, m_voiceEngine.get());
}

void ViewManager::ParticleView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    specter::View::resized(root, sub);
    urde::CGraphics::SetViewportResolution({sub.size[0], sub.size[1]});
}

void ViewManager::ParticleView::draw(boo::IGraphicsCommandQueue *gfxQ)
{
    gfxQ->clearTarget(false, true);
    if (m_vm.m_modelTest.IsLoaded())
    {
        CModelFlags flags;

        flags.m_extendedShaderIdx = 2;
        if (std::fmod(m_theta, M_PIF) < M_PIF / 2.f)
            flags.m_extendedShaderIdx = 1;

        m_theta += 0.01f;
        CGraphics::SetModelMatrix(zeus::CTransform::RotateZ(m_theta) * zeus::CTransform::Scale(10.f));
        //CGraphics::SetModelMatrix(zeus::CTransform::Identity());
        CGraphics::SetViewPointMatrix(zeus::lookAt(zeus::CVector3f{0.f, -10.f, 4.f}, {0.f, 0.f, 0.f}));
        boo::SWindowRect windowRect = m_vm.m_mainWindow->getWindowFrame();
        float aspect = windowRect.size[0] / float(windowRect.size[1]);
        CGraphics::SetPerspective(55.0, aspect, 0.1f, 1000.f);

        std::vector<CLight> lights = {CLight::BuildLocalAmbient({}, {0.5f, 0.5f, 0.5f, 1.f}),
                                      CLight::BuildCustom({0.f, -20.f, 10.f}, {0.f, 1.f, 0.f},
                                      {200.f, 200.f, 200.f}, 0.f, 0.f, 1.f, 1.f, 0.f, 0.f),
                                      CLight::BuildDirectional({0.f, 0.f, -1.f}, {0.3f, 0.3f, 0.3f})};
        //lights = {CLight::BuildLocalAmbient({}, {0.0f, 0.0f, 0.0f, 1.f})};
        m_vm.m_modelTest->GetInstance().ActivateLights(lights);
        m_vm.m_modelTest->Draw(flags);

        m_spaceWarpFilter.setStrength(std::sin(m_theta * 5.f) * 0.5f + 0.5f);
        m_spaceWarpFilter.draw(zeus::CVector2f{0.f, 0.f});
    }
    if (m_vm.m_partGen)
    {
        m_vm.m_partGen->Update(1.0 / 60.0);

        if (m_vm.m_partGen->IsSystemDeletable())
            m_vm.m_partGen->Reset();

        CGraphics::SetModelMatrix(zeus::CTransform::Identity());
        CGraphics::SetViewPointMatrix(zeus::CTransform::Identity() + zeus::CVector3f(0.f, -10.f, 0.f));
        boo::SWindowRect windowRect = m_vm.m_mainWindow->getWindowFrame();
        float aspect = windowRect.size[0] / float(windowRect.size[1]);
        CGraphics::SetPerspective(55.0, aspect, 0.1f, 1000.f);
        //gfxQ->clearTarget(false, true);
        m_vm.m_partGen->Render();

        /*
        m_vm.m_lineRenderer->Reset();
        m_vm.m_lineRenderer->AddVertex({-0.5f, 0.f, -0.5f}, zeus::CColor::skBlue, 1.f);
        m_vm.m_lineRenderer->AddVertex({-0.5f, 0.f, 0.5f}, zeus::CColor::skBlue, 1.f);
        m_vm.m_lineRenderer->AddVertex({0.5f, 10.f, 0.5f}, zeus::CColor::skRed, 3.f);
        m_vm.m_lineRenderer->AddVertex({0.5f, 0.f, -0.5f}, zeus::CColor::skBlue, 1.f);
        m_vm.m_lineRenderer->Render();
        */
    }
    if (m_vm.m_moviePlayer)
    {
        if (m_vm.m_moviePlayer->GetIsMovieFinishedPlaying())
        {
            m_vm.m_moviePlayer.reset(new CMoviePlayer("Video/01_startloop.thp", -1.f, true, false));
            m_vm.m_moviePlayer->SetFrame({-1.0f, 1.0f, 0.f}, {-1.0f, -1.0f, 0.f}, {1.0f, -1.0f, 0.f}, {1.0f, 1.0f, 0.f});
        }

        m_vm.m_moviePlayer->Update(1.f / 60.f);
        m_vm.m_moviePlayer->DrawFrame();
    }
}

specter::View* ViewManager::BuildSpaceViews()
{
    m_rootSpaceView = m_rootSpace->buildSpaceView(m_viewResources);
    return m_rootSpaceView;
}

specter::RootView* ViewManager::SetupRootView()
{
    m_rootView.reset(new specter::RootView(*this, m_viewResources, m_mainWindow.get()));
    m_rootView->setBackground(zeus::CColor::skBlack);
    return m_rootView.get();
}

SplashScreen* ViewManager::SetupSplashView()
{
    m_splash.reset(new SplashScreen(*this, m_viewResources));
    if (!m_showSplash)
        m_splash->close(true);
    return m_splash.get();
}

void ViewManager::RootSpaceViewBuilt(specter::View *view)
{
    std::vector<specter::View*>& cViews = m_rootView->accessContentViews();
    cViews.clear();
    cViews.push_back(view);
    cViews.push_back(m_splash.get());
    m_rootView->updateSize();
}

void ViewManager::ProjectChanged(hecl::Database::Project& proj)
{
    CDvdFile::Shutdown();
    CDvdFile::Initialize(hecl::ProjectPath(proj.getProjectWorkingPath(), _S("out/MP1")));
}

void ViewManager::SetupEditorView()
{
    m_rootSpace.reset(new RootSpace(*this));

    SplitSpace* split = new SplitSpace(*this, nullptr, specter::SplitView::Axis::Horizontal);
    m_rootSpace->setChild(std::unique_ptr<Space>(split));
    split->setChildSlot(0, std::make_unique<ResourceBrowser>(*this, split));
    split->setChildSlot(1, std::make_unique<ResourceBrowser>(*this, split));

    BuildSpaceViews();
}

void ViewManager::SetupEditorView(ConfigReader& r)
{
    m_rootSpace.reset(Space::NewRootSpaceFromConfigStream(*this, r));
    BuildSpaceViews();
}

void ViewManager::SaveEditorView(ConfigWriter& w)
{
    if (!m_rootSpace)
        return;
    m_rootSpace->saveState(w);
}

void ViewManager::DismissSplash()
{
    if (!m_showSplash)
        return;
    m_showSplash = false;
    m_splash->close();
}

ViewManager::ViewManager(hecl::Runtime::FileStoreManager& fileMgr, hecl::CVarManager& cvarMgr)
: m_fileStoreManager(fileMgr), m_cvarManager(cvarMgr), m_projManager(*this),
  m_fontCache(fileMgr), m_translator(urde::SystemLocaleOrEnglish()), m_voiceCallback(*this),
  m_recentProjectsPath(hecl::SysFormat(_S("%s/recent_projects.txt"), fileMgr.getStoreRoot().c_str())),
  m_recentFilesPath(hecl::SysFormat(_S("%s/recent_files.txt"), fileMgr.getStoreRoot().c_str()))
{
    Space::SpaceMenuNode::InitializeStrings(*this);
    char path[2048];
    hecl::Sstat theStat;

    FILE* fp = hecl::Fopen(m_recentProjectsPath.c_str(), _S("r"), hecl::FileLockType::Read);
    if (fp)
    {
        while (fgets(path, 2048, fp))
        {
            std::string pathStr(path);
            pathStr.pop_back();
            hecl::SystemStringView pathStrView(pathStr);
            if (!hecl::Stat(pathStrView.c_str(), &theStat) && S_ISDIR(theStat.st_mode))
                m_recentProjects.push_back(pathStrView);
        }
        fclose(fp);
    }

    fp = hecl::Fopen(m_recentFilesPath.c_str(), _S("r"), hecl::FileLockType::Read);
    if (fp)
    {
        while (fgets(path, 2048, fp))
        {
            std::string pathStr(path);
            pathStr.pop_back();
            hecl::SystemStringView pathStrView(pathStr);
            if (!hecl::Stat(pathStrView.c_str(), &theStat) && S_ISDIR(theStat.st_mode))
                m_recentFiles.push_back(pathStrView);
        }
        fclose(fp);
    }
}

ViewManager::~ViewManager() {}

void ViewManager::pushRecentProject(const hecl::SystemString& path)
{
    for (hecl::SystemString& testPath : m_recentProjects)
    {
        if (path == testPath)
            return;
    }
    m_recentProjects.push_back(path);
    FILE* fp = hecl::Fopen(m_recentProjectsPath.c_str(), _S("w"), hecl::FileLockType::Write);
    if (fp)
    {
        for (hecl::SystemString& pPath : m_recentProjects)
            fprintf(fp, "%s\n", hecl::SystemUTF8View(pPath).c_str());
        fclose(fp);
    }
}

void ViewManager::pushRecentFile(const hecl::SystemString& path)
{
    for (hecl::SystemString& testPath : m_recentFiles)
    {
        if (path == testPath)
            return;
    }
    m_recentFiles.push_back(path);
    FILE* fp = hecl::Fopen(m_recentFilesPath.c_str(), _S("w"), hecl::FileLockType::Write);
    if (fp)
    {
        for (hecl::SystemString& pPath : m_recentFiles)
            fprintf(fp, "%s\n", hecl::SystemUTF8View(pPath).c_str());
        fclose(fp);
    }}

void ViewManager::init(boo::IApplication* app)
{
    m_mainWindow = std::unique_ptr<boo::IWindow>(app->newWindow(_S("URDE"), 1));
    m_mainWindow->showWindow();
    m_mainWindow->setWaitCursor(true);

    float pixelFactor = 1.0;

    m_mainBooFactory = m_mainWindow->getMainContextDataFactory();
    m_mainCommandQueue = m_mainWindow->getCommandQueue();
    m_viewResources.init(m_mainBooFactory, &m_fontCache, &m_themeData, pixelFactor);
    m_iconsToken = InitializeIcons(m_viewResources);
    m_badgeToken = InitializeBadging(m_viewResources);
    m_viewResources.prepFontCacheAsync(m_mainWindow.get());
    specter::RootView* root = SetupRootView();
    m_showSplash = true;
    root->accessContentViews().push_back(SetupSplashView());
    root->updateSize();
    m_renderTex = root->renderTex();
    m_mainWindow->setWaitCursor(false);
    m_voiceEngine = boo::NewAudioVoiceEngine();
    /*
    CGraphics::InitializeBoo(gf, m_mainWindow->getCommandQueue(), root->renderTex());
    CModelShaders::Initialize(m_fileStoreManager, gf);
    CElementGen::Initialize();
    CMoviePlayer::Initialize();
    CLineRenderer::Initialize();
    */
}

bool ViewManager::proc()
{
    boo::IGraphicsCommandQueue* gfxQ = m_mainWindow->getCommandQueue();
    if (m_rootView->isDestroyed())
        return false;

    if (m_updatePf)
    {
        m_viewResources.resetPixelFactor(m_reqPf);
        specter::RootView* root = SetupRootView();
        if (m_rootSpace)
            BuildSpaceViews();
        else
        {
            std::vector<specter::View*>& cViews = m_rootView->accessContentViews();
            cViews.push_back(SetupSplashView());
        }
        root->updateSize();
        m_updatePf = false;
    }

    m_rootView->dispatchEvents();
    m_rootView->internalThink();
    if (m_rootSpace)
        m_rootSpace->think();
    if (m_splash)
        m_splash->think();

    if (m_deferSplit)
    {
        SplitSpace* ss = static_cast<SplitSpace*>(m_deferSplit->spaceSplit(m_deferSplitAxis, m_deferSplitThisSlot));
        m_rootView->startSplitDrag(ss->splitView(), m_deferSplitCoord);
        m_deferSplit = nullptr;
    }

    ++m_editorFrames;
    if (m_rootSpaceView && m_editorFrames <= 30)
        m_rootSpaceView->setMultiplyColor(zeus::CColor::lerp({1,1,1,0}, {1,1,1,1}, m_editorFrames / 30.0));

    m_projManager.mainUpdate();

    m_rootView->draw(gfxQ);
    CGraphics::EndScene();
    gfxQ->execute();
    m_voiceEngine->pumpAndMixVoices();
    m_projManager.asyncIdle();
    m_mainWindow->waitForRetrace();
    CGraphics::TickRenderTimings();

    return true;
}

void ViewManager::stop()
{
    m_videoVoice.reset();
    m_projManager.shutdown();
    TShader<CThermalColdFilter>::Shutdown();
    TShader<CSpaceWarpFilter>::Shutdown();
    CElementGen::Shutdown();
    CMoviePlayer::Shutdown();
    CLineRenderer::Shutdown();
    CDvdFile::Shutdown();
    m_iconsToken.doDestroy();
    m_viewResources.destroyResData();
    m_fontCache.destroyAtlases();
    m_mainWindow->getCommandQueue()->stopRenderer();
}

}

