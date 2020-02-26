#pragma once

#include <array>

#include "ITweak.hpp"
#include "Runtime/IFactory.hpp"
#include "Runtime/CPlayerState.hpp"

namespace DataSpec {

struct ITweakGunRes : ITweak {
  using ResId = urde::CAssetId;
  using EBeamId = urde::CPlayerState::EBeamId;

  ResId x4_gunMotion;
  ResId x8_grappleArm;
  ResId xc_rightHand;

  ResId x10_powerBeam;
  ResId x14_iceBeam;
  ResId x18_waveBeam;
  ResId x1c_plasmaBeam;
  ResId x20_phazonBeam;

  ResId x24_holoTransition;

  ResId x28_bombSet;
  ResId x2c_bombExplode;
  ResId x30_powerBombExplode;

  /* Power, Ice, Wave, Plasma, Phazon / Beam, Ball */
  using WeaponPair = std::array<ResId, 2>;
  std::array<WeaponPair, 5> x34_weapons;
  std::array<ResId, 5> x84_muzzle;
  std::array<ResId, 5> x94_charge;
  std::array<ResId, 5> xa4_auxMuzzle;

  ResId xb4_grappleSegment;
  ResId xb8_grappleClaw;
  ResId xbc_grappleHit;
  ResId xc0_grappleMuzzle;
  ResId xc4_grappleSwoosh;

  ResId GetBeamModel(EBeamId beam) const {
    auto b = int(beam);
    if (b < 0 || b > 4)
      b = 0;
    switch (EBeamId(b)) {
    default:
    case EBeamId::Power:
      return x10_powerBeam;
    case EBeamId::Ice:
      return x14_iceBeam;
    case EBeamId::Wave:
      return x18_waveBeam;
    case EBeamId::Plasma:
      return x1c_plasmaBeam;
    case EBeamId::Phazon:
      return x20_phazonBeam;
    }
  }

  const WeaponPair& GetWeaponPair(EBeamId beam) const {
    const auto b = int(beam);
    if (b < 0 || b > 4) {
      return x34_weapons[0];
    }
    return x34_weapons[b];
  }

  void ResolveResources(const urde::IFactory& factory) {
    x4_gunMotion = factory.GetResourceIdByName(GetGunMotion())->id;
    x8_grappleArm = factory.GetResourceIdByName(GetGrappleArm())->id;
    xc_rightHand = factory.GetResourceIdByName(GetRightHand())->id;

    x10_powerBeam = factory.GetResourceIdByName(GetPowerBeam())->id;
    x14_iceBeam = factory.GetResourceIdByName(GetIceBeam())->id;
    x18_waveBeam = factory.GetResourceIdByName(GetWaveBeam())->id;
    x1c_plasmaBeam = factory.GetResourceIdByName(GetPlasmaBeam())->id;
    x20_phazonBeam = factory.GetResourceIdByName(GetPhazonBeam())->id;

    x24_holoTransition = factory.GetResourceIdByName(GetHoloTransition())->id;

    x28_bombSet = factory.GetResourceIdByName(GetBombSet())->id;
    x2c_bombExplode = factory.GetResourceIdByName(GetBombExplode())->id;
    x30_powerBombExplode = factory.GetResourceIdByName(GetPowerBombExplode())->id;

    for (size_t i = 0; i < x34_weapons.size(); ++i) {
      for (size_t j = 0; j < x34_weapons[i].size(); ++j) {
        x34_weapons[i][j] = factory.GetResourceIdByName(GetWeapon(i, j != 0))->id;
      }
    }

    for (size_t i = 0; i < x84_muzzle.size(); ++i) {
      x84_muzzle[i] = factory.GetResourceIdByName(GetMuzzleParticle(i))->id;
    }

    for (size_t i = 0; i < x94_charge.size(); ++i) {
      x94_charge[i] = factory.GetResourceIdByName(GetChargeParticle(i))->id;
    }

    for (size_t i = 0; i < xa4_auxMuzzle.size(); ++i) {
      xa4_auxMuzzle[i] = factory.GetResourceIdByName(GetAuxMuzzleParticle(i))->id;
    }

    xb4_grappleSegment = factory.GetResourceIdByName(GetGrappleSegmentParticle())->id;
    xb8_grappleClaw = factory.GetResourceIdByName(GetGrappleClawParticle())->id;
    xbc_grappleHit = factory.GetResourceIdByName(GetGrappleHitParticle())->id;
    xc0_grappleMuzzle = factory.GetResourceIdByName(GetGrappleMuzzleParticle())->id;
    xc4_grappleSwoosh = factory.GetResourceIdByName(GetGrappleSwooshParticle())->id;
  }

protected:
  virtual const std::string& GetGunMotion() const = 0;
  virtual const std::string& GetGrappleArm() const = 0;
  virtual const std::string& GetRightHand() const = 0;

  virtual const std::string& GetPowerBeam() const = 0;
  virtual const std::string& GetIceBeam() const = 0;
  virtual const std::string& GetWaveBeam() const = 0;
  virtual const std::string& GetPlasmaBeam() const = 0;
  virtual const std::string& GetPhazonBeam() const = 0;

  virtual const std::string& GetHoloTransition() const = 0;

  virtual const std::string& GetBombSet() const = 0;
  virtual const std::string& GetBombExplode() const = 0;
  virtual const std::string& GetPowerBombExplode() const = 0;

  virtual const std::string& GetWeapon(size_t idx, bool ball) const = 0;
  virtual const std::string& GetMuzzleParticle(size_t idx) const = 0;
  virtual const std::string& GetChargeParticle(size_t idx) const = 0;
  virtual const std::string& GetAuxMuzzleParticle(size_t idx) const = 0;

  virtual const std::string& GetGrappleSegmentParticle() const = 0;
  virtual const std::string& GetGrappleClawParticle() const = 0;
  virtual const std::string& GetGrappleHitParticle() const = 0;
  virtual const std::string& GetGrappleMuzzleParticle() const = 0;
  virtual const std::string& GetGrappleSwooshParticle() const = 0;
};

} // namespace DataSpec
