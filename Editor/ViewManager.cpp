#include "ViewManager.hpp"
#include "Specter/Control.hpp"

namespace RUDE
{

void ViewManager::SetupRootView()
{
    m_rootView.reset(new Specter::RootView(*this, m_viewResources, m_mainWindow.get()));
    m_splitView.reset(new Specter::SplitView(m_viewResources, *m_rootView, Specter::SplitView::Axis::Horizontal));
    m_rootView->setContentView(m_splitView.get());

    m_space1.reset(new Specter::Space(m_viewResources, *m_splitView, Specter::Toolbar::Position::Top));
    m_butt1.reset(new Specter::Button(m_viewResources, m_space1->toolbar(),
                                      &m_setTo1, "Hello Button"));
    m_space1->toolbar().push_back(m_butt1.get());

    m_space2.reset(new Specter::Space(m_viewResources, *m_splitView, Specter::Toolbar::Position::Bottom));
    m_butt2.reset(new Specter::Button(m_viewResources, m_space2->toolbar(),
                                      &m_setTo2, "こんにちはボタン"));
    m_space2->toolbar().push_back(m_butt2.get());

    m_splitView->setContentView(0, m_space1.get());
    m_splitView->setContentView(1, m_space2.get());

    m_rootView->setBackground(Zeus::CColor::skGrey);

    m_textView1.reset(new Specter::MultiLineTextView(m_viewResources, *m_space1, m_viewResources.m_heading18));
    m_space1->setContentView(m_textView1.get());

    m_textView2.reset(new Specter::MultiLineTextView(m_viewResources, *m_space2, m_viewResources.m_heading18));
    m_space2->setContentView(m_textView2.get());

    m_textView1->typesetGlyphs("Hello, World!\n\n", m_viewResources.themeData().uiText());
    m_textView2->typesetGlyphs("こんにちは世界！\n\n", m_viewResources.themeData().uiText());

    m_textView1->setBackground(m_viewResources.themeData().viewportBackground());
    m_textView2->setBackground(m_viewResources.themeData().viewportBackground());

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

