#pragma once

#include "ITweak.hpp"
#include "zeus/CAABox.hpp"

namespace DataSpec {
/* Same as CDamageInfo */
struct SShotParam : BigDNA {
  AT_DECL_DNA_YAML
  Value<atInt32> weaponType = -1;
  bool charged : 1;
  bool combo : 1;
  bool instaKill : 1;
  Value<float> damage = 0.f;
  Value<float> radiusDamage = 0.f;
  Value<float> radius = 0.f;
  Value<float> knockback = 0.f;
  bool noImmunity : 1;
  SShotParam() {
    charged = false;
    combo = false;
    instaKill = false;
    noImmunity = false;
  }
};

struct SComboShotParam : SShotParam {
  AT_DECL_DNA_YAML
  SComboShotParam() { combo = true; }
};

struct SChargedShotParam : SShotParam {
  AT_DECL_DNA_YAML
  SChargedShotParam() { charged = true; }
};

struct SWeaponInfo : BigDNA {
  AT_DECL_DNA_YAML
  Value<float> x0_coolDown = 0.1f;
  SShotParam x4_normal;
  SChargedShotParam x20_charged;
};

struct ITweakPlayerGun : ITweak {
  AT_DECL_DNA_YAML
};
} // namespace DataSpec
