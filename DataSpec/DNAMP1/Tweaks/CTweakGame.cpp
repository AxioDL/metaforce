#include "DataSpec/DNAMP1/Tweaks/CTweakGame.hpp"
#include "Runtime/Camera/CCameraManager.hpp"

#include <hecl/CVar.hpp>
#include <hecl/CVarManager.hpp>

namespace DataSpec::DNAMP1 {
hecl::CVar* tw_fov = nullptr;
hecl::CVar* tw_hardmodeDMult = nullptr;
hecl::CVar* tw_hardmodeWMult = nullptr;
namespace {
constexpr std::string_view skFov = "tweaks.game.FieldOfView"sv;
constexpr std::string_view skHardModeDamageMultName = "tweaks.game.HardModeDamageMult"sv;
constexpr std::string_view skHardModeWeaponMultName = "tweaks.game.HardModeWeaponMult"sv;
} // anonymous namespace

void CTweakGame::_tweakGameListener(hecl::CVar* cv) {
  if (cv == tw_fov) {
    x24_fov = cv->toReal();
  } else if (cv == tw_hardmodeDMult) {
    x60_hardmodeDamageMult = cv->toReal();
  } else if (cv == tw_hardmodeWMult) {
    x64_hardmodeWeaponMult = cv->toReal();
  }
}

void CTweakGame::initCVars(hecl::CVarManager* mgr) {
  auto assignRealValue = [this, mgr](std::string_view name, std::string_view desc, float& v, hecl::CVar::EFlags flags) {
    hecl::CVar* cv = mgr->findOrMakeCVar(name, desc, v, flags);
    // Check if the CVar was deserialized, this avoid an unnecessary conversion
    if (cv->wasDeserialized())
      v = cv->toReal();
    cv->addListener([this](hecl::CVar* cv) { _tweakGameListener(cv); });
    return cv;
  };

  tw_fov = assignRealValue(skFov, "", x24_fov,
                           hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive);
  tw_hardmodeDMult =
      assignRealValue(skHardModeDamageMultName, "", x60_hardmodeDamageMult,
                      hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::Cheat);
  tw_hardmodeWMult =
      assignRealValue(skHardModeWeaponMultName, "", x64_hardmodeWeaponMult,
                      hecl::CVar::EFlags::Game | hecl::CVar::EFlags::Archive | hecl::CVar::EFlags::Cheat);
}
} // namespace DataSpec::DNAMP1
