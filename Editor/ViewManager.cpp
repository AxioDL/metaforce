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
#include "Graphics/CMetroidModelInstance.hpp"
#include "World/CWorldTransManager.hpp"
#include "Graphics/Shaders/CColoredQuadFilter.hpp"
#include "Graphics/Shaders/CTexturedQuadFilter.hpp"
#include "Audio/CStreamAudioManager.hpp"
#include <cstdio>

using YAMLNode = athena::io::YAMLNode;

namespace urde
{

void ViewManager::InitMP1(MP1::CMain& main)
{
    main.Init(m_fileStoreManager, m_mainWindow.get(), m_voiceEngine.get(), *m_amuseAllocWrapper);
    main.WarmupShaders();

    m_testGameView.reset(new TestGameView(*this, m_viewResources, *m_rootView));

    m_rootView->accessContentViews().clear();
    m_rootView->accessContentViews().push_back(m_testGameView.get());
    m_rootView->updateSize();
}

void ViewManager::TestGameView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    specter::View::resized(root, sub);
    urde::CGraphics::SetViewportResolution({sub.size[0], sub.size[1]});
}

void ViewManager::TestGameView::draw(boo::IGraphicsCommandQueue* gfxQ)
{
    m_vm.m_projManager.mainDraw();
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
  m_fontCache(fileMgr), m_translator(urde::SystemLocaleOrEnglish()),
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
    m_mainWindow = app->newWindow(_S("URDE"), 1);
    m_mainWindow->showWindow();
    m_mainWindow->setWaitCursor(true);

    float pixelFactor = m_mainWindow->getVirtualPixelFactor();

    m_mainBooFactory = m_mainWindow->getMainContextDataFactory();
    m_mainPlatformName = m_mainBooFactory->platformName();
    m_mainWindow->setTitle(_S("URDE [") + hecl::SystemString(m_mainPlatformName) + _S("]"));
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
    m_amuseAllocWrapper.emplace(*m_voiceEngine);

    for (const auto& arg : app->getArgs())
    {
        if (hecl::SearchForProject(arg))
        {
            m_deferedProject = arg;
            break;
        }
    }
}

bool ViewManager::proc()
{
    if (!m_deferedProject.empty() && m_viewResources.fontCacheReady())
    {
        m_projManager.openProject(m_deferedProject);
        m_deferedProject.clear();
    }

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

    if (g_Renderer)
        g_Renderer->BeginScene();
    m_rootView->draw(gfxQ);
    if (g_Renderer)
        g_Renderer->EndScene();
    gfxQ->execute();
    m_projManager.asyncIdle();
    m_mainWindow->waitForRetrace(m_voiceEngine.get());
    CBooModel::ClearModelUniformCounters();
    CGraphics::TickRenderTimings();
    return true;
}

void ViewManager::stop()
{
    m_videoVoice.reset();
    m_projManager.shutdown();
    CDvdFile::Shutdown();
    m_iconsToken.doDestroy();
    m_viewResources.destroyResData();
    m_fontCache.destroyAtlases();
    m_mainWindow->getCommandQueue()->stopRenderer();
}

}

