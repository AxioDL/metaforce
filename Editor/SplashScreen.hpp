#pragma once

#include "specter/View.hpp"
#include "specter/ModalWindow.hpp"
#include "specter/MultiLineTextView.hpp"
#include "specter/FileBrowser.hpp"
#include "specter/Menu.hpp"
#include "specter/Icon.hpp"

#include "ViewManager.hpp"

namespace urde {
static logvisor::Module Log("specter::SplashScreen");

class SplashScreen : public specter::ModalWindow {
  ViewManager& m_vm;

  zeus::CColor m_textColor;
  zeus::CColor m_textColorClear;

  std::unique_ptr<specter::TextView> m_title;
  std::string m_buildInfoStr;
  std::unique_ptr<specter::MultiLineTextView> m_buildInfo;
  std::unique_ptr<specter::IconView> m_badgeIcon;
  std::unique_ptr<specter::TextView> m_badgeText;

  std::string m_newString;
  specter::ViewChild<std::unique_ptr<specter::Button>> m_newButt;
  std::string m_openString;
  specter::ViewChild<std::unique_ptr<specter::Button>> m_openButt;
  std::string m_extractString;
  specter::ViewChild<std::unique_ptr<specter::Button>> m_extractButt;

  specter::ViewChild<std::unique_ptr<specter::FileBrowser>> m_fileBrowser;

  struct NewProjBinding : specter::IButtonBinding {
    SplashScreen& m_splash;
    hecl::SystemString m_deferPath;
    NewProjBinding(SplashScreen& splash) : m_splash(splash) {}
    std::string_view name(const specter::Control* control) const override { return m_splash.m_newString.c_str(); }
    std::string_view help(const specter::Control* control) const override {
      return "Creates an empty project at selected path";
    }
    void activated(const specter::Button* button, const boo::SWindowCoord& coord) override {
      m_splash.m_fileBrowser.m_view.reset(new specter::FileBrowser(
          m_splash.rootView().viewRes(), m_splash, m_splash.m_newString, specter::FileBrowser::Type::NewHECLProject,
          [&](bool ok, hecl::SystemStringView path) {
            if (ok)
              m_deferPath = path;
          }));
      m_splash.updateSize();
      m_splash.m_newButt.mouseLeave(coord);
    }
  } m_newProjBind;

  struct OpenProjBinding : specter::IButtonBinding {
    SplashScreen& m_splash;
    hecl::SystemString m_deferPath;
    OpenProjBinding(SplashScreen& splash) : m_splash(splash), m_openRecentMenuRoot(*this) {}
    std::string_view name(const specter::Control* control) const override { return m_splash.m_openString.c_str(); }
    std::string_view help(const specter::Control* control) const override {
      return "Opens an existing project at selected path";
    }
    void activated(const specter::Button* button, const boo::SWindowCoord& coord) override {
      m_splash.m_fileBrowser.m_view.reset(new specter::FileBrowser(
          m_splash.rootView().viewRes(), m_splash, m_splash.m_openString, specter::FileBrowser::Type::OpenHECLProject,
          [&](bool ok, hecl::SystemStringView path) {
            if (ok)
              m_deferPath = path;
          }));
      m_splash.updateSize();
      m_splash.m_openButt.mouseLeave(coord);
    }

    struct OpenRecentMenuRoot : specter::IMenuNode {
      OpenProjBinding& m_openProjBind;
      OpenRecentMenuRoot(OpenProjBinding& openProjBind) : m_openProjBind(openProjBind) {}

      std::string m_text;
      const std::string* text() const override { return &m_text; }

      struct OpenRecentMenuItem final : specter::IMenuNode {
        OpenRecentMenuRoot& m_parent;

        hecl::SystemString m_path;
        std::string m_text;

        const std::string* text() const override { return &m_text; }
        void activated(const boo::SWindowCoord& coord) override {
          m_parent.m_openProjBind.m_deferPath = m_path;
          m_parent.m_openProjBind.m_splash.m_openButt.m_view->closeMenu(coord);
        }

        OpenRecentMenuItem(OpenRecentMenuRoot& parent, const hecl::SystemString& path)
        : m_parent(parent), m_path(path) {
          std::vector<hecl::SystemString> pathComps = specter::FileBrowser::PathComponents(path);
          if (pathComps.size())
            m_text = hecl::SystemUTF8Conv(pathComps.back()).str();
        }
      };
      std::vector<OpenRecentMenuItem> m_items;

      size_t subNodeCount() const override { return m_items.size(); }
      specter::IMenuNode* subNode(size_t idx) override { return &m_items[idx]; }

      void buildNodes(const std::vector<hecl::SystemString>* recentProjects) {
        m_items.clear();
        if (recentProjects) {
          m_items.reserve(recentProjects->size());
          for (const hecl::SystemString& path : *recentProjects)
            m_items.emplace_back(*this, path);
        }
      }
    } m_openRecentMenuRoot;

    MenuStyle menuStyle(const specter::Button* button) const override { return MenuStyle::Auxiliary; }
    std::unique_ptr<View> buildMenu(const specter::Button* button) override {
      m_openRecentMenuRoot.buildNodes(m_splash.m_vm.recentProjects());
      return std::unique_ptr<View>(new specter::Menu(m_splash.rootView().viewRes(), m_splash, &m_openRecentMenuRoot));
    }
  } m_openProjBind;

  struct ExtractProjBinding : specter::IButtonBinding {
    SplashScreen& m_splash;
    hecl::SystemString m_deferPath;
    ExtractProjBinding(SplashScreen& splash) : m_splash(splash) {}
    std::string_view name(const specter::Control* control) const override { return m_splash.m_extractString.c_str(); }
    std::string_view help(const specter::Control* control) const override {
      return "Extracts game image as project at selected path";
    }
    void activated(const specter::Button* button, const boo::SWindowCoord& coord) override {
      m_splash.m_fileBrowser.m_view.reset(
          new specter::FileBrowser(m_splash.rootView().viewRes(), m_splash, m_splash.m_extractString,
                                   specter::FileBrowser::Type::OpenFile, [&](bool ok, hecl::SystemStringView path) {
                                     if (ok)
                                       m_deferPath = path;
                                   }));
      m_splash.updateSize();
      m_splash.m_extractButt.mouseLeave(coord);
    }
  } m_extractProjBind;

public:
  SplashScreen(ViewManager& vm, specter::ViewResources& res);
  void think() override;
  void updateContentOpacity(float opacity) override;

  void mouseDown(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseUp(const boo::SWindowCoord&, boo::EMouseButton, boo::EModifierKey) override;
  void mouseMove(const boo::SWindowCoord&) override;
  void mouseEnter(const boo::SWindowCoord&) override;
  void mouseLeave(const boo::SWindowCoord&) override;
  void scroll(const boo::SWindowCoord&, const boo::SScrollDelta&) override;
  void touchDown(const boo::STouchCoord&, uintptr_t) override;
  void touchUp(const boo::STouchCoord&, uintptr_t) override;
  void touchMove(const boo::STouchCoord&, uintptr_t) override;
  void charKeyDown(unsigned long, boo::EModifierKey, bool) override;
  void specialKeyDown(boo::ESpecialKey, boo::EModifierKey, bool) override;

  void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) override;
  void draw(boo::IGraphicsCommandQueue* gfxQ) override;
};

} // namespace urde
