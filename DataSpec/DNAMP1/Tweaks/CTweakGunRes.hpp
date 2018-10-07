#pragma once

#include "../../DNACommon/Tweaks/ITweakGunRes.hpp"

namespace DataSpec::DNAMP1
{

struct CTweakGunRes final : ITweakGunRes
{
    AT_DECL_DNA_YAML

    String<-1> m_gunMotion;
    String<-1> m_grappleArm;
    String<-1> m_rightHand;

    String<-1> m_powerBeam;
    String<-1> m_iceBeam;
    String<-1> m_waveBeam;
    String<-1> m_plasmaBeam;
    String<-1> m_phazonBeam;

    String<-1> m_holoTransition;

    String<-1> m_bombSet;
    String<-1> m_bombExplode;
    String<-1> m_powerBombExplode;

    String<-1> m_powerBeamWp;
    String<-1> m_powerBallWp;
    String<-1> m_iceBeamWp;
    String<-1> m_iceBallWp;
    String<-1> m_waveBeamWp;
    String<-1> m_waveBallWp;
    String<-1> m_plasmaBeamWp;
    String<-1> m_plasmaBallWp;
    String<-1> m_phazonBeamWp;
    String<-1> m_phazonBallWp;

    String<-1> m_powerMuzzle;
    String<-1> m_iceMuzzle;
    String<-1> m_waveMuzzle;
    String<-1> m_plasmaMuzzle;
    String<-1> m_phazonMuzzle;

    String<-1> m_powerCharge;
    String<-1> m_iceCharge;
    String<-1> m_waveCharge;
    String<-1> m_plasmaCharge;
    String<-1> m_phazonCharge;

    String<-1> m_powerAuxMuzzle;
    String<-1> m_iceAuxMuzzle;
    String<-1> m_waveAuxMuzzle;
    String<-1> m_plasmaAuxMuzzle;
    String<-1> m_phazonAuxMuzzle;

    String<-1> m_grappleSegment;
    String<-1> m_grappleClaw;
    String<-1> m_grappleHit;
    String<-1> m_grappleMuzzle;
    String<-1> m_grappleSwoosh;

    const std::string& GetGunMotion() const { return m_gunMotion; }
    const std::string& GetGrappleArm() const { return m_grappleArm; }
    const std::string& GetRightHand() const { return m_rightHand; }

    const std::string& GetPowerBeam() const { return m_powerBeam; }
    const std::string& GetIceBeam() const { return m_iceBeam; }
    const std::string& GetWaveBeam() const { return m_waveBeam; }
    const std::string& GetPlasmaBeam() const { return m_plasmaBeam; }
    const std::string& GetPhazonBeam() const { return m_phazonBeam; }

    const std::string& GetHoloTransition() const { return m_holoTransition; }

    const std::string& GetBombSet() const { return m_bombSet; }
    const std::string& GetBombExplode() const { return m_bombExplode; }
    const std::string& GetPowerBombExplode() const { return m_powerBombExplode; }

    const std::string& GetWeapon(size_t idx, bool ball) const { return (&m_powerBeamWp)[idx * 2 + ball]; }
    const std::string& GetMuzzleParticle(size_t idx) const { return (&m_powerMuzzle)[idx]; }
    const std::string& GetChargeParticle(size_t idx) const { return (&m_powerCharge)[idx]; }
    const std::string& GetAuxMuzzleParticle(size_t idx) const { return (&m_powerAuxMuzzle)[idx]; }

    const std::string& GetGrappleSegmentParticle() const { return m_grappleSegment; }
    const std::string& GetGrappleClawParticle() const { return m_grappleClaw; }
    const std::string& GetGrappleHitParticle() const { return m_grappleHit; }
    const std::string& GetGrappleMuzzleParticle() const { return m_grappleMuzzle; }
    const std::string& GetGrappleSwooshParticle() const { return m_grappleSwoosh; }

    CTweakGunRes() = default;
    CTweakGunRes(athena::io::IStreamReader& in) { read(in); }
};

}

