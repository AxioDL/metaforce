#pragma once

#include "specter/View.hpp"
#include "specter/ModalWindow.hpp"
#include "specter/MultiLineTextView.hpp"
#include "specter/FileBrowser.hpp"
#include "specter/Menu.hpp"
#include "specter/Icon.hpp"

#include "ViewManager.hpp"

namespace metaforce {
static logvisor::Module Log("specter::SplashScreen");

class SplashScreen : public specter::ModalWindow {
  ViewManager& m_vm;

  zeus::CColor m_textColor;

  std::unique_ptr<specter::TextView> m_title;
  std::string m_buildInfoStr;
  std::unique_ptr<specter::MultiLineTextView> m_buildInfo;
  std::unique_ptr<specter::TextView> m_badgeText;

  std::unique_ptr<specter::TextView> m_infoStr;

public:
  SplashScreen(ViewManager& vm, specter::ViewResources& res);
  void think() override;
  void updateContentOpacity(float opacity) override;

  void resized(const boo::SWindowRect& root, const boo::SWindowRect& sub) override;
  void draw(boo::IGraphicsCommandQueue* gfxQ) override;
};

} // namespace metaforce
