#include "Runtime/MP1/Tweaks/CTweakPlayerRes.hpp"

namespace metaforce::MP1 {
CTweakPlayerRes::CTweakPlayerRes(CInputStream& in, bool hasNewFields) {
  m_saveStationIcon = in.Get<std::string>();
  m_missileStationIcon = in.Get<std::string>();
  m_elevatorIcon = in.Get<std::string>();

  m_minesBreakFirstTopIcon = in.Get<std::string>();
  m_minesBreakFirstBottomIcon = in.Get<std::string>();
  m_minesBreakSecondTopIcon = in.Get<std::string>();
  m_minesBreakSecondBottomIcon = in.Get<std::string>();

  /* ADDED IN JP/PAL/TRILOGY */
  if (hasNewFields) {
    m_mapArrowDown = in.Get<std::string>();
    m_mapArrowUp = in.Get<std::string>();
  }
  /* END */

  m_lStickN = in.Get<std::string>();
  m_lStickU = in.Get<std::string>();
  m_lStickUL = in.Get<std::string>();
  m_lStickL = in.Get<std::string>();
  m_lStickDL = in.Get<std::string>();
  m_lStickD = in.Get<std::string>();
  m_lStickDR = in.Get<std::string>();
  m_lStickR = in.Get<std::string>();
  m_lStickUR = in.Get<std::string>();

  m_cStickN = in.Get<std::string>();
  m_cStickU = in.Get<std::string>();
  m_cStickUL = in.Get<std::string>();
  m_cStickL = in.Get<std::string>();
  m_cStickDL = in.Get<std::string>();
  m_cStickD = in.Get<std::string>();
  m_cStickDR = in.Get<std::string>();
  m_cStickR = in.Get<std::string>();
  m_cStickUR = in.Get<std::string>();

  m_lTriggerOut = in.Get<std::string>();
  m_lTriggerIn = in.Get<std::string>();
  m_rTriggerOut = in.Get<std::string>();
  m_rTriggerIn = in.Get<std::string>();

  m_startButtonOut = in.Get<std::string>();
  m_startButtonIn = in.Get<std::string>();
  m_aButtonOut = in.Get<std::string>();
  m_aButtonIn = in.Get<std::string>();
  m_bButtonOut = in.Get<std::string>();
  m_bButtonIn = in.Get<std::string>();
  m_xButtonOut = in.Get<std::string>();
  m_xButtonIn = in.Get<std::string>();
  m_yButtonOut = in.Get<std::string>();
  m_yButtonIn = in.Get<std::string>();

  m_ballTransitionsANCS = in.Get<std::string>();
  m_ballTransitionsPower = in.Get<std::string>();
  m_ballTransitionsIce = in.Get<std::string>();
  m_ballTransitionsWave = in.Get<std::string>();
  m_ballTransitionsPlasma = in.Get<std::string>();
  m_ballTransitionsPhazon = in.Get<std::string>();

  m_cinePower = in.Get<std::string>();
  m_cineIce = in.Get<std::string>();
  m_cineWave = in.Get<std::string>();
  m_cinePlasma = in.Get<std::string>();
  m_cinePhazon = in.Get<std::string>();

  m_cinematicMoveOutofIntoPlayerDistance = in.ReadFloat();
}
} // namespace metaforce::MP1