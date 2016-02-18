#include "ViewManager.hpp"
#include "Specter/Control.hpp"
#include "Specter/Space.hpp"
#include "Specter/Menu.hpp"
#include "SplashScreen.hpp"
#include "locale/locale.hpp"
#include "ResourceBrowser.hpp"
#include "icons/icons.hpp"
#include "Runtime/Particle/CGenDescription.hpp"
#include "Runtime/Particle/CElectricDescription.hpp"
#include "Runtime/Particle/CSwooshDescription.hpp"
#include "Runtime/CModel.hpp"
#include "Runtime/CGraphics.hpp"
#include <cstdio>

using YAMLNode = Athena::io::YAMLNode;

namespace URDE
{

void ViewManager::BuildTestPART(pshag::IObjectStore& objStore)
{
    m_partGenDesc = objStore.GetObj({HECL::FOURCC('PART'), 0x1E348530});
    m_partGen.reset(new pshag::CElementGen(m_partGenDesc,
                                           pshag::CElementGen::EModelOrientationType::Normal,
                                           pshag::CElementGen::EOptionalSystemFlags::None));
    m_particleView.reset(new ParticleView(*this, m_viewResources, *m_rootView));
    m_lineRenderer.reset(new pshag::CLineRenderer(pshag::CLineRenderer::EPrimitiveMode::LineStrip, 4, nullptr, true));

    m_rootView->accessContentViews().push_back(m_particleView.get());
    m_rootView->updateSize();
}

void ViewManager::ParticleView::resized(const boo::SWindowRect& root, const boo::SWindowRect& sub)
{
    Specter::View::resized(root, sub);
    pshag::CGraphics::SetViewportResolution({sub.size[0], sub.size[1]});
}

void ViewManager::ParticleView::draw(boo::IGraphicsCommandQueue *gfxQ)
{
    if (m_vm.m_partGen)
    {
        m_vm.m_partGen->Update(1.0 / 60.0);
        pshag::CGraphics::SetModelMatrix(Zeus::CTransform::Identity());
        pshag::CGraphics::SetViewPointMatrix(Zeus::CTransform::Identity() + Zeus::CVector3f(0.f, -10.f, 0.f));
        boo::SWindowRect windowRect = m_vm.m_mainWindow->getWindowFrame();
        float aspect = windowRect.size[0] / float(windowRect.size[1]);
        pshag::CGraphics::SetPerspective(55.0, aspect, 0.001f, 1000.f);
        //gfxQ->clearTarget(false, true);
        m_vm.m_partGen->Render();

        /*
        m_vm.m_lineRenderer->Reset();
        m_vm.m_lineRenderer->AddVertex({-0.5f, 0.f, -0.5f}, Zeus::CColor::skBlue, 1.f);
        m_vm.m_lineRenderer->AddVertex({-0.5f, 0.f, 0.5f}, Zeus::CColor::skBlue, 1.f);
        m_vm.m_lineRenderer->AddVertex({0.5f, 10.f, 0.5f}, Zeus::CColor::skRed, 3.f);
        m_vm.m_lineRenderer->AddVertex({0.5f, 0.f, -0.5f}, Zeus::CColor::skBlue, 1.f);
        m_vm.m_lineRenderer->Render();
        */
    }
}

Specter::View* ViewManager::BuildSpaceViews()
{
    m_rootSpaceView = m_rootSpace->buildSpaceView(m_viewResources);
    return m_rootSpaceView;
}

Specter::RootView* ViewManager::SetupRootView()
{
    m_rootView.reset(new Specter::RootView(*this, m_viewResources, m_mainWindow.get()));
    m_rootView->setBackground(Zeus::CColor::skBlack);
    return m_rootView.get();
}

SplashScreen* ViewManager::SetupSplashView()
{
    m_splash.reset(new SplashScreen(*this, m_viewResources));
    if (!m_showSplash)
        m_splash->close(true);
    return m_splash.get();
}

void ViewManager::RootSpaceViewBuilt(Specter::View *view)
{
    std::vector<Specter::View*>& cViews = m_rootView->accessContentViews();
    cViews.clear();
    cViews.push_back(view);
    cViews.push_back(m_splash.get());
    m_rootView->updateSize();
}

void ViewManager::SetupEditorView()
{
    m_rootSpace.reset(new RootSpace(*this));

    SplitSpace* split = new SplitSpace(*this, nullptr, Specter::SplitView::Axis::Horizontal);
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

ViewManager::ViewManager(HECL::Runtime::FileStoreManager& fileMgr, HECL::CVarManager& cvarMgr)
: m_fileStoreManager(fileMgr), m_cvarManager(cvarMgr), m_projManager(*this),
  m_fontCache(fileMgr), m_translator(URDE::SystemLocaleOrEnglish()),
  m_recentProjectsPath(HECL::SysFormat(_S("%s/recent_projects.txt"), fileMgr.getStoreRoot().c_str())),
  m_recentFilesPath(HECL::SysFormat(_S("%s/recent_files.txt"), fileMgr.getStoreRoot().c_str()))
{
    Space::SpaceMenuNode::InitializeStrings(*this);
    char path[2048];
    HECL::Sstat theStat;

    FILE* fp = HECL::Fopen(m_recentProjectsPath.c_str(), _S("r"), HECL::FileLockType::Read);
    if (fp)
    {
        while (fgets(path, 2048, fp))
        {
            std::string pathStr(path);
            pathStr.pop_back();
            HECL::SystemStringView pathStrView(pathStr);
            if (!HECL::Stat(pathStrView.c_str(), &theStat) && S_ISDIR(theStat.st_mode))
                m_recentProjects.push_back(pathStrView);
        }
        fclose(fp);
    }

    fp = HECL::Fopen(m_recentFilesPath.c_str(), _S("r"), HECL::FileLockType::Read);
    if (fp)
    {
        while (fgets(path, 2048, fp))
        {
            std::string pathStr(path);
            pathStr.pop_back();
            HECL::SystemStringView pathStrView(pathStr);
            if (!HECL::Stat(pathStrView.c_str(), &theStat) && S_ISDIR(theStat.st_mode))
                m_recentFiles.push_back(pathStrView);
        }
        fclose(fp);
    }
}

ViewManager::~ViewManager() {}

void ViewManager::pushRecentProject(const HECL::SystemString& path)
{
    for (HECL::SystemString& testPath : m_recentProjects)
    {
        if (path == testPath)
            return;
    }
    m_recentProjects.push_back(path);
    FILE* fp = HECL::Fopen(m_recentProjectsPath.c_str(), _S("w"), HECL::FileLockType::Write);
    if (fp)
    {
        for (HECL::SystemString& pPath : m_recentProjects)
            fprintf(fp, "%s\n", HECL::SystemUTF8View(pPath).c_str());
        fclose(fp);
    }
}

void ViewManager::pushRecentFile(const HECL::SystemString& path)
{
    for (HECL::SystemString& testPath : m_recentFiles)
    {
        if (path == testPath)
            return;
    }
    m_recentFiles.push_back(path);
    FILE* fp = HECL::Fopen(m_recentFilesPath.c_str(), _S("w"), HECL::FileLockType::Write);
    if (fp)
    {
        for (HECL::SystemString& pPath : m_recentFiles)
            fprintf(fp, "%s\n", HECL::SystemUTF8View(pPath).c_str());
        fclose(fp);
    }}

void ViewManager::init(boo::IApplication* app)
{
    m_mainWindow = std::unique_ptr<boo::IWindow>(app->newWindow(_S("URDE")));
    m_mainWindow->showWindow();
    m_mainWindow->setWaitCursor(true);

    float pixelFactor = 1.0;

    boo::IGraphicsDataFactory* gf = m_mainWindow->getMainContextDataFactory();
    m_viewResources.init(gf, &m_fontCache, &m_themeData, pixelFactor);
    m_iconsToken = InitializeIcons(m_viewResources);
    m_viewResources.prepFontCacheAsync(m_mainWindow.get());
    Specter::RootView* root = SetupRootView();
    m_showSplash = true;
    root->accessContentViews().push_back(SetupSplashView());
    root->updateSize();

    m_mainWindow->setWaitCursor(false);

    pshag::CGraphics::InitializeBoo(gf, m_mainWindow->getCommandQueue());
    pshag::CElementGen::Initialize();
    pshag::CLineRenderer::Initialize();
}

bool ViewManager::proc()
{
    boo::IGraphicsCommandQueue* gfxQ = m_mainWindow->getCommandQueue();
    if (m_rootView->isDestroyed())
        return false;

    if (m_updatePf)
    {
        m_viewResources.resetPixelFactor(m_reqPf);
        Specter::RootView* root = SetupRootView();
        if (m_rootSpace)
            BuildSpaceViews();
        else
        {
            std::vector<Specter::View*>& cViews = m_rootView->accessContentViews();
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
        m_rootSpaceView->setMultiplyColor(Zeus::CColor::lerp({1,1,1,0}, {1,1,1,1}, m_editorFrames / 30.0));

    m_rootView->draw(gfxQ);
    gfxQ->execute();
    m_mainWindow->waitForRetrace();

    return true;
}

void ViewManager::stop()
{
    pshag::CElementGen::Shutdown();
    pshag::CLineRenderer::Shutdown();
    m_mainWindow->getCommandQueue()->stopRenderer();
}

}

