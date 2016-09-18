#ifndef __DNACOMMON_ITWEAKGUNRES_HPP__
#define __DNACOMMON_ITWEAKGUNRES_HPP__

#include "../DNACommon.hpp"
#include "Runtime/IFactory.hpp"

namespace DataSpec
{

struct ITweakGunRes : BigYAML
{
    using ResId = int64_t;
    enum class EBeamId
    {
        Power,
        Ice,
        Plasma,
        Wave,
        Phazon
    };

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
    ResId x34_weapons[5][2];
    ResId x84_muzzle[5];
    ResId x94_charge[5];
    ResId xa4_auxMuzzle[5];

    ResId xb4_grappleSegment;
    ResId xb8_grappleClaw;
    ResId xbc_grappleHit;
    ResId xc0_grappleMuzzle;
    ResId xc4_grappleSwoosh;

    ResId GetBeamModel(EBeamId beam) const
    {
        int b = int(beam);
        if (b < 0 || b > 4)
            b = 0;
        switch (EBeamId(b))
        {
        case EBeamId::Power:
        default:
            return x10_powerBeam;
        case EBeamId::Ice:
            return x14_iceBeam;
        case EBeamId::Plasma:
            return x1c_plasmaBeam;
        case EBeamId::Wave:
            return x18_waveBeam;
        case EBeamId::Phazon:
            return x20_phazonBeam;
        }
    }

    void ResolveResources(const urde::IFactory& factory)
    {
        x4_gunMotion = factory.GetResourceIdByName(GetGunMotion().c_str())->id;
        x8_grappleArm = factory.GetResourceIdByName(GetGrappleArm().c_str())->id;
        xc_rightHand = factory.GetResourceIdByName(GetRightHand().c_str())->id;

        x10_powerBeam = factory.GetResourceIdByName(GetPowerBeam().c_str())->id;
        x14_iceBeam = factory.GetResourceIdByName(GetIceBeam().c_str())->id;
        x18_waveBeam = factory.GetResourceIdByName(GetWaveBeam().c_str())->id;
        x1c_plasmaBeam = factory.GetResourceIdByName(GetPlasmaBeam().c_str())->id;
        x20_phazonBeam = factory.GetResourceIdByName(GetPhazonBeam().c_str())->id;

        x24_holoTransition = factory.GetResourceIdByName(GetHoloTransition().c_str())->id;

        x28_bombSet = factory.GetResourceIdByName(GetBombSet().c_str())->id;
        x2c_bombExplode = factory.GetResourceIdByName(GetBombExplode().c_str())->id;
        x30_powerBombExplode = factory.GetResourceIdByName(GetPowerBombExplode().c_str())->id;

        for (int i=0 ; i<5 ; ++i)
            for (int j=0 ; j<2 ; ++j)
                x34_weapons[i][j] = factory.GetResourceIdByName(GetWeapon(i, j).c_str())->id;

        for (int i=0 ; i<5 ; ++i)
            x84_muzzle[i] = factory.GetResourceIdByName(GetMuzzleParticle(i).c_str())->id;

        for (int i=0 ; i<5 ; ++i)
            x94_charge[i] = factory.GetResourceIdByName(GetChargeParticle(i).c_str())->id;

        for (int i=0 ; i<5 ; ++i)
            xa4_auxMuzzle[i] = factory.GetResourceIdByName(GetAuxMuzzleParticle(i).c_str())->id;

        xb4_grappleSegment = factory.GetResourceIdByName(GetGrappleSegmentParticle().c_str())->id;
        xb8_grappleClaw = factory.GetResourceIdByName(GetGrappleClawParticle().c_str())->id;
        xbc_grappleHit = factory.GetResourceIdByName(GetGrappleHitParticle().c_str())->id;
        xc0_grappleMuzzle = factory.GetResourceIdByName(GetGrappleMuzzleParticle().c_str())->id;
        xc4_grappleSwoosh = factory.GetResourceIdByName(GetGrappleSwooshParticle().c_str())->id;
    }

protected:
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

    virtual const std::string& GetWeapon(size_t idx, bool ball) const=0;
    virtual const std::string& GetMuzzleParticle(size_t idx) const=0;
    virtual const std::string& GetChargeParticle(size_t idx) const=0;
    virtual const std::string& GetAuxMuzzleParticle(size_t idx) const=0;

    virtual const std::string& GetGrappleSegmentParticle() const=0;
    virtual const std::string& GetGrappleClawParticle() const=0;
    virtual const std::string& GetGrappleHitParticle() const=0;
    virtual const std::string& GetGrappleMuzzleParticle() const=0;
    virtual const std::string& GetGrappleSwooshParticle() const=0;
};

}

#endif
