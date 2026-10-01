#include "Metaforce/UI/UI.hpp"

#include "Metaforce/UI/MenuBar.hpp"
#include "Metaforce/UI/RuntimeConfig.hpp"

#include "Kyoto/Audio/CSfxManager.hpp"
#include "MetroidPrime/SFX/UI.h"

#include <borealis/ui/ui.hpp>

namespace metaforce::ui {
namespace {
using borealis::ui::NavSound;

constexpr ushort kNoSfx = 0xFFFF;

bool sInitialized = false;

ushort nav_sound_sfx(NavSound sound) {
  switch (sound) {
  // TODO: find values that don't suck
  // case NavSound::Click:
  // case NavSound::BindingChanged:
  // case NavSound::WindowOpen:
  //   return SFXui_x_invchoos_00;
  // case NavSound::Play:
  // case NavSound::ItemEnable:
  //   return SFXui_x_quitaff_00;
  // case NavSound::ItemDisable:
  //   return SFXui_x_quitneg_00;
  // case NavSound::MenuOpen:
  //   return SFXui_x_invon_00;
  // case NavSound::MenuClose:
  //   return SFXui_x_invoff_00;
  // case NavSound::WindowClose:
  //   return SFXui_x_invback_00;
  // case NavSound::TabChanged:
  //   return SFXui_x_invflip_00;
  case NavSound::ItemFocus:
    return SFXui_x_invsel_00;
  // case NavSound::ItemChange:
  //   return SFXui_x_quitsel_00;
  // case NavSound::Warning:
  //   return SFXui_x_warning_00;
  default:
    return kNoSfx;
  }
}

void play_nav_sound(NavSound sound) {
  if (!GetRuntimeConfig().ui.sounds) {
    return;
  }
  if (const ushort sfx = nav_sound_sfx(sound); sfx != kNoSfx) {
    CSfxManager::SfxStart(sfx, 0x7f, 0x40, false, CSfxManager::kMedPriority, false,
                          CSfxManager::kAllAreas);
  }
}

void load_fonts() {
  borealis::ui::load_font("TitilliumWeb-Regular.ttf", true);
  borealis::ui::load_font("TitilliumWeb-SemiBold.ttf");
  borealis::ui::load_font("TitilliumWeb-Bold.ttf");
  borealis::ui::load_font("SairaSemiCondensed-Regular.ttf");
  borealis::ui::load_font("SairaSemiCondensed-SemiBold.ttf");
  borealis::ui::load_font("SairaSemiCondensed-Bold.ttf");
  borealis::ui::load_font("MaterialSymbolsRounded-Regular.ttf");
  borealis::ui::load_font("NotoMono-Regular.ttf");
}

} // namespace

RuntimeConfig& GetRuntimeConfig() {
  static RuntimeConfig sConfig;
  return sConfig;
}

bool Initialize() {
  if (sInitialized) {
    return true;
  }
  if (!borealis::ui::initialize()) {
    return false;
  }
  load_fonts();
  borealis::ui::set_nav_sound_handler(&play_nav_sound);
  borealis::ui::set_user_scale(GetRuntimeConfig().ui.scale);
  borealis::ui::push_document(std::make_unique< MenuBar >(), false);
  sInitialized = true;
  return true;
}

void Shutdown() {
  if (!sInitialized) {
    return;
  }
  borealis::ui::shutdown();
  sInitialized = false;
}

void HandleEvent(const SDL_Event& event) {
  if (sInitialized) {
    borealis::ui::handle_event(event);
  }
}

void Update() {
  if (sInitialized) {
    borealis::ui::update();
  }
}

} // namespace metaforce::ui
