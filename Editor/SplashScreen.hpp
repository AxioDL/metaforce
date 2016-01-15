#ifndef URDE_SPLASH_SCREEN_HPP
#define URDE_SPLASH_SCREEN_HPP

#include <Specter/View.hpp>
#include <Specter/ModalWindow.hpp>
#include <Specter/MultiLineTextView.hpp>
#include <Specter/FileBrowser.hpp>
#include <Specter/Menu.hpp>
#include "ViewManager.hpp"

namespace URDE
{
static LogVisor::LogModule Log("Specter::SplashScreen");

class SplashScreen : public Specter::ModalWindow
{
    ViewManager& m_vm;

    Zeus::CColor m_textColor;
    Zeus::CColor m_textColorClear;

    std::unique_ptr<Specter::TextView> m_title;
    std::string m_buildInfoStr;
    std::unique_ptr<Specter::MultiLineTextView> m_buildInfo;

    std::string m_newString;
    Specter::ViewChild<std::unique_ptr<Specter::Button>> m_newButt;
    std::string m_openString;
    Specter::ViewChild<std::unique_ptr<Specter::Button>> m_openButt;
    std::string m_extractString;
    Specter::ViewChild<std::unique_ptr<Specter::Button>> m_extractButt;

    Specter::ViewChild<std::unique_ptr<Specter::FileBrowser>> m_fileBrowser;

    struct NewProjBinding : Specter::IButtonBinding
    {
        SplashScreen& m_splash;
        HECL::SystemString m_deferPath;
        NewProjBinding(SplashScreen& splash) : m_splash(splash) {}
        const char* name(const Specter::Control* control) const {return m_splash.m_newString.c_str();}
        const char* help(const Specter::Control* control) const {return "Creates an empty project at selected path";}
        void activated(const Specter::Button* button, const boo::SWindowCoord& coord)
        {
            m_splash.m_fileBrowser.m_view.reset(
                        new Specter::FileBrowser(m_splash.rootView().viewRes(),
                                                 m_splash, m_splash.m_newString,
                                                 Specter::FileBrowser::Type::NewHECLProject,
                                                 [&](bool ok, const HECL::SystemString& path)
            {
                if (ok)
                    m_deferPath = path;
            }));
            m_splash.updateSize();
            m_splash.m_newButt.mouseLeave(coord);
        }
    } m_newProjBind;

    struct OpenProjBinding : Specter::IButtonBinding
    {
        SplashScreen& m_splash;
        HECL::SystemString m_deferPath;
        OpenProjBinding(SplashScreen& splash) : m_splash(splash), m_openRecentMenuRoot(*this) {}
        const char* name(const Specter::Control* control) const {return m_splash.m_openString.c_str();}
        const char* help(const Specter::Control* control) const {return "Opens an existing project at selected path";}
        void activated(const Specter::Button* button, const boo::SWindowCoord& coord)
        {
            m_splash.m_fileBrowser.m_view.reset(
                        new Specter::FileBrowser(m_splash.rootView().viewRes(),
                                                 m_splash, m_splash.m_openString,
                                                 Specter::FileBrowser::Type::OpenHECLProject,
                                                 [&](bool ok, const HECL::SystemString& path)
            {
                if (ok)
                    m_deferPath = path;
            }));
            m_splash.updateSize();
            m_splash.m_openButt.mouseLeave(coord);
        }

        struct OpenRecentMenuRoot : Specter::IMenuNode
        {
            OpenProjBinding& m_openProjBind;
            OpenRecentMenuRoot(OpenProjBinding& openProjBind) : m_openProjBind(openProjBind) {}

            std::string m_text;
            const std::string* text() const {return &m_text;}

            struct OpenRecentMenuItem : Specter::IMenuNode
            {
                OpenRecentMenuRoot& m_parent;

                HECL::SystemString m_path;
                std::string m_text;

                const std::string* text() const {return &m_text;}
                void activated() {m_parent.m_openProjBind.m_deferPath = m_path;}

                OpenRecentMenuItem(OpenRecentMenuRoot& parent, const HECL::SystemString& path)
                : m_parent(parent), m_path(path)
                {
                    std::vector<HECL::SystemString> pathComps = Specter::FileBrowser::PathComponents(path);
                    if (pathComps.size())
                        m_text = HECL::SystemUTF8View(pathComps.back()).str();
                }
            };
            std::vector<OpenRecentMenuItem> m_items;

            size_t subNodeCount() const {return m_items.size();}
            Specter::IMenuNode* subNode(size_t idx) {return &m_items[idx];}

            void buildNodes(const std::vector<HECL::SystemString>* recentProjects)
            {
                m_items.clear();
                if (recentProjects)
                {
                    m_items.reserve(recentProjects->size());
                    for (const HECL::SystemString& path : *recentProjects)
                        m_items.emplace_back(*this, path);
                }
            }
        } m_openRecentMenuRoot;

        MenuStyle menuStyle(const Specter::Button* button) const {return MenuStyle::Auxiliary;}
        std::unique_ptr<View> buildMenu(const Specter::Button* button)
        {
            m_openRecentMenuRoot.buildNodes(m_splash.m_vm.recentProjects());
            return std::unique_ptr<View>(new Specter::Menu(m_splash.rootView().viewRes(),
                                                           m_splash, &m_openRecentMenuRoot));
        }
    } m_openProjBind;

    struct ExtractProjBinding : Specter::IButtonBinding
    {
        SplashScreen& m_splash;
        HECL::SystemString m_deferPath;
        ExtractProjBinding(SplashScreen& splash) : m_splash(splash) {}
        const char* name(const Specter::Control* control) const {return m_splash.m_extractString.c_str();}
        const char* help(const Specter::Control* control) const {return "Extracts game image as project at selected path";}
        void activated(const Specter::Button* button, const boo::SWindowCoord& coord)
        {
            m_splash.m_fileBrowser.m_view.reset(
                        new Specter::FileBrowser(m_splash.rootView().viewRes(),
                                                 m_splash, m_splash.m_extractString,
                                                 Specter::FileBrowser::Type::OpenFile,
                                                 [&](bool ok, const HECL::SystemString& path)
            {
                if (ok)
                    m_deferPath = path;
            }));
            m_splash.updateSize();
            m_splash.m_extractButt.mouseLeave(coord);
        }
    } m_extractProjBind;

public:
    SplashScreen(ViewManager& vm, Specter::ViewResources& res);
    void think();
    void updateContentOpacity(float opacity);

    void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey);
    void mouseMove(const boo::SWindowCoord&);
    void mouseEnter(const boo::SWindowCoord&);
    void mouseLeave(const boo::SWindowCoord&);
    void scroll(const boo::SWindowCoord&, const boo::SScrollDelta&);
    void touchDown(const boo::STouchCoord&, uintptr_t);
    void touchUp(const boo::STouchCoord&, uintptr_t);
    void touchMove(const boo::STouchCoord&, uintptr_t);
    void charKeyDown(unsigned long, boo::EModifierKey, bool);
    void specialKeyDown(boo::ESpecialKey, boo::EModifierKey, bool);

    void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub);
    void draw(boo::IGraphicsCommandQueue* gfxQ);
};

}


#endif // URDE_SPLASH_SCREEN_HPP
