#pragma once

#include <array>

#include "ITweak.hpp"
#include "Runtime/IFactory.hpp"
#include "Runtime/CPlayerState.hpp"

namespace DataSpec {

struct ITweakPlayerRes : ITweak {
  using ResId = urde::CAssetId;
  using EBeamId = urde::CPlayerState::EBeamId;

  ResId x4_saveStationIcon;
  ResId x8_missileStationIcon;
  ResId xc_elevatorIcon;

  ResId x10_minesBreakFirstTopIcon;
  ResId x14_minesBreakFirstBottomIcon;
  ResId x18_minesBreakSecondTopIcon;
  ResId x1c_minesBreakSecondBottomIcon;

  ResId  rs5_mapArrowUp;
  ResId  rs5_mapArrowDown;

  /* N, U, UL, L, DL, D, DR, R, UR */
  std::array<ResId, 9> x24_lStick;
  std::array<ResId, 9> x4c_cStick;

  /* Out, In */
  std::array<ResId, 2> x74_lTrigger;
  std::array<ResId, 2> x80_rTrigger;
  std::array<ResId, 2> x8c_startButton;
  std::array<ResId, 2> x98_aButton;
  std::array<ResId, 2> xa4_bButton;
  std::array<ResId, 2> xb0_xButton;
  std::array<ResId, 2> xbc_yButton;

  ResId xc4_ballTransitionsANCS;

  /* Power, Ice, Wave, Plasma, Phazon */
  std::array<ResId, 5> xc8_ballTransitions;
  std::array<ResId, 5> xdc_cineGun;

  float xf0_cinematicMoveOutofIntoPlayerDistance;

  ResId GetBeamBallTransitionModel(EBeamId beam) const {
    auto b = size_t(beam);
    if (b >= xc8_ballTransitions.size()) {
      b = 0;
    }
    switch (EBeamId(b)) {
    case EBeamId::Power:
    default:
      return xc8_ballTransitions[0];
    case EBeamId::Ice:
      return xc8_ballTransitions[1];
    case EBeamId::Wave:
      return xc8_ballTransitions[2];
    case EBeamId::Plasma:
      return xc8_ballTransitions[3];
    case EBeamId::Phazon:
      return xc8_ballTransitions[4];
    }
  }

  ResId GetBeamCineModel(EBeamId beam) const {
    auto b = size_t(beam);
    if (b >= xdc_cineGun.size()) {
      b = 0;
    }
    switch (EBeamId(b)) {
    case EBeamId::Power:
    default:
      return xdc_cineGun[0];
    case EBeamId::Ice:
      return xdc_cineGun[1];
    case EBeamId::Wave:
      return xdc_cineGun[2];
    case EBeamId::Plasma:
      return xdc_cineGun[3];
    case EBeamId::Phazon:
      return xdc_cineGun[4];
    }
  }

  void ResolveResources(const urde::IFactory& factory) {
    x4_saveStationIcon = factory.GetResourceIdByName(_GetSaveStationIcon())->id;
    x8_missileStationIcon = factory.GetResourceIdByName(_GetMissileStationIcon())->id;
    xc_elevatorIcon = factory.GetResourceIdByName(_GetElevatorIcon())->id;

    x10_minesBreakFirstTopIcon = factory.GetResourceIdByName(_GetMinesBreakFirstTopIcon())->id;
    x14_minesBreakFirstBottomIcon = factory.GetResourceIdByName(_GetMinesBreakFirstBottomIcon())->id;
    x18_minesBreakSecondTopIcon = factory.GetResourceIdByName(_GetMinesBreakSecondTopIcon())->id;
    x1c_minesBreakSecondBottomIcon = factory.GetResourceIdByName(_GetMinesBreakSecondBottomIcon())->id;

    for (size_t i = 0; i < x24_lStick.size(); ++i) {
      x24_lStick[i] = factory.GetResourceIdByName(_GetLStick(i))->id;
    }

    for (size_t i = 0; i < x4c_cStick.size(); ++i) {
      x4c_cStick[i] = factory.GetResourceIdByName(_GetCStick(i))->id;
    }

    for (size_t i = 0; i < x74_lTrigger.size(); ++i) {
      x74_lTrigger[i] = factory.GetResourceIdByName(_GetLTrigger(i))->id;
    }

    for (size_t i = 0; i < x80_rTrigger.size(); ++i) {
      x80_rTrigger[i] = factory.GetResourceIdByName(_GetRTrigger(i))->id;
    }

    for (size_t i = 0; i < x8c_startButton.size(); ++i) {
      x8c_startButton[i] = factory.GetResourceIdByName(_GetStartButton(i))->id;
    }

    for (size_t i = 0; i < x98_aButton.size(); ++i) {
      x98_aButton[i] = factory.GetResourceIdByName(_GetAButton(i))->id;
    }

    for (size_t i = 0; i < xa4_bButton.size(); ++i) {
      xa4_bButton[i] = factory.GetResourceIdByName(_GetBButton(i))->id;
    }

    for (size_t i = 0; i < xb0_xButton.size(); ++i) {
      xb0_xButton[i] = factory.GetResourceIdByName(_GetXButton(i))->id;
    }

    for (size_t i = 0; i < xbc_yButton.size(); ++i) {
      xbc_yButton[i] = factory.GetResourceIdByName(_GetYButton(i))->id;
    }

    xc4_ballTransitionsANCS = factory.GetResourceIdByName(_GetBallTransitionsANCS())->id;

    for (size_t i = 0; i < xc8_ballTransitions.size(); ++i) {
      xc8_ballTransitions[i] = factory.GetResourceIdByName(_GetBallTransitionBeamRes(i))->id;
    }

    for (size_t i = 0; i < xdc_cineGun.size(); ++i) {
      xdc_cineGun[i] = factory.GetResourceIdByName(_GetBeamCineModel(i))->id;
    }

    xf0_cinematicMoveOutofIntoPlayerDistance = _GetCinematicMoveOutofIntoPlayerDistance();
  }

protected:
  virtual std::string_view _GetSaveStationIcon() const = 0;
  virtual std::string_view _GetMissileStationIcon() const = 0;
  virtual std::string_view _GetElevatorIcon() const = 0;

  virtual std::string_view _GetMinesBreakFirstTopIcon() const = 0;
  virtual std::string_view _GetMinesBreakFirstBottomIcon() const = 0;
  virtual std::string_view _GetMinesBreakSecondTopIcon() const = 0;
  virtual std::string_view _GetMinesBreakSecondBottomIcon() const = 0;

  virtual std::string_view _GetLStick(size_t idx) const = 0;
  virtual std::string_view _GetCStick(size_t idx) const = 0;

  virtual std::string_view _GetLTrigger(size_t idx) const = 0;
  virtual std::string_view _GetRTrigger(size_t idx) const = 0;
  virtual std::string_view _GetStartButton(size_t idx) const = 0;
  virtual std::string_view _GetAButton(size_t idx) const = 0;
  virtual std::string_view _GetBButton(size_t idx) const = 0;
  virtual std::string_view _GetXButton(size_t idx) const = 0;
  virtual std::string_view _GetYButton(size_t idx) const = 0;

  virtual std::string_view _GetBallTransitionsANCS() const = 0;

  virtual std::string_view _GetBallTransitionBeamRes(size_t idx) const = 0;
  virtual std::string_view _GetBeamCineModel(size_t idx) const = 0;

  virtual float _GetCinematicMoveOutofIntoPlayerDistance() const = 0;
};

} // namespace DataSpec
