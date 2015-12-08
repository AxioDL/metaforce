#include "ViewManager.hpp"
#include "Specter/Control.hpp"

namespace RUDE
{

struct SetTo1 : Specter::IButtonBinding
{
    ViewManager& m_vm;
    std::string m_name = "SetTo1";
    std::string m_help = "Sets scale factor to 1.0";
    SetTo1(ViewManager& vm) : m_vm(vm) {}

    const std::string& name() const {return m_name;}
    const std::string& help() const {return m_help;}
    void pressed(const boo::SWindowCoord& coord)
    {
        m_vm.rootView().viewRes().resetPixelFactor(1.0);
        m_vm.rootView().resetResources(m_vm.rootView().viewRes());
        m_vm.ResetResources();
        m_vm.rootView().updateSize();
    }
};

struct SetTo2 : Specter::IButtonBinding
{
    ViewManager& m_vm;
    std::string m_name = "SetTo2";
    std::string m_help = "Sets scale factor to 2.0";
    SetTo2(ViewManager& vm) : m_vm(vm) {}

    const std::string& name() const {return m_name;}
    const std::string& help() const {return m_help;}
    void pressed(const boo::SWindowCoord& coord)
    {
        m_vm.rootView().viewRes().resetPixelFactor(2.0);
        m_vm.rootView().resetResources(m_vm.rootView().viewRes());
        m_vm.ResetResources();
        m_vm.rootView().updateSize();
    }
};

void ViewManager::ResetResources()
{
    Specter::MultiLineTextView* textView1 = new Specter::MultiLineTextView(m_viewResources, *m_space1, m_viewResources.m_heading18);
    m_space1->setContentView(std::unique_ptr<Specter::MultiLineTextView>(textView1));

    Specter::MultiLineTextView* textView2 = new Specter::MultiLineTextView(m_viewResources, *m_space2, m_viewResources.m_heading18);
    m_space2->setContentView(std::unique_ptr<Specter::MultiLineTextView>(textView2));

    textView1->typesetGlyphs("Hello, World!\n\n", m_viewResources.themeData().uiText());
    textView2->typesetGlyphs("こんにちは世界！\n\n", m_viewResources.themeData().uiText());

    textView1->setBackground(m_viewResources.themeData().viewportBackground());
    textView2->setBackground(m_viewResources.themeData().viewportBackground());
}

void ViewManager::SetupRootView()
{
    m_rootView.reset(new Specter::RootView(*this, m_viewResources, m_mainWindow.get()));
    Specter::SplitView* splitView = new Specter::SplitView(m_viewResources, *m_rootView, Specter::SplitView::Axis::Horizontal);
    m_rootView->setContentView(std::unique_ptr<Specter::SplitView>(splitView));

    m_test1 = m_cvarManager.newCVar("hello_button", "Help for Hello Button", false,
                                    HECL::CVar::EFlags::Archive | HECL::CVar::EFlags::Editor);
    m_space1 = new Specter::Space(m_viewResources, *splitView, Specter::Toolbar::Position::Top);
    m_space1->toolbar().push_back(std::make_unique<Specter::Button>(m_viewResources, m_space1->toolbar(),
        std::make_unique<SetTo1>(*this), "Hello Button"));

    m_test2 = m_cvarManager.newCVar("hello_button_jp", "Help for Japanese Hello Button", false,
                                    HECL::CVar::EFlags::Archive | HECL::CVar::EFlags::Editor);
    m_space2 = new Specter::Space(m_viewResources, *splitView, Specter::Toolbar::Position::Bottom);
    m_space2->toolbar().push_back(std::make_unique<Specter::Button>(m_viewResources, m_space2->toolbar(),
        std::make_unique<SetTo2>(*this), "こんにちはボタン"));

    splitView->setContentView(0, std::unique_ptr<Specter::Space>(m_space1));
    splitView->setContentView(1, std::unique_ptr<Specter::Space>(m_space2));

    m_rootView->setBackground(Zeus::CColor::skGrey);

    ResetResources();
}

void ViewManager::init(boo::IApplication* app)
{
    m_mainWindow = std::unique_ptr<boo::IWindow>(app->newWindow(_S("RUDE")));
    m_mainWindow->showWindow();
    m_mainWindow->setWaitCursor(true);

    float pixelFactor = 1.0;
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
#if 0
    if (m_cvPixelFactor->isModified())
    {
        float pixelFactor = m_cvPixelFactor->toFloat();
        m_viewResources.resetPixelFactor(pixelFactor);
        m_rootView->resetResources(m_viewResources);
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

