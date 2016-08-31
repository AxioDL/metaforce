#ifndef _DNAMP1_CTWEAKGUNRES_HPP_
#define _DNAMP1_CTWEAKGUNRES_HPP_

#include "../../DNACommon/Tweaks/ITweakGunRes.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct CTweakGunRes : ITweakGunRes
{
    DECL_YAML

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

    const std::string& GetPowerBeamWeapon() const { return m_powerBeamWp; }
    const std::string& GetPowerBallWeapon() const { return m_powerBallWp; }
    const std::string& GetIceBeamWeapon() const { return m_iceBeamWp; }
    const std::string& GetIceBallWeapon() const { return m_iceBallWp; }
    const std::string& GetWaveBeamWeapon() const { return m_waveBeamWp; }
    const std::string& GetWaveBallWeapon() const { return m_waveBallWp; }
    const std::string& GetPlasmaBeamWeapon() const { return m_plasmaBeamWp; }
    const std::string& GetPlasmaBallWeapon() const { return m_plasmaBallWp; }
    const std::string& GetPhazonBeamWeapon() const { return m_phazonBeamWp; }
    const std::string& GetPhazonBallWeapon() const { return m_phazonBallWp; }

    const std::string& GetPowerMuzzleParticle() const { return m_powerMuzzle; }
    const std::string& GetIceMuzzleParticle() const { return m_iceMuzzle; }
    const std::string& GetWaveMuzzleParticle() const { return m_waveMuzzle; }
    const std::string& GetPlasmaMuzzleParticle() const { return m_plasmaMuzzle; }
    const std::string& GetPhazonMuzzleParticle() const { return m_phazonMuzzle; }

    const std::string& GetPowerChargeParticle() const { return m_powerCharge; }
    const std::string& GetIceChargeParticle() const { return m_iceCharge; }
    const std::string& GetWaveChargeParticle() const { return m_waveCharge; }
    const std::string& GetPlasmaChargeParticle() const { return m_plasmaCharge; }
    const std::string& GetPhazonChargeParticle() const { return m_phazonCharge; }

    const std::string& GetPowerAuxMuzzleParticle() const { return m_powerAuxMuzzle; }
    const std::string& GetIceAuxMuzzleParticle() const { return m_iceAuxMuzzle; }
    const std::string& GetWaveAuxMuzzleParticle() const { return m_waveAuxMuzzle; }
    const std::string& GetPlasmaAuxMuzzleParticle() const { return m_plasmaAuxMuzzle; }
    const std::string& GetPhazonAuxMuzzleParticle() const { return m_phazonAuxMuzzle; }

    const std::string& GetGrappleSegmentParticle() const { return m_grappleSegment; }
    const std::string& GetGrappleClawParticle() const { return m_grappleClaw; }
    const std::string& GetGrappleHitParticle() const { return m_grappleHit; }
    const std::string& GetGrappleMuzzleParticle() const { return m_grappleMuzzle; }
    const std::string& GetGrappleSwooshParticle() const { return m_grappleSwoosh; }

    CTweakGunRes() = default;
    CTweakGunRes(athena::io::IStreamReader& in) { read(in); }
};

}
}

#endif // _DNAMP1_CTWEAKGUNRES_HPP_
