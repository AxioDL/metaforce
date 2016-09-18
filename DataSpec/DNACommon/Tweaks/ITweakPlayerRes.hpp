#ifndef __DNACOMMON_ITWEAKPLAYERRES_HPP__
#define __DNACOMMON_ITWEAKPLAYERRES_HPP__

#include "../DNACommon.hpp"
#include "Runtime/IFactory.hpp"

namespace DataSpec
{

struct ITweakPlayerRes : BigYAML
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

    ResId x4_saveStationIcon;
    ResId x8_missileStationIcon;
    ResId xc_elevatorIcon;

    ResId x10_minesBreakFirstTopIcon;
    ResId x14_minesBreakFirstBottomIcon;
    ResId x18_minesBreakSecondTopIcon;
    ResId x1c_minesBreakSecondBottomIcon;

    /* N, U, UL, L, DL, D, DR, R, UR */
    ResId x24_lStick[9];
    ResId x4c_cStick[9];

    /* Out, In */
    ResId x74_lTrigger[2];
    ResId x80_rTrigger[2];
    ResId x8c_startButton[2];
    ResId x98_aButton[2];
    ResId xa4_bButton[2];
    ResId xb0_xButton[2];
    ResId xbc_yButton[2];

    ResId xc4_ballTransitionsANCS;

    /* Power, Ice, Wave, Plasma, Phazon */
    ResId xc8_ballTransitions[5];
    ResId xc8_cineGun[5];

    float xf0_unkFloat;

    ResId GetBeamBallTransitionModel(EBeamId beam) const
    {
        int b = int(beam);
        if (b < 0 || b > 4)
            b = 0;
        switch (EBeamId(b))
        {
        case EBeamId::Power:
        default:
            return xc8_ballTransitions[0];
        case EBeamId::Ice:
            return xc8_ballTransitions[1];
        case EBeamId::Plasma:
            return xc8_ballTransitions[3];
        case EBeamId::Wave:
            return xc8_ballTransitions[2];
        case EBeamId::Phazon:
            return xc8_ballTransitions[4];
        }
    }

    ResId GetBeamCineModel(EBeamId beam) const
    {
        int b = int(beam);
        if (b < 0 || b > 4)
            b = 0;
        switch (EBeamId(b))
        {
        case EBeamId::Power:
        default:
            return xc8_cineGun[0];
        case EBeamId::Ice:
            return xc8_cineGun[1];
        case EBeamId::Plasma:
            return xc8_cineGun[3];
        case EBeamId::Wave:
            return xc8_cineGun[2];
        case EBeamId::Phazon:
            return xc8_cineGun[4];
        }
    }

    void ResolveResources(const urde::IFactory& factory)
    {
        x4_saveStationIcon = factory.GetResourceIdByName(GetSaveStationIcon().c_str())->id;
        x8_missileStationIcon = factory.GetResourceIdByName(GetMissileStationIcon().c_str())->id;
        xc_elevatorIcon = factory.GetResourceIdByName(GetElevatorIcon().c_str())->id;

        x10_minesBreakFirstTopIcon = factory.GetResourceIdByName(GetMinesBreakFirstTopIcon().c_str())->id;
        x14_minesBreakFirstBottomIcon = factory.GetResourceIdByName(GetMinesBreakFirstTopIcon().c_str())->id;
        x18_minesBreakSecondTopIcon = factory.GetResourceIdByName(GetMinesBreakFirstTopIcon().c_str())->id;
        x1c_minesBreakSecondBottomIcon = factory.GetResourceIdByName(GetMinesBreakFirstTopIcon().c_str())->id;

        for (int i=0 ; i<9 ; ++i)
            x24_lStick[i] = factory.GetResourceIdByName(GetLStick(i).c_str())->id;

        for (int i=0 ; i<9 ; ++i)
            x4c_cStick[i] = factory.GetResourceIdByName(GetCStick(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            x74_lTrigger[i] = factory.GetResourceIdByName(GetLTrigger(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            x80_rTrigger[i] = factory.GetResourceIdByName(GetRTrigger(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            x8c_startButton[i] = factory.GetResourceIdByName(GetStartButton(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            x98_aButton[i] = factory.GetResourceIdByName(GetAButton(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            xa4_bButton[i] = factory.GetResourceIdByName(GetBButton(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            xb0_xButton[i] = factory.GetResourceIdByName(GetXButton(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            xbc_yButton[i] = factory.GetResourceIdByName(GetYButton(i).c_str())->id;

        xc4_ballTransitionsANCS = factory.GetResourceIdByName(GetBallTransitionsANCS().c_str())->id;

        for (int i=0 ; i<5 ; ++i)
            xc8_ballTransitions[i] = factory.GetResourceIdByName(GetBallTransitionModel(i).c_str())->id;

        for (int i=0 ; i<5 ; ++i)
            xc8_cineGun[i] = factory.GetResourceIdByName(GetBeamCineModel(i).c_str())->id;

        xf0_unkFloat = GetUnkFloat();
    }

protected:
    virtual const std::string& GetSaveStationIcon() const=0;
    virtual const std::string& GetMissileStationIcon() const=0;
    virtual const std::string& GetElevatorIcon() const=0;

    virtual const std::string& GetMinesBreakFirstTopIcon() const=0;
    virtual const std::string& GetMinesBreakFirstBottomIcon() const=0;
    virtual const std::string& GetMinesBreakSecondTopIcon() const=0;
    virtual const std::string& GetMinesBreakSecondBottomIcon() const=0;

    virtual const std::string& GetLStick(size_t idx) const=0;
    virtual const std::string& GetCStick(size_t idx) const=0;

    virtual const std::string& GetLTrigger(size_t idx) const=0;
    virtual const std::string& GetRTrigger(size_t idx) const=0;
    virtual const std::string& GetStartButton(size_t idx) const=0;
    virtual const std::string& GetAButton(size_t idx) const=0;
    virtual const std::string& GetBButton(size_t idx) const=0;
    virtual const std::string& GetXButton(size_t idx) const=0;
    virtual const std::string& GetYButton(size_t idx) const=0;

    virtual const std::string& GetBallTransitionsANCS() const=0;

    virtual const std::string& GetBallTransitionModel(size_t idx) const=0;
    virtual const std::string& GetBeamCineModel(size_t idx) const=0;

    virtual float GetUnkFloat() const=0;
};

}

#endif // __DNACOMMON_ITWEAKPLAYERRES_HPP__
