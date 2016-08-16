#ifndef __DNACOMMON_ITWEAKPLAYERRES_HPP__
#define __DNACOMMON_ITWEAKPLAYERRES_HPP__

#include "../DNACommon.hpp"

namespace DataSpec
{

struct ITweakPlayerRes : BigYAML
{
    enum class EBeamId
    {
        Power,
        Ice,
        Plasma,
        Wave,
        Phazon
    };

    virtual const std::string& GetSaveStationIcon() const=0;
    virtual const std::string& GetMissileStationIcon() const=0;
    virtual const std::string& GetElevatorIcon() const=0;

    virtual const std::string& GetMinesBreakFirstTopIcon() const=0;
    virtual const std::string& GetMinesBreakFirstBottomIcon() const=0;
    virtual const std::string& GetMinesBreakSecondTopIcon() const=0;
    virtual const std::string& GetMinesBreakSecondBottomIcon() const=0;

    virtual const std::string& GetLStickN() const=0;
    virtual const std::string& GetLStickU() const=0;
    virtual const std::string& GetLStickUL() const=0;
    virtual const std::string& GetLStickL() const=0;
    virtual const std::string& GetLStickDL() const=0;
    virtual const std::string& GetLStickD() const=0;
    virtual const std::string& GetLStickDR() const=0;
    virtual const std::string& GetLStickR() const=0;
    virtual const std::string& GetLStickUR() const=0;

    virtual const std::string& GetCStickN() const=0;
    virtual const std::string& GetCStickU() const=0;
    virtual const std::string& GetCStickUL() const=0;
    virtual const std::string& GetCStickL() const=0;
    virtual const std::string& GetCStickDL() const=0;
    virtual const std::string& GetCStickD() const=0;
    virtual const std::string& GetCStickDR() const=0;
    virtual const std::string& GetCStickR() const=0;
    virtual const std::string& GetCStickUR() const=0;

    virtual const std::string& GetLTriggerOut() const=0;
    virtual const std::string& GetLTriggerIn() const=0;
    virtual const std::string& GetRTriggerOut() const=0;
    virtual const std::string& GetRTriggerIn() const=0;

    virtual const std::string& GetStartButtonOut() const=0;
    virtual const std::string& GetStartButtonIn() const=0;
    virtual const std::string& GetAButtonOut() const=0;
    virtual const std::string& GetAButtonIn() const=0;
    virtual const std::string& GetBButtonOut() const=0;
    virtual const std::string& GetBButtonIn() const=0;
    virtual const std::string& GetXButtonOut() const=0;
    virtual const std::string& GetXButtonIn() const=0;
    virtual const std::string& GetYButtonOut() const=0;
    virtual const std::string& GetYButtonIn() const=0;

    virtual const std::string& GetBallTransitionsANCS() const=0;
    virtual const std::string& GetBallTransitionsPowerBeamModel() const=0;
    virtual const std::string& GetBallTransitionsIceBeamModel() const=0;
    virtual const std::string& GetBallTransitionsWaveBeamModel() const=0;
    virtual const std::string& GetBallTransitionsPlasmaBeamModel() const=0;
    virtual const std::string& GetBallTransitionsPhazonBeamModel() const=0;

    virtual const std::string& GetPowerBeamCineModel() const=0;
    virtual const std::string& GetIceBeamCineModel() const=0;
    virtual const std::string& GetWaveBeamCineModel() const=0;
    virtual const std::string& GetPlasmaBeamCineModel() const=0;
    virtual const std::string& GetPhazonBeamCineModel() const=0;

    const std::string& GetBeamBallTransitionModel(EBeamId beam) const
    {
        int b = int(beam);
        if (b < 0 || b > 4)
            b = 0;
        switch (EBeamId(b))
        {
        case EBeamId::Power:
        default:
            return GetBallTransitionsPowerBeamModel();
        case EBeamId::Ice:
            return GetBallTransitionsIceBeamModel();
        case EBeamId::Plasma:
            return GetBallTransitionsPlasmaBeamModel();
        case EBeamId::Wave:
            return GetBallTransitionsWaveBeamModel();
        case EBeamId::Phazon:
            return GetBallTransitionsPhazonBeamModel();
        }
    }

    const std::string& GetBeamCineModel(EBeamId beam) const
    {
        int b = int(beam);
        if (b < 0 || b > 4)
            b = 0;
        switch (EBeamId(b))
        {
        case EBeamId::Power:
        default:
            return GetPowerBeamCineModel();
        case EBeamId::Ice:
            return GetIceBeamCineModel();
        case EBeamId::Plasma:
            return GetPlasmaBeamCineModel();
        case EBeamId::Wave:
            return GetWaveBeamCineModel();
        case EBeamId::Phazon:
            return GetPhazonBeamCineModel();
        }
    }
};

}

#endif // __DNACOMMON_ITWEAKPLAYERRES_HPP__
