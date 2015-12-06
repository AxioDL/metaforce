#include "ViewManager.hpp"

namespace RUDE
{

void ViewManager::SetupRootView()
{
    m_rootView.reset(new Specter::RootView(*this, m_viewResources, m_mainWindow.get()));
    Specter::SplitView* splitView = new Specter::SplitView(m_viewResources, *m_rootView, Specter::SplitView::Axis::Horizontal);
    m_rootView->setContentView(std::unique_ptr<Specter::SplitView>(splitView));

    m_test1 = m_cvarManager.newCVar("hello_button", "Help for Hello Button", false,
                                    HECL::CVar::EFlags::Archive | HECL::CVar::EFlags::Editor);
    Specter::Space* space1 = new Specter::Space(m_viewResources, *splitView, Specter::Toolbar::Position::Top);
    space1->toolbar().push_back(std::make_unique<Specter::Button>(m_viewResources, space1->toolbar(),
        std::make_unique<Specter::CVarControlBinding>(m_test1), "Hello Button"));
    Specter::MultiLineTextView* textView1 = new Specter::MultiLineTextView(m_viewResources, *space1, m_viewResources.m_heading18);
    space1->setContentView(std::unique_ptr<Specter::MultiLineTextView>(textView1));

    m_test2 = m_cvarManager.newCVar("hello_button_jp", "Help for Japanese Hello Button", false,
                                    HECL::CVar::EFlags::Archive | HECL::CVar::EFlags::Editor);
    Specter::Space* space2 = new Specter::Space(m_viewResources, *splitView, Specter::Toolbar::Position::Bottom);
    space2->toolbar().push_back(std::make_unique<Specter::Button>(m_viewResources, space2->toolbar(),
        std::make_unique<Specter::CVarControlBinding>(m_test2), "こんにちはボタン"));
    Specter::MultiLineTextView* textView2 = new Specter::MultiLineTextView(m_viewResources, *space2, m_viewResources.m_heading18);
    space2->setContentView(std::unique_ptr<Specter::MultiLineTextView>(textView2));

    splitView->setContentView(0, std::unique_ptr<Specter::Space>(space1));
    splitView->setContentView(1, std::unique_ptr<Specter::Space>(space2));

    textView1->typesetGlyphs("Hello, World!\n\n", m_viewResources.themeData().uiText());
    textView2->typesetGlyphs("こんにちは世界！\n\n", m_viewResources.themeData().uiText());

    textView1->setBackground(m_viewResources.themeData().viewportBackground());
    textView2->setBackground(m_viewResources.themeData().viewportBackground());
    m_rootView->setBackground(Zeus::CColor::skGrey);
}

void ViewManager::init(boo::IApplication* app)
{
    m_mainWindow = std::unique_ptr<boo::IWindow>(app->newWindow(_S("RUDE")));
    m_mainWindow->showWindow();
    m_mainWindow->setWaitCursor(true);

    unsigned dpi = m_mainWindow->getVirtualPixelFactor() * 72;
    m_cvDPI = m_cvarManager.newCVar("ed_dpi", "User-selected UI DPI",
                                    int(dpi), HECL::CVar::EFlags::Editor | HECL::CVar::EFlags::Archive);

    boo::IGraphicsDataFactory* gf = m_mainWindow->getMainContextDataFactory();
    m_viewResources.init(gf, &m_fontCache, Specter::ThemeData(), dpi);
    SetupRootView();

    m_mainWindow->setWaitCursor(false);
}

bool ViewManager::proc()
{
    boo::IGraphicsCommandQueue* gfxQ = m_mainWindow->getCommandQueue();
    if (m_rootView->isDestroyed())
        return false;

    if (m_cvDPI->isModified())
    {
        unsigned dpi = m_cvDPI->toInteger();
        m_viewResources.resetDPI(dpi);
        m_rootView->resetResources(m_viewResources);
        m_cvDPI->clearModified();
    }

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

