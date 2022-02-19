#pragma once

#include "Runtime/Tweaks/ITweakGunRes.hpp"

namespace metaforce::MP1 {

struct CTweakGunRes final : Tweaks::ITweakGunRes {
  std::string m_gunMotion;
  std::string m_grappleArm;
  std::string m_rightHand;

  std::string m_powerBeam;
  std::string m_iceBeam;
  std::string m_waveBeam;
  std::string m_plasmaBeam;
  std::string m_phazonBeam;

  std::string m_holoTransition;

  std::string m_bombSet;
  std::string m_bombExplode;
  std::string m_powerBombExplode;

  std::string m_powerBeamWp;
  std::string m_powerBallWp;
  std::string m_iceBeamWp;
  std::string m_iceBallWp;
  std::string m_waveBeamWp;
  std::string m_waveBallWp;
  std::string m_plasmaBeamWp;
  std::string m_plasmaBallWp;
  std::string m_phazonBeamWp;
  std::string m_phazonBallWp;

  std::string m_powerMuzzle;
  std::string m_iceMuzzle;
  std::string m_waveMuzzle;
  std::string m_plasmaMuzzle;
  std::string m_phazonMuzzle;

  std::string m_powerCharge;
  std::string m_iceCharge;
  std::string m_waveCharge;
  std::string m_plasmaCharge;
  std::string m_phazonCharge;

  std::string m_powerAuxMuzzle;
  std::string m_iceAuxMuzzle;
  std::string m_waveAuxMuzzle;
  std::string m_plasmaAuxMuzzle;
  std::string m_phazonAuxMuzzle;

  std::string m_grappleSegment;
  std::string m_grappleClaw;
  std::string m_grappleHit;
  std::string m_grappleMuzzle;
  std::string m_grappleSwoosh;

  const std::string& GetGunMotion() const override { return m_gunMotion; }
  const std::string& GetGrappleArm() const override { return m_grappleArm; }
  const std::string& GetRightHand() const override { return m_rightHand; }

  const std::string& GetPowerBeam() const override { return m_powerBeam; }
  const std::string& GetIceBeam() const override { return m_iceBeam; }
  const std::string& GetWaveBeam() const override { return m_waveBeam; }
  const std::string& GetPlasmaBeam() const override { return m_plasmaBeam; }
  const std::string& GetPhazonBeam() const override { return m_phazonBeam; }

  const std::string& GetHoloTransition() const override { return m_holoTransition; }

  const std::string& GetBombSet() const override { return m_bombSet; }
  const std::string& GetBombExplode() const override { return m_bombExplode; }
  const std::string& GetPowerBombExplode() const override { return m_powerBombExplode; }

  const std::string& GetWeapon(size_t idx, bool ball) const override { return (&m_powerBeamWp)[idx * 2 + ball]; }
  const std::string& GetMuzzleParticle(size_t idx) const override { return (&m_powerMuzzle)[idx]; }
  const std::string& GetChargeParticle(size_t idx) const override { return (&m_powerCharge)[idx]; }
  const std::string& GetAuxMuzzleParticle(size_t idx) const override { return (&m_powerAuxMuzzle)[idx]; }

  const std::string& GetGrappleSegmentParticle() const override { return m_grappleSegment; }
  const std::string& GetGrappleClawParticle() const override { return m_grappleClaw; }
  const std::string& GetGrappleHitParticle() const override { return m_grappleHit; }
  const std::string& GetGrappleMuzzleParticle() const override { return m_grappleMuzzle; }
  const std::string& GetGrappleSwooshParticle() const override { return m_grappleSwoosh; }

  CTweakGunRes() = default;
  CTweakGunRes(CInputStream& in);
};

} // namespace metaforce::MP1
