#ifndef _DNAMP1_CTWEAKPLAYERRES_HPP_
#define _DNAMP1_CTWEAKPLAYERRES_HPP_

#include "../../DNACommon/Tweaks/ITweakPlayerRes.hpp"

namespace DataSpec
{
namespace DNAMP1
{

struct CTweakPlayerRes : ITweakPlayerRes
{
    DECL_YAML

    String<-1> m_saveStationIcon;
    String<-1> m_missileStationIcon;
    String<-1> m_elevatorIcon;

    String<-1> m_minesBreakFirstTopIcon;
    String<-1> m_minesBreakFirstBottomIcon;
    String<-1> m_minesBreakSecondTopIcon;
    String<-1> m_minesBreakSecondBottomIcon;

    String<-1> m_lStickN;
    String<-1> m_lStickU;
    String<-1> m_lStickUL;
    String<-1> m_lStickL;
    String<-1> m_lStickDL;
    String<-1> m_lStickD;
    String<-1> m_lStickDR;
    String<-1> m_lStickR;
    String<-1> m_lStickUR;

    String<-1> m_cStickN;
    String<-1> m_cStickU;
    String<-1> m_cStickUL;
    String<-1> m_cStickL;
    String<-1> m_cStickDL;
    String<-1> m_cStickD;
    String<-1> m_cStickDR;
    String<-1> m_cStickR;
    String<-1> m_cStickUR;

    String<-1> m_lTriggerOut;
    String<-1> m_lTriggerIn;
    String<-1> m_rTriggerOut;
    String<-1> m_rTriggerIn;

    String<-1> m_startButtonOut;
    String<-1> m_startButtonIn;
    String<-1> m_aButtonOut;
    String<-1> m_aButtonIn;
    String<-1> m_bButtonOut;
    String<-1> m_bButtonIn;
    String<-1> m_xButtonOut;
    String<-1> m_xButtonIn;
    String<-1> m_yButtonOut;
    String<-1> m_yButtonIn;

    String<-1> m_ballTransitionsANCS;
    String<-1> m_ballTransitionsPower;
    String<-1> m_ballTransitionsIce;
    String<-1> m_ballTransitionsWave;
    String<-1> m_ballTransitionsPlasma;
    String<-1> m_ballTransitionsPhazon;

    String<-1> m_cinePower;
    String<-1> m_cineIce;
    String<-1> m_cineWave;
    String<-1> m_cinePlasma;
    String<-1> m_cinePhazon;

    Value<float> m_unkFloat;

    const std::string& GetSaveStationIcon() const { return m_saveStationIcon; }
    const std::string& GetMissileStationIcon() const { return m_missileStationIcon; }
    const std::string& GetElevatorIcon() const { return m_elevatorIcon; }

    const std::string& GetMinesBreakFirstTopIcon() const { return m_minesBreakFirstTopIcon; }
    const std::string& GetMinesBreakFirstBottomIcon() const { return m_minesBreakFirstBottomIcon; }
    const std::string& GetMinesBreakSecondTopIcon() const { return m_minesBreakSecondTopIcon; }
    const std::string& GetMinesBreakSecondBottomIcon() const { return m_minesBreakSecondBottomIcon; }

    const std::string& GetLStick(size_t idx) const { return (&m_lStickN)[idx]; }
    const std::string& GetCStick(size_t idx) const { return (&m_cStickN)[idx]; }

    const std::string& GetLTrigger(size_t idx) const { return (&m_lTriggerOut)[idx]; }
    const std::string& GetRTrigger(size_t idx) const { return (&m_rTriggerOut)[idx]; }
    const std::string& GetStartButton(size_t idx) const { return (&m_startButtonOut)[idx]; }
    const std::string& GetAButton(size_t idx) const { return (&m_aButtonOut)[idx]; }
    const std::string& GetBButton(size_t idx) const { return (&m_bButtonOut)[idx]; }
    const std::string& GetXButton(size_t idx) const { return (&m_xButtonOut)[idx]; }
    const std::string& GetYButton(size_t idx) const { return (&m_yButtonOut)[idx]; }

    const std::string& GetBallTransitionsANCS() const { return m_ballTransitionsANCS; }

    const std::string& GetBallTransitionBeamRes(size_t idx) const { return (&m_ballTransitionsPower)[idx]; }
    const std::string& GetBeamCineModel(size_t idx) const { return (&m_cinePower)[idx]; }

    float GetUnkFloat() const { return m_unkFloat; }

    CTweakPlayerRes() = default;
    CTweakPlayerRes(athena::io::IStreamReader& in) { read(in); }
};

}
}

#endif // _DNAMP1_CTWEAKPLAYERRES_HPP_
