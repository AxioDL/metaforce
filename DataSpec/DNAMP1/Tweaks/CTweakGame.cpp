#include "DataSpec/DNAMP1/Tweaks/CTweakGame.hpp"
#include "Runtime/CStringExtras.hpp"

#include <hecl/CVar.hpp>
#include <hecl/CVarManager.hpp>

namespace DataSpec::DNAMP1 {
namespace {
constexpr std::string_view skFov = "tweaks.game.FieldOfView"sv;
constexpr std::string_view skHardModeDamageMultName = "tweaks.game.HardModeDamageMult"sv;
constexpr std::string_view skHardModeWeaponMultName = "tweaks.game.HardModeWeaponMult"sv;
}
void CTweakGame::_tweakGameListener(hecl::CVar* cv) {
  if (urde::CStringExtras::CompareCaseInsensitive(cv->name(), skFov)) {
    x24_fov = cv->toReal();
  } else if (urde::CStringExtras::CompareCaseInsensitive(cv->name(), skHardModeDamageMultName)) {
    x60_hardmodeDamageMult = cv->toReal();
  } else if (urde::CStringExtras::CompareCaseInsensitive(cv->name(), skHardModeWeaponMultName)) {
    x64_hardmodeWeaponMult = cv->toReal();
  }
  cv->clearModified();
}

void CTweakGame::initCVars(hecl::CVarManager* mgr) {
  auto assignRealValue = [this, mgr](std::string_view name, std::string_view desc, float& v, hecl::CVar::EFlags flags) {
    hecl::CVar* cv = mgr->findOrMakeCVar(name, desc, v, flags);
    // Check if the CVar was deserialized, this avoid an unnecessary conversion
    if (cv->wasDeserialized())
      v = cv->toReal();
    cv->addListener([this](hecl::CVar* cv) { _tweakGameListener(cv); });
  };

  assignRealValue(skFov, "", x24_fov,
              hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::ModifyRestart);
  assignRealValue(skHardModeDamageMultName, "", x60_hardmodeDamageMult,
              hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::Cheat);
  assignRealValue(skHardModeWeaponMultName, "", x64_hardmodeWeaponMult,
              hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::Cheat);
}
} // namespace DataSpec::DNAMP1
