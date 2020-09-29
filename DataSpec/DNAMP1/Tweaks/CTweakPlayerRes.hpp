#pragma once

#include "DataSpec/DNACommon/Tweaks/ITweakPlayerRes.hpp"

namespace DataSpec::DNAMP1 {

template <bool NewRep>
struct AT_SPECIALIZE_PARMS(true, false) CTweakPlayerRes final : ITweakPlayerRes {
  AT_DECL_DNA_YAML

  String<-1> m_saveStationIcon;
  String<-1> m_missileStationIcon;
  String<-1> m_elevatorIcon;

  String<-1> m_minesBreakFirstTopIcon;
  String<-1> m_minesBreakFirstBottomIcon;
  String<-1> m_minesBreakSecondTopIcon;
  String<-1> m_minesBreakSecondBottomIcon;

  String<AT_DNA_COUNT(NewRep == true ? -1 : 0)> m_mapArrowDown;
  String<AT_DNA_COUNT(NewRep == true ? -1 : 0)> m_mapArrowUp;

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

  Value<float> m_cinematicMoveOutofIntoPlayerDistance;

  std::string_view _GetSaveStationIcon() const override { return m_saveStationIcon; }
  std::string_view _GetMissileStationIcon() const override { return m_missileStationIcon; }
  std::string_view _GetElevatorIcon() const override { return m_elevatorIcon; }

  std::string_view _GetMinesBreakFirstTopIcon() const override { return m_minesBreakFirstTopIcon; }
  std::string_view _GetMinesBreakFirstBottomIcon() const override { return m_minesBreakFirstBottomIcon; }
  std::string_view _GetMinesBreakSecondTopIcon() const override { return m_minesBreakSecondTopIcon; }
  std::string_view _GetMinesBreakSecondBottomIcon() const override { return m_minesBreakSecondBottomIcon; }

  std::string_view _GetLStick(size_t idx) const override { return (&m_lStickN)[idx]; }
  std::string_view _GetCStick(size_t idx) const override { return (&m_cStickN)[idx]; }

  std::string_view _GetLTrigger(size_t idx) const override { return (&m_lTriggerOut)[idx]; }
  std::string_view _GetRTrigger(size_t idx) const override { return (&m_rTriggerOut)[idx]; }
  std::string_view _GetStartButton(size_t idx) const override { return (&m_startButtonOut)[idx]; }
  std::string_view _GetAButton(size_t idx) const override { return (&m_aButtonOut)[idx]; }
  std::string_view _GetBButton(size_t idx) const override { return (&m_bButtonOut)[idx]; }
  std::string_view _GetXButton(size_t idx) const override { return (&m_xButtonOut)[idx]; }
  std::string_view _GetYButton(size_t idx) const override { return (&m_yButtonOut)[idx]; }

  std::string_view _GetBallTransitionsANCS() const override { return m_ballTransitionsANCS; }

  std::string_view _GetBallTransitionBeamRes(size_t idx) const override { return (&m_ballTransitionsPower)[idx]; }
  std::string_view _GetBeamCineModel(size_t idx) const override { return (&m_cinePower)[idx]; }

  float _GetCinematicMoveOutofIntoPlayerDistance() const override { return m_cinematicMoveOutofIntoPlayerDistance; }

  CTweakPlayerRes() = default;
  CTweakPlayerRes(athena::io::IStreamReader& in) { read(in); }
};

} // namespace DataSpec::DNAMP1
