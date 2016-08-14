#ifndef __DNACOMMON_ITWEAKGUNRES_HPP__
#define __DNACOMMON_ITWEAKGUNRES_HPP__

#include "../DNACommon.hpp"

namespace DataSpec
{

struct ITweakGunRes : BigYAML
{
    virtual const std::string& GetGunMotion() const=0;
    virtual const std::string& GetGrappleArm() const=0;
    virtual const std::string& GetRightHand() const=0;

    virtual const std::string& GetPowerBeam() const=0;
    virtual const std::string& GetIceBeam() const=0;
    virtual const std::string& GetWaveBeam() const=0;
    virtual const std::string& GetPlasmaBeam() const=0;
    virtual const std::string& GetPhazonBeam() const=0;

    virtual const std::string& GetHoloTransition() const=0;

    virtual const std::string& GetBombSet() const=0;
    virtual const std::string& GetBombExplode() const=0;
    virtual const std::string& GetPowerBombExplode() const=0;

    virtual const std::string& GetPowerBeamWeapon() const=0;
    virtual const std::string& GetPowerBallWeapon() const=0;
    virtual const std::string& GetIceBeamWeapon() const=0;
    virtual const std::string& GetIceBallWeapon() const=0;
    virtual const std::string& GetWaveBeamWeapon() const=0;
    virtual const std::string& GetWaveBallWeapon() const=0;
    virtual const std::string& GetPlasmaBeamWeapon() const=0;
    virtual const std::string& GetPlasmaBallWeapon() const=0;
    virtual const std::string& GetPhazonBeamWeapon() const=0;
    virtual const std::string& GetPhazonBallWeapon() const=0;

    virtual const std::string& GetPowerMuzzleParticle() const=0;
    virtual const std::string& GetIceMuzzleParticle() const=0;
    virtual const std::string& GetWaveMuzzleParticle() const=0;
    virtual const std::string& GetPlasmaMuzzleParticle() const=0;
    virtual const std::string& GetPhazonMuzzleParticle() const=0;

    virtual const std::string& GetPowerChargeParticle() const=0;
    virtual const std::string& GetIceChargeParticle() const=0;
    virtual const std::string& GetWaveChargeParticle() const=0;
    virtual const std::string& GetPlasmaChargeParticle() const=0;
    virtual const std::string& GetPhazonChargeParticle() const=0;

    virtual const std::string& GetPowerAuxMuzzleParticle() const=0;
    virtual const std::string& GetIceAuxMuzzleParticle() const=0;
    virtual const std::string& GetWaveAuxMuzzleParticle() const=0;
    virtual const std::string& GetPlasmaAuxMuzzleParticle() const=0;
    virtual const std::string& GetPhazonAuxMuzzleParticle() const=0;

    virtual const std::string& GetGrappleSegmentParticle() const=0;
    virtual const std::string& GetGrappleClawParticle() const=0;
    virtual const std::string& GetGrappleHitParticle() const=0;
    virtual const std::string& GetGrappleMuzzleParticle() const=0;
    virtual const std::string& GetGrappleSwooshParticle() const=0;
};

}

#endif
