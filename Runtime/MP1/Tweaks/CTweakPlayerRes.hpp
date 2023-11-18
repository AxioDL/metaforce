#pragma once

#include "Runtime/Tweaks/ITweakPlayerRes.hpp"

namespace metaforce::MP1 {

struct CTweakPlayerRes final : Tweaks::ITweakPlayerRes {
  std::string m_saveStationIcon;
  std::string m_missileStationIcon;
  std::string m_elevatorIcon;

  std::string m_minesBreakFirstTopIcon;
  std::string m_minesBreakFirstBottomIcon;
  std::string m_minesBreakSecondTopIcon;
  std::string m_minesBreakSecondBottomIcon;

  /* ADDED IN PAL/TRILOGY */
  std::string m_mapArrowDown;
  std::string m_mapArrowUp;
  /* END */

  std::string m_lStickN;
  std::string m_lStickU;
  std::string m_lStickUL;
  std::string m_lStickL;
  std::string m_lStickDL;
  std::string m_lStickD;
  std::string m_lStickDR;
  std::string m_lStickR;
  std::string m_lStickUR;

  std::string m_cStickN;
  std::string m_cStickU;
  std::string m_cStickUL;
  std::string m_cStickL;
  std::string m_cStickDL;
  std::string m_cStickD;
  std::string m_cStickDR;
  std::string m_cStickR;
  std::string m_cStickUR;

  std::string m_lTriggerOut;
  std::string m_lTriggerIn;
  std::string m_rTriggerOut;
  std::string m_rTriggerIn;

  std::string m_startButtonOut;
  std::string m_startButtonIn;
  std::string m_aButtonOut;
  std::string m_aButtonIn;
  std::string m_bButtonOut;
  std::string m_bButtonIn;
  std::string m_xButtonOut;
  std::string m_xButtonIn;
  std::string m_yButtonOut;
  std::string m_yButtonIn;

  std::string m_ballTransitionsANCS;
  std::string m_ballTransitionsPower;
  std::string m_ballTransitionsIce;
  std::string m_ballTransitionsWave;
  std::string m_ballTransitionsPlasma;
  std::string m_ballTransitionsPhazon;

  std::string m_cinePower;
  std::string m_cineIce;
  std::string m_cineWave;
  std::string m_cinePlasma;
  std::string m_cinePhazon;

  float m_cinematicMoveOutofIntoPlayerDistance;

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
  CTweakPlayerRes(CInputStream& in, bool hasNewFields);
};

} // namespace DataSpec::DNAMP1
