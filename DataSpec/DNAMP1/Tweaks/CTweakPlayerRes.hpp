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

    const std::string& GetSaveStationIcon() const { return m_saveStationIcon; }
    const std::string& GetMissileStationIcon() const { return m_missileStationIcon; }
    const std::string& GetElevatorIcon() const { return m_elevatorIcon; }

    const std::string& GetMinesBreakFirstTopIcon() const { return m_minesBreakFirstTopIcon; }
    const std::string& GetMinesBreakFirstBottomIcon() const { return m_minesBreakFirstBottomIcon; }
    const std::string& GetMinesBreakSecondTopIcon() const { return m_minesBreakSecondTopIcon; }
    const std::string& GetMinesBreakSecondBottomIcon() const { return m_minesBreakSecondBottomIcon; }

    const std::string& GetLStickN() const { return m_lStickN; }
    const std::string& GetLStickU() const { return m_lStickU; }
    const std::string& GetLStickUL() const { return m_lStickUL; }
    const std::string& GetLStickL() const { return m_lStickL; }
    const std::string& GetLStickDL() const { return m_lStickDL; }
    const std::string& GetLStickD() const { return m_lStickD; }
    const std::string& GetLStickDR() const { return m_lStickDR; }
    const std::string& GetLStickR() const { return m_lStickR; }
    const std::string& GetLStickUR() const { return m_lStickUR; }

    const std::string& GetCStickN() const { return m_cStickN; }
    const std::string& GetCStickU() const { return m_cStickU; }
    const std::string& GetCStickUL() const { return m_cStickUL; }
    const std::string& GetCStickL() const { return m_cStickL; }
    const std::string& GetCStickDL() const { return m_cStickDL; }
    const std::string& GetCStickD() const { return m_cStickD; }
    const std::string& GetCStickDR() const { return m_cStickDR; }
    const std::string& GetCStickR() const { return m_cStickR; }
    const std::string& GetCStickUR() const { return m_cStickUR; }

    const std::string& GetLTriggerOut() const { return m_lTriggerOut; }
    const std::string& GetLTriggerIn() const { return m_lTriggerIn; }
    const std::string& GetRTriggerOut() const { return m_rTriggerOut; }
    const std::string& GetRTriggerIn() const { return m_rTriggerIn; }

    const std::string& GetStartButtonOut() const { return m_startButtonOut; }
    const std::string& GetStartButtonIn() const { return m_startButtonIn; }
    const std::string& GetAButtonOut() const { return m_aButtonOut; }
    const std::string& GetAButtonIn() const { return m_aButtonIn; }
    const std::string& GetBButtonOut() const { return m_bButtonOut; }
    const std::string& GetBButtonIn() const { return m_bButtonIn; }
    const std::string& GetXButtonOut() const { return m_xButtonOut; }
    const std::string& GetXButtonIn() const { return m_xButtonIn; }
    const std::string& GetYButtonOut() const { return m_yButtonOut; }
    const std::string& GetYButtonIn() const { return m_yButtonIn; }

    const std::string& GetBallTransitionsANCS() const { return m_ballTransitionsANCS; }
    const std::string& GetBallTransitionsPowerBeamModel() const { return m_ballTransitionsPower; }
    const std::string& GetBallTransitionsIceBeamModel() const { return m_ballTransitionsIce; }
    const std::string& GetBallTransitionsWaveBeamModel() const { return m_ballTransitionsWave; }
    const std::string& GetBallTransitionsPlasmaBeamModel() const { return m_ballTransitionsPlasma; }
    const std::string& GetBallTransitionsPhazonBeamModel() const { return m_ballTransitionsPhazon; }

    const std::string& GetPowerBeamCineModel() const { return m_cinePower; }
    const std::string& GetIceBeamCineModel() const { return m_cineIce; }
    const std::string& GetWaveBeamCineModel() const { return m_cineWave; }
    const std::string& GetPlasmaBeamCineModel() const { return m_cinePlasma; }
    const std::string& GetPhazonBeamCineModel() const { return m_cinePhazon; }
};

}
}

#endif // _DNAMP1_CTWEAKPLAYERRES_HPP_
