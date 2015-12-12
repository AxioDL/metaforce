#include "ViewManager.hpp"
#include "Specter/Control.hpp"
#include "Specter/Space.hpp"

using YAMLNode = Athena::io::YAMLNode;

namespace RUDE
{

Specter::View* ViewManager::BuildSpaceViews(RUDE::Space* space)
{
    Specter::Space* sspace = space->buildSpace(m_viewResources);
    Specter::View* sview = space->buildContent(m_viewResources);
    sspace->setContentView(sview);
    if (space->usesToolbar())
        space->buildToolbar(m_viewResources, sspace->toolbar());
    return sspace;
}

void ViewManager::SetupRootView()
{
    m_rootView.reset(new Specter::RootView(*this, m_viewResources, m_mainWindow.get()));
    m_rootView->setBackground(Zeus::CColor::skGrey);
    m_rootView->setContentView(m_split.buildContent(m_viewResources));
    m_split.m_splitView->setContentView(0, BuildSpaceViews(&m_space1));
    m_split.m_splitView->setContentView(1, BuildSpaceViews(&m_space2));
    m_rootView->updateSize();
}

void ViewManager::init(boo::IApplication* app)
{
    m_mainWindow = std::unique_ptr<boo::IWindow>(app->newWindow(_S("RUDE")));
    m_mainWindow->showWindow();
    m_mainWindow->setWaitCursor(true);

    float pixelFactor = 2.0;
    m_cvPixelFactor = m_cvarManager.newCVar("ed_pixelfactor", "User-selected UI Scale",
                                    pixelFactor, HECL::CVar::EFlags::Editor | HECL::CVar::EFlags::Archive);

    boo::IGraphicsDataFactory* gf = m_mainWindow->getMainContextDataFactory();
    m_viewResources.init(gf, &m_fontCache, Specter::ThemeData(), pixelFactor);
    SetupRootView();

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
        m_updatePf = false;
    }
#if 0
    if (m_cvPixelFactor->isModified())
    {
        float pixelFactor = m_cvPixelFactor->toFloat();
        m_viewResources.resetPixelFactor(pixelFactor);
        m_cvPixelFactor->clearModified();
    }
#endif
    m_rootView->dispatchEvents();
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

