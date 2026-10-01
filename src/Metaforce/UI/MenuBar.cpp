#include "Metaforce/UI/MenuBar.hpp"

#include "Metaforce/Runtime.hpp"
#include "Metaforce/UI/SettingsWindow.hpp"

#include <borealis/ui/modal.hpp>

namespace metaforce::ui {
using namespace borealis::ui;

MenuBar::MenuBar() { build_tabs(); }

void MenuBar::build_tabs() {
  mTabBar->add_tab("Settings", [this] { push(std::make_unique< SettingsWindow >()); });
  mTabBar->add_tab("Quit", [this] {
    mTabBar->set_active_tab(-1);
    const auto dismiss = [](Modal& modal) { modal.pop(); };
    push(std::make_unique< Modal >(Modal::Props{
        .title = "Quit Metaforce",
        .bodyText = "Unsaved progress will be lost.",
        .actions =
            {
                ModalAction{
                    .label = "Cancel",
                    .onPressed =
                        [dismiss](Modal& modal) {
                          play_nav_sound(NavSound::WindowClose);
                          dismiss(modal);
                        },
                },
                ModalAction{
                    .label = "Quit",
                    .onPressed =
                        [dismiss](Modal& modal) {
                          play_nav_sound(NavSound::Click);
                          dismiss(modal);
                          RequestQuit();
                        },
                },
            },
        .onDismiss = dismiss,
        .icon = "question-mark",
    }));
  });
}

} // namespace metaforce::ui
