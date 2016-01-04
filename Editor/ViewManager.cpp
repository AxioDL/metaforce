#include "ViewManager.hpp"
#include "Specter/Control.hpp"
#include "Specter/Space.hpp"
#include "SplashScreen.hpp"
#include "locale/locale.hpp"

using YAMLNode = Athena::io::YAMLNode;

namespace RUDE
{

Specter::View* ViewManager::BuildSpaceViews(RUDE::Space* space)
{
    Specter::Space* sspace = space->buildSpaceView(m_viewResources);
    Specter::View* sview = space->buildContentView(m_viewResources);
    sspace->setContentView(sview);
    if (space->usesToolbar())
        space->buildToolbarView(m_viewResources, sspace->toolbar());
    return sspace;
}

void ViewManager::SetupRootView()
{
    m_rootView.reset(new Specter::RootView(*this, m_viewResources, m_mainWindow.get()));
    m_rootView->setBackground(Zeus::CColor::skBlack);
    m_rootView->updateSize();
}

void ViewManager::SetupSplashView()
{
    m_splash.reset(new SplashScreen(*this, m_viewResources));
    m_rootView->setContentView(m_splash.get());
    m_rootView->updateSize();
    m_showSplash = true;
}

void ViewManager::SetupEditorView()
{
    m_rootSpace.reset(new SplitSpace(*this));
    m_rootView->setContentView(BuildSpaceViews(m_rootSpace.get()));
    m_rootView->updateSize();
    m_showSplash = false;
}

void ViewManager::SetupEditorView(Athena::io::YAMLDocReader& r)
{
    m_rootSpace.reset(Space::NewSpaceFromYAMLStream(*this, r));
    m_rootView->setContentView(BuildSpaceViews(m_rootSpace.get()));
    m_rootView->updateSize();
    m_showSplash = false;
}

ViewManager::ViewManager(HECL::Runtime::FileStoreManager& fileMgr, HECL::CVarManager& cvarMgr)
: m_fileStoreManager(fileMgr), m_cvarManager(cvarMgr), m_projManager(*this),
  m_fontCache(fileMgr), m_translator(RUDE::SystemLocaleOrEnglish())
{}

ViewManager::~ViewManager() {}

void ViewManager::pushRecentProject(const HECL::SystemString& path)
{
    m_recentProjects.push_back(path);
}

void ViewManager::pushRecentFile(const HECL::SystemString& path)
{
    m_recentFiles.push_back(path);
}

void ViewManager::init(boo::IApplication* app)
{
    m_mainWindow = std::unique_ptr<boo::IWindow>(app->newWindow(_S("RUDE")));
    m_mainWindow->showWindow();
    m_mainWindow->setWaitCursor(true);

    float pixelFactor = 1.0;

    boo::IGraphicsDataFactory* gf = m_mainWindow->getMainContextDataFactory();
    m_viewResources.init(gf, &m_fontCache, Specter::ThemeData(), pixelFactor);
    m_viewResources.prepFontCacheAsync(m_mainWindow.get());
    SetupRootView();
    SetupSplashView();

    m_mainWindow->setWaitCursor(false);
}

bool ViewManager::proc()
{
    boo::IGraphicsCommandQueue* gfxQ = m_mainWindow->getCommandQueue();
    if (m_rootView->isDestroyed())
        return false;
    if (m_updatePf)
    {
        m_viewResources.resetPixelFactor(m_reqPf);
        SetupRootView();
        if (m_showSplash)
            SetupSplashView();
        else
            SetupEditorView();
        m_updatePf = false;
    }
    m_rootView->dispatchEvents();
    if (m_showSplash)
        m_splash->think();
    m_rootView->draw(gfxQ);
    gfxQ->execute();
    m_mainWindow->waitForRetrace();

    return true;
}

void ViewManager::stop()
{
    m_mainWindow->getCommandQueue()->stopRenderer();
}

}

