#ifndef __DNACOMMON_ITWEAKPLAYERRES_HPP__
#define __DNACOMMON_ITWEAKPLAYERRES_HPP__

#include "ITweak.hpp"
#include "Runtime/IFactory.hpp"
#include "Runtime/CPlayerState.hpp"

namespace DataSpec
{

struct ITweakPlayerRes : ITweak
{
    using ResId = urde::CAssetId;
    using EBeamId = urde::CPlayerState::EBeamId;

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
        x4_saveStationIcon = factory.GetResourceIdByName(_GetSaveStationIcon().c_str())->id;
        x8_missileStationIcon = factory.GetResourceIdByName(_GetMissileStationIcon().c_str())->id;
        xc_elevatorIcon = factory.GetResourceIdByName(_GetElevatorIcon().c_str())->id;

        x10_minesBreakFirstTopIcon = factory.GetResourceIdByName(_GetMinesBreakFirstTopIcon().c_str())->id;
        x14_minesBreakFirstBottomIcon = factory.GetResourceIdByName(_GetMinesBreakFirstTopIcon().c_str())->id;
        x18_minesBreakSecondTopIcon = factory.GetResourceIdByName(_GetMinesBreakFirstTopIcon().c_str())->id;
        x1c_minesBreakSecondBottomIcon = factory.GetResourceIdByName(_GetMinesBreakFirstTopIcon().c_str())->id;

        for (int i=0 ; i<9 ; ++i)
            x24_lStick[i] = factory.GetResourceIdByName(_GetLStick(i).c_str())->id;

        for (int i=0 ; i<9 ; ++i)
            x4c_cStick[i] = factory.GetResourceIdByName(_GetCStick(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            x74_lTrigger[i] = factory.GetResourceIdByName(_GetLTrigger(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            x80_rTrigger[i] = factory.GetResourceIdByName(_GetRTrigger(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            x8c_startButton[i] = factory.GetResourceIdByName(_GetStartButton(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            x98_aButton[i] = factory.GetResourceIdByName(_GetAButton(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            xa4_bButton[i] = factory.GetResourceIdByName(_GetBButton(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            xb0_xButton[i] = factory.GetResourceIdByName(_GetXButton(i).c_str())->id;

        for (int i=0 ; i<2 ; ++i)
            xbc_yButton[i] = factory.GetResourceIdByName(_GetYButton(i).c_str())->id;

        xc4_ballTransitionsANCS = factory.GetResourceIdByName(_GetBallTransitionsANCS().c_str())->id;

        for (int i=0 ; i<5 ; ++i)
            xc8_ballTransitions[i] = factory.GetResourceIdByName(_GetBallTransitionBeamRes(i).c_str())->id;

        for (int i=0 ; i<5 ; ++i)
            xc8_cineGun[i] = factory.GetResourceIdByName(_GetBeamCineModel(i).c_str())->id;

        xf0_unkFloat = _GetUnkFloat();
    }

protected:
    virtual const std::string& _GetSaveStationIcon() const=0;
    virtual const std::string& _GetMissileStationIcon() const=0;
    virtual const std::string& _GetElevatorIcon() const=0;

    virtual const std::string& _GetMinesBreakFirstTopIcon() const=0;
    virtual const std::string& _GetMinesBreakFirstBottomIcon() const=0;
    virtual const std::string& _GetMinesBreakSecondTopIcon() const=0;
    virtual const std::string& _GetMinesBreakSecondBottomIcon() const=0;

    virtual const std::string& _GetLStick(size_t idx) const=0;
    virtual const std::string& _GetCStick(size_t idx) const=0;

    virtual const std::string& _GetLTrigger(size_t idx) const=0;
    virtual const std::string& _GetRTrigger(size_t idx) const=0;
    virtual const std::string& _GetStartButton(size_t idx) const=0;
    virtual const std::string& _GetAButton(size_t idx) const=0;
    virtual const std::string& _GetBButton(size_t idx) const=0;
    virtual const std::string& _GetXButton(size_t idx) const=0;
    virtual const std::string& _GetYButton(size_t idx) const=0;

    virtual const std::string& _GetBallTransitionsANCS() const=0;

    virtual const std::string& _GetBallTransitionBeamRes(size_t idx) const=0;
    virtual const std::string& _GetBeamCineModel(size_t idx) const=0;

    virtual float _GetUnkFloat() const=0;
};

}

#endif // __DNACOMMON_ITWEAKPLAYERRES_HPP__
