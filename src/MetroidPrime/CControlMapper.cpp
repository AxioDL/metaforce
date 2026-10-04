#include "MetroidPrime/CControlMapper.hpp"

#if VERSION >= VERSION_R3IJ_00

#include "MetroidPrime/CStateManager.hpp"
#include "MetroidPrime/Cameras/CCameraManager.hpp"
#include "MetroidPrime/Player/CGameOptions.hpp"
#include "MetroidPrime/Player/CGameState.hpp"
#include "MetroidPrime/Player/CPlayer.hpp"
#include "MetroidPrime/Tweaks/CTweakPlayerControl.hpp"
#include "MetroidPrime/Tweaks/CTweaks.hpp"

#include "Kyoto/Math/CMath.hpp"
#include "Kyoto/Math/CloseEnough.hpp"

const float CControlMapper::skDefaultTapHoldThreshold = 0.175f;

CControlMapper::CControlMapper(float tapHoldThreshold)
: mCommandEnabled(true)
, mCommandOverridden(false)
, mSelectorActive(0)
, mSelectorFadeTime(0.f)
, mActiveSelectorCommand(kC_None)
, mReleasedSelectorCommand(kC_None)
, mTapHoldThreshold(tapHoldThreshold)
, mDigitalTime(91)
, mReleaseTime(91) {
  Reset();
}

void CControlMapper::UpdateCommandTimes(const CFinalInput& input) {
  for (int i = 0; i < kC_MAX; ++i) {
    ECommands command = static_cast< ECommands >(i);
    if (GetPressInput(command, input, kFT_Unfiltered)) {
      mDigitalTime[i] = 0.f;
      mReleaseTime[i] = 0.f;
    } else if (GetDigitalInput(command, input, kFT_Unfiltered)) {
      mDigitalTime[i] += input.Time();
      mReleaseTime[i] += input.Time();
    } else if (GetReleaseInput(command, input, kFT_Unfiltered)) {
      mDigitalTime[i] = 0.f;
    }
  }
}

bool CControlMapper::CanOpenSelector(const CStateManager& mgr, const CPlayer& player,
                                     ECommands command) const {
  return mgr.GetGameState() == CStateManager::kGS_Running &&
         mgr.GetCameraManager()->IsInFPCamera() && player.CanOpenSelector(mgr, command);
}

void CControlMapper::UpdateSelector(ECommands command, const CFinalInput& input,
                                    const CStateManager& mgr, const CPlayer& player) {
  if (mSelectorActive != 0 && command == mActiveSelectorCommand &&
      !CanOpenSelector(mgr, player, command)) {
    ResetSelector();
    mSelectorFadeTime = 0.5f;
  } else if (mActiveSelectorCommand == kC_None || mActiveSelectorCommand == command) {
    switch (mSelectorActive) {
    case 1:
      if (!GetDigitalInput(command, input, kFT_Filtered)) {
        mSelectorActive = 0;
        mSelectorFadeTime = 0.5f;
        mReleasedSelectorCommand = mActiveSelectorCommand;
        mActiveSelectorCommand = kC_None;
      }
      break;
    case 0:
      if (GetDigitalInput(command, input, kFT_Filtered) &&
          mDigitalTime[command] > mTapHoldThreshold &&
          CanOpenSelector(mgr, player, command)) {
        mSelectorActive = 1;
        mActiveSelectorCommand = command;
      }
      break;
    }
  }
}

float CControlMapper::GetSelectorFade() const {
  float result = 0.f;
  if (mSelectorFadeTime > 0.f) {
    const float& minFade = 0.f;
    const float& maxFade = 1.f;
    result = CMath::FastMin(CMath::FastMax(minFade, mSelectorFadeTime / 0.5f), maxFade);
  }
  return result;
}

bool CControlMapper::GetSelectorReleaseInput(ECommands command, const CFinalInput& input,
                                             const CStateManager& mgr,
                                             const CPlayer& player) const {
#if NONMATCHING
  if (command < 0 || command >= kC_MAX) {
    return false;
  }
#endif
  if (CanOpenSelector(mgr, player, command)) {
    return command == mReleasedSelectorCommand;
  }
  return false;
}

void CControlMapper::UpdateCommandSwaps() {
  bool swapped = IsCommandRemapped(kC_BeamMenu);
  if (swapped != gpGameState->GameOptions().GetIsSwitchVisorBeamControls()) {
    SwapCommands(kC_BeamMenu, kC_VisorMenu, !swapped);
  }
  swapped = IsCommandRemapped(kC_JumpOrBoost);
  if (swapped != gpGameState->GameOptions().GetIsFireAndJumpSwapped()) {
    SwapCommands(kC_JumpOrBoost, kC_FireOrBomb, !swapped);
    SetCommandMapping(kC_Command16, GetCommandMapping(kC_FireOrBomb));
  }
}

void CControlMapper::Update(const CFinalInput& input, const CStateManager& mgr,
                            const CPlayer& player) {
  UpdateCommandSwaps();
  UpdateCommandTimes(input);
  if (mSelectorFadeTime > 0.f) {
    mSelectorFadeTime -= input.Time();
    if (mSelectorFadeTime <= 0.f) {
      ResetSelector();
    }
  }
  mReleasedSelectorCommand = kC_None;
  UpdateSelector(kC_VisorMenu, input, mgr, player);
  UpdateSelector(kC_BeamMenu, input, mgr, player);
}

bool CControlMapper::IsSplineControl(int control) {
  return static_cast< uint >(control - CFinalInput::kPC_NunchukRollNegative) <=
         CFinalInput::kPC_WiimotePitchPositive - CFinalInput::kPC_NunchukRollNegative;
}

float CControlMapper::GetAnalogInput(ECommands command, const CFinalInput& input,
                                     EFilterType filter) const {
#if NONMATCHING
  if (command < 0 || command >= kC_MAX) {
    return 0.f;
  }
#endif
  float result = 0.f;
  SCommandMapping mapping = GetCommandMapping(command);
  if (filter == kFT_Unfiltered || (filter == kFT_Filtered && mCommandEnabled[command])) {
    const CTweakPlayerControl::SCommandDescription& desc =
        gpTweakPlayerControlCurrent->GetCommandDescription(command);
    switch (desc.mType) {
    case CTweakPlayerControl::kCT_Physical: {
      result = input.GetAnalog(mapping.mPrimaryControl);
      if (IsSplineControl(mapping.mPrimaryControl)) {
        result = desc.mPrimary.mResponse.EvaluateAt(result);
      }
      break;
    }
    case CTweakPlayerControl::kCT_PhysicalCombination: {
      result = input.GetAnalog(mapping.mPrimaryControl);
      switch (desc.mPhysicalBoolean) {
      case CTweakPlayerControl::kCB_And: {
        if (!input.GetDigital(mapping.mSecondaryControl)) {
          result = 0.f;
        } else if (IsSplineControl(mapping.mPrimaryControl) &&
                   input.CheckValidControl(mapping.mPrimaryControl)) {
          result = desc.mPrimary.mResponse.EvaluateAt(result);
        }
        break;
      }
      case CTweakPlayerControl::kCB_Or: {
        const CMayaSpline* response = &desc.mPrimary.mResponse;
        if (close_enough(result, 0.f, 0.05f) ||
            !input.CheckValidControl(mapping.mPrimaryControl)) {
          result = input.GetAnalog(mapping.mSecondaryControl);
          response = &desc.mSecondary.mResponse;
        }
        if ((IsSplineControl(mapping.mPrimaryControl) &&
             input.CheckValidControl(mapping.mPrimaryControl)) ||
            (IsSplineControl(mapping.mSecondaryControl) &&
             input.CheckValidControl(mapping.mSecondaryControl))) {
          result = response->EvaluateAt(result);
        }
        break;
      }
      case CTweakPlayerControl::kCB_AndNot: {
        if (input.GetDigital(mapping.mSecondaryControl)) {
          result = 0.f;
        } else if (IsSplineControl(mapping.mPrimaryControl) &&
                   input.CheckValidControl(mapping.mPrimaryControl)) {
          result = desc.mPrimary.mResponse.EvaluateAt(result);
        }
        break;
      }
      }
      break;
    }
    case CTweakPlayerControl::kCT_Virtual: {
      result = input.GetMotionAnalog(mapping.mPrimaryControl);
      break;
    }
    case CTweakPlayerControl::kCT_VirtualCombination: {
      result = input.GetMotionAnalog(mapping.mPrimaryControl);
      switch (desc.mVirtualBoolean) {
      case CTweakPlayerControl::kCB_And: {
        result += input.GetMotionAnalog(mapping.mSecondaryControl);
        break;
      }
      case CTweakPlayerControl::kCB_Or: {
        if (close_enough(result, 0.f, 0.05f) ||
            !input.CheckValidControl(mapping.mPrimaryControl)) {
          result = input.GetMotionAnalog(mapping.mSecondaryControl);
        }
        break;
      }
      case CTweakPlayerControl::kCB_AndNot: {
        if (input.GetMotionDigital(mapping.mSecondaryControl)) {
          result = 0.f;
        }
        break;
      }
      }
      break;
    }
    case CTweakPlayerControl::kCT_Virtual2: {
      result = input.GetSwingAnalog(mapping.mPrimaryControl);
      break;
    }
    }
  }
  return result;
}

bool CControlMapper::TestVirtualMenu(ECommands command, const CVector2f& pointer) const {
  float limit = 1.f;
  CVector2f position(CMath::FastLimit(pointer.GetX(), limit),
                     CMath::FastLimit(pointer.GetY(), limit));
  float halfWidth = 320.f;
  float halfHeight = 224.f;
  position[0] *= halfWidth;
  position[1] *= halfHeight;
  position[0] = halfWidth + position[0];
  position[1] = halfHeight - position[1];
  const CTweakPlayerControl::SCommandDescription& desc =
      gpTweakPlayerControlCurrent->GetCommandDescription(command);
  if (desc.mType != CTweakPlayerControl::kCT_VirtualMenu) {
    return false;
  }

  bool result = false;
  switch (desc.mVirtualMenu.mShape) {
  case CTweakPlayerControl::kVMS_Annulus: {
    CVector2f offset = position - CVector2f(desc.mVirtualMenu.mAnnulus.mCenterX,
                                            desc.mVirtualMenu.mAnnulus.mCenterY);
    float distanceSquared = offset.MagSquared();
    if (distanceSquared <= desc.mVirtualMenu.mAnnulus.mOuterRadius *
                               desc.mVirtualMenu.mAnnulus.mOuterRadius &&
        distanceSquared >= desc.mVirtualMenu.mAnnulus.mInnerRadius *
                               desc.mVirtualMenu.mAnnulus.mInnerRadius) {
      result = true;
    }
    break;
  }
  case CTweakPlayerControl::kVMS_Rectangle: {
    float half = 0.5f;
    float halfWidth = half * desc.mVirtualMenu.mRectangle.mWidth;
    if (position.GetX() <= desc.mVirtualMenu.mRectangle.mCenterX + halfWidth &&
        position.GetX() > desc.mVirtualMenu.mRectangle.mCenterX - halfWidth) {
      float halfHeight = half * desc.mVirtualMenu.mRectangle.mHeight;
      if (position.GetY() <= desc.mVirtualMenu.mRectangle.mCenterY + halfHeight &&
          position.GetY() > desc.mVirtualMenu.mRectangle.mCenterY - halfHeight) {
        result = true;
      }
    }
    break;
  }
  case CTweakPlayerControl::kVMS_Sector: {
    CVector2f offset = position - CVector2f(desc.mVirtualMenu.mSector.mCenterX,
                                            desc.mVirtualMenu.mSector.mCenterY);
    float distanceSquared = offset.MagSquared();
    if (distanceSquared <= desc.mVirtualMenu.mSector.mOuterRadius *
                               desc.mVirtualMenu.mSector.mOuterRadius &&
        distanceSquared >= desc.mVirtualMenu.mSector.mInnerRadius *
                               desc.mVirtualMenu.mSector.mInnerRadius) {
      CVector2f direction(
          sinf((M_PIF / 180.f) * desc.mVirtualMenu.mSector.mCenterAngleDegrees),
          cosf((M_PIF / 180.f) * desc.mVirtualMenu.mSector.mCenterAngleDegrees));
      if (acosf(CVector2f::Dot(direction.AsNormalized(), offset.AsNormalized())) <
          0.5f * ((M_PIF / 180.f) * desc.mVirtualMenu.mSector.mSweepDegrees)) {
        result = true;
      }
    }
  } break;
  }
  return result;
}

bool CControlMapper::GetDigitalInput(ECommands command, const CFinalInput& input,
                                     EFilterType filter) const {
#if NONMATCHING
  if (command < 0 || command >= kC_MAX) {
    return false;
  }
#endif
  bool result = false;
  SCommandMapping mapping = GetCommandMapping(command);
  if (filter == kFT_Unfiltered || (filter == kFT_Filtered && mCommandEnabled[command])) {
    const CTweakPlayerControl::SCommandDescription& desc =
        gpTweakPlayerControlCurrent->GetCommandDescription(command);
    switch (desc.mType) {
    case CTweakPlayerControl::kCT_Physical: {
      result = input.GetDigital(mapping.mPrimaryControl);
      break;
    }
    case CTweakPlayerControl::kCT_PhysicalCombination: {
      switch (desc.mPhysicalBoolean) {
      case CTweakPlayerControl::kCB_And: {
        result = input.GetDigital(mapping.mPrimaryControl) &&
                 input.GetDigital(mapping.mSecondaryControl);
        break;
      }
      case CTweakPlayerControl::kCB_Or: {
        result = input.GetDigital(mapping.mPrimaryControl) ||
                 input.GetDigital(mapping.mSecondaryControl);
        break;
      }
      case CTweakPlayerControl::kCB_AndNot: {
        result = input.GetDigital(mapping.mPrimaryControl) &&
                 !input.GetDigital(mapping.mSecondaryControl);
        break;
      }
      }
      break;
    }
    case CTweakPlayerControl::kCT_Virtual: {
      result = input.GetMotionDigital(mapping.mPrimaryControl);
      break;
    }
    case CTweakPlayerControl::kCT_VirtualCombination: {
      switch (desc.mVirtualBoolean) {
      case CTweakPlayerControl::kCB_And: {
        result = input.GetMotionDigital(mapping.mPrimaryControl) &&
                 input.GetMotionDigital(mapping.mSecondaryControl);
        break;
      }
      case CTweakPlayerControl::kCB_Or: {
        result = input.GetMotionDigital(mapping.mPrimaryControl) ||
                 input.GetMotionDigital(mapping.mSecondaryControl);
        break;
      }
      case CTweakPlayerControl::kCB_AndNot: {
        result = input.GetMotionDigital(mapping.mPrimaryControl) &&
                 !input.GetMotionDigital(mapping.mSecondaryControl);
        break;
      }
      }
      break;
    }
    case CTweakPlayerControl::kCT_Virtual2: {
      result = input.GetSwingDigital(mapping.mPrimaryControl);
      break;
    }
    case CTweakPlayerControl::kCT_VirtualMenu: {
      result = TestVirtualMenu(command, input.GetPointerPosition());
      break;
    }
    }
  }
  return result;
}

bool CControlMapper::GetPressInput(ECommands command, const CFinalInput& input,
                                   EFilterType filter) const {
#if NONMATCHING
  if (command < 0 || command >= kC_MAX) {
    return false;
  }
#endif
  bool result = false;
  SCommandMapping mapping = GetCommandMapping(command);
  if (filter == kFT_Unfiltered || (filter == kFT_Filtered && mCommandEnabled[command])) {
    const CTweakPlayerControl::SCommandDescription& desc =
        gpTweakPlayerControlCurrent->GetCommandDescription(command);
    switch (desc.mType) {
    case CTweakPlayerControl::kCT_Physical: {
      result = input.GetPressed(mapping.mPrimaryControl);
      break;
    }
    case CTweakPlayerControl::kCT_PhysicalCombination: {
      switch (desc.mPhysicalBoolean) {
      case CTweakPlayerControl::kCB_And: {
        result = input.GetPressed(mapping.mPrimaryControl) &&
                 input.GetPressed(mapping.mSecondaryControl);
        break;
      }
      case CTweakPlayerControl::kCB_Or: {
        result = input.GetPressed(mapping.mPrimaryControl) ||
                 input.GetPressed(mapping.mSecondaryControl);
        break;
      }
      case CTweakPlayerControl::kCB_AndNot: {
        result = input.GetPressed(mapping.mPrimaryControl) &&
                 !input.GetPressed(mapping.mSecondaryControl);
        break;
      }
      }
      break;
    }
    case CTweakPlayerControl::kCT_Virtual: {
      result = input.GetMotionPressed(mapping.mPrimaryControl);
      break;
    }
    case CTweakPlayerControl::kCT_VirtualCombination: {
      switch (desc.mVirtualBoolean) {
      case CTweakPlayerControl::kCB_And: {
        result = input.GetMotionPressed(mapping.mPrimaryControl) &&
                 input.GetMotionPressed(mapping.mSecondaryControl);
        break;
      }
      case CTweakPlayerControl::kCB_Or: {
        result = input.GetMotionPressed(mapping.mPrimaryControl) ||
                 input.GetMotionPressed(mapping.mSecondaryControl);
        break;
      }
      case CTweakPlayerControl::kCB_AndNot: {
        result = input.GetMotionPressed(mapping.mPrimaryControl) &&
                 !input.GetMotionPressed(mapping.mSecondaryControl);
        break;
      }
      }
      break;
    }
    case CTweakPlayerControl::kCT_Virtual2: {
      result = input.GetSwingPressed(mapping.mPrimaryControl);
      break;
    }
    case CTweakPlayerControl::kCT_VirtualMenu: {
      result = TestVirtualMenu(command, input.GetPointerPosition());
      break;
    }
    }
  }
  return result;
}

bool CControlMapper::GetReleaseInput(ECommands command, const CFinalInput& input,
                                     EFilterType filter) const {
#if NONMATCHING
  if (command < 0 || command >= kC_MAX) {
    return false;
  }
#endif
  bool result = false;
  SCommandMapping mapping = GetCommandMapping(command);
  if (filter == kFT_Unfiltered || (filter == kFT_Filtered && mCommandEnabled[command])) {
    const CTweakPlayerControl::SCommandDescription& desc =
        gpTweakPlayerControlCurrent->GetCommandDescription(command);
    switch (desc.mType) {
    case CTweakPlayerControl::kCT_Physical: {
      result = input.GetReleased(mapping.mPrimaryControl);
      break;
    }
    case CTweakPlayerControl::kCT_PhysicalCombination: {
      switch (desc.mPhysicalBoolean) {
      case CTweakPlayerControl::kCB_And: {
        result = input.GetReleased(mapping.mPrimaryControl) &&
                 input.GetReleased(mapping.mSecondaryControl);
        break;
      }
      case CTweakPlayerControl::kCB_Or: {
        result = input.GetReleased(mapping.mPrimaryControl) ||
                 input.GetReleased(mapping.mSecondaryControl);
        break;
      }
      case CTweakPlayerControl::kCB_AndNot: {
        result = input.GetReleased(mapping.mPrimaryControl) &&
                 !input.GetReleased(mapping.mSecondaryControl);
        break;
      }
      }
      break;
    }
    case CTweakPlayerControl::kCT_Virtual: {
      result = input.GetMotionReleased(mapping.mPrimaryControl);
      break;
    }
    case CTweakPlayerControl::kCT_VirtualCombination: {
      switch (desc.mVirtualBoolean) {
      case CTweakPlayerControl::kCB_And: {
        result = input.GetMotionReleased(mapping.mPrimaryControl) &&
                 input.GetMotionReleased(mapping.mSecondaryControl);
        break;
      }
      case CTweakPlayerControl::kCB_Or: {
        result = input.GetMotionReleased(mapping.mPrimaryControl) ||
                 input.GetMotionReleased(mapping.mSecondaryControl);
        break;
      }
      case CTweakPlayerControl::kCB_AndNot: {
        result = input.GetMotionReleased(mapping.mPrimaryControl) &&
                 !input.GetMotionReleased(mapping.mSecondaryControl);
        break;
      }
      }
      break;
    }
    case CTweakPlayerControl::kCT_Virtual2: {
      result = input.GetSwingReleased(mapping.mPrimaryControl);
      break;
    }
    }
  }
  return result;
}

bool CControlMapper::GetTapInput(ECommands command, const CFinalInput& input,
                                 EFilterType filter) const {
  return GetReleaseInput(command, input, filter) &&
         mReleaseTime[command] <= mTapHoldThreshold && mReleaseTime[command] > 0.f;
}

void CControlMapper::ResetCommandFilters() {
  mCommandEnabled.clear();
  for (int i = 0; i < kC_MAX; ++i) {
    mCommandEnabled.push_back(true);
  }
}

void CControlMapper::SetCommandEnabled(ECommands command, bool enabled) {
#if NONMATCHING
  if (command < 0 || command >= kC_MAX) {
    return;
  }
#endif
  mCommandEnabled[command] = enabled;
}

void CControlMapper::SetCommandMapping(ECommands command, const SCommandMapping& mapping) {
#if NONMATCHING
  if (command < 0 || command >= kC_MAX) {
    return;
  }
#endif
  if (!mCommandOverridden.empty()) {
    if (mCommandOverridden[command]) {
      for (rstl::reserved_vector< SCommandOverride, 12 >::iterator it =
               mCommandOverrides.begin();
           it != mCommandOverrides.end(); ++it) {
        if (it->mCommand == command) {
          it->mMapping = mapping;
        }
      }
    } else {
      if (mCommandOverrides.size() != mCommandOverrides.capacity()) {
        mCommandOverridden[command] = true;
        mCommandOverrides.push_back(SCommandOverride(command, mapping));
      }
    }
  }
}

void CControlMapper::RestoreCommandMapping(ECommands command) {
  if (!mCommandOverridden.empty() && IsCommandRemapped(command)) {
    mCommandOverridden[command] = false;
    for (rstl::reserved_vector< SCommandOverride, 12 >::iterator it = mCommandOverrides.begin();
         it != mCommandOverrides.end(); ++it) {
      if (it->mCommand == command) {
        mCommandOverrides.erase(it);
        return;
      }
    }
  }
}

bool CControlMapper::IsCommandRemapped(ECommands command) const {
#if NONMATCHING
  if (command < 0 || command >= kC_MAX) {
    return false;
  }
#endif
  return command < mCommandOverridden.size() && mCommandOverridden[command];
}

CControlMapper::SCommandMapping CControlMapper::GetCommandMapping(ECommands command) const {
#if NONMATCHING
  if (command < 0 || command >= kC_MAX) {
    return SCommandMapping(CTweakPlayerControl::kCT_None, 0, 0);
  }
#endif
  if (!mCommandOverridden.empty() && command < mCommandOverridden.size() &&
      mCommandOverridden[command]) {
    for (rstl::reserved_vector< SCommandOverride, 12 >::const_iterator it =
             mCommandOverrides.begin();
         it != mCommandOverrides.end(); ++it) {
      if (it->mCommand == command) {
        return it->mMapping;
      }
    }
  }
  SCommandMapping mapping = gpTweakPlayerControlCurrent->GetCommandMapping(command);
  return mapping;
}

void CControlMapper::ResetCommandMappings() {
  mCommandOverridden.clear();
  for (int i = 0; i < kC_MAX; ++i) {
    mCommandOverridden.push_back(false);
  }
  mCommandOverrides.clear();
}

void CControlMapper::ResetCommandTimes() {
  for (int i = 0; i < kC_MAX; ++i) {
    mDigitalTime[i] = 0.f;
    mReleaseTime[i] = 0.f;
  }
}

void CControlMapper::ResetInputState() {
  ResetCommandTimes();
  ResetSelector();
}

void CControlMapper::ResetSelector() {
  mSelectorActive = 0;
  mSelectorFadeTime = 0.f;
  mActiveSelectorCommand = kC_None;
}

void CControlMapper::Reset() {
  ResetCommandFilters();
  ResetCommandMappings();
  ResetInputState();
}

void CControlMapper::SwapCommands(ECommands commandA, ECommands commandB, bool swap) {
#if NONMATCHING
  if (commandA < 0 || commandB < 0) {
    return;
  }
#endif
  if (commandA >= kC_MAX || commandB >= kC_MAX || commandA == commandB) {
    return;
  }
  if (swap) {
    if (!IsCommandRemapped(commandA)) {
      SCommandMapping mappingA = GetCommandMapping(commandA);
      SCommandMapping mappingB = GetCommandMapping(commandB);
      SetCommandMapping(commandA, mappingB);
      SetCommandMapping(commandB, mappingA);
    }
  } else {
    RestoreCommandMapping(commandA);
    RestoreCommandMapping(commandB);
  }
}

#else

#include "MetroidPrime/Tweaks/CTweaks.hpp"

#include "MetroidPrime/Tweaks/CTweakPlayerControl.hpp"

#include "Kyoto/Input/CFinalInput.hpp"

rstl::reserved_vector< bool, 67 > ControlMapper::gCommandFilterFlag(true);

const FAnalogInput ControlMapper::gAnalogInputs[] = {
    nullptr,
    &CFinalInput::ALAUp,
    &CFinalInput::ALADown,
    &CFinalInput::ALALeft,
    &CFinalInput::ALARight,
    &CFinalInput::ARAUp,
    &CFinalInput::ARADown,
    &CFinalInput::ARALeft,
    &CFinalInput::ARARight,
    &CFinalInput::ALTrigger,
    &CFinalInput::ARTrigger,
    &CFinalInput::ADPUp,
    &CFinalInput::ADPDown,
    &CFinalInput::ADPLeft,
    &CFinalInput::ADPRight,
    &CFinalInput::AA,
    &CFinalInput::AB,
    &CFinalInput::AX,
    &CFinalInput::AY,
    &CFinalInput::AZ,
    &CFinalInput::AL,
    &CFinalInput::AR,
    &CFinalInput::AStart,
};

const FDigitalInput ControlMapper::gDigitalInputs[] = {
    nullptr,
    &CFinalInput::DLAUp,
    &CFinalInput::DLADown,
    &CFinalInput::DLALeft,
    &CFinalInput::DLARight,
    &CFinalInput::DRAUp,
    &CFinalInput::DRADown,
    &CFinalInput::DRALeft,
    &CFinalInput::DRARight,
    &CFinalInput::DLTrigger,
    &CFinalInput::DRTrigger,
    &CFinalInput::DDPUp,
    &CFinalInput::DDPDown,
    &CFinalInput::DDPLeft,
    &CFinalInput::DDPRight,
    &CFinalInput::DA,
    &CFinalInput::DB,
    &CFinalInput::DX,
    &CFinalInput::DY,
    &CFinalInput::DZ,
    &CFinalInput::DL,
    &CFinalInput::DR,
    &CFinalInput::DStart,
};

const FDigitalInput ControlMapper::gPressInputs[] = {
    nullptr,
    &CFinalInput::PLAUp,
    &CFinalInput::PLADown,
    &CFinalInput::PLALeft,
    &CFinalInput::PLARight,
    &CFinalInput::PRAUp,
    &CFinalInput::PRADown,
    &CFinalInput::PRALeft,
    &CFinalInput::PRARight,
    &CFinalInput::PLTrigger,
    &CFinalInput::PRTrigger,
    &CFinalInput::PDPUp,
    &CFinalInput::PDPDown,
    &CFinalInput::PDPLeft,
    &CFinalInput::PDPRight,
    &CFinalInput::PA,
    &CFinalInput::PB,
    &CFinalInput::PX,
    &CFinalInput::PY,
    &CFinalInput::PZ,
    &CFinalInput::PL,
    &CFinalInput::PR,
    &CFinalInput::PStart,
};

const char* ControlMapper::GetDescriptionForFunction(EFunctionList function) {
  switch (function) {
  case kFL_None:
    return "None";
  case kFL_LeftStickUp:
    return "Left Stick Up";
  case kFL_LeftStickDown:
    return "Left Stick Down";
  case kFL_LeftStickLeft:
    return "Left Stick Left";
  case kFL_LeftStickRight:
    return "Left Stick Right";
  case kFL_RightStickUp:
    return "Right Stick Up";
  case kFL_RightStickDown:
    return "Right Stick Down";
  case kFL_RightStickLeft:
    return "Right Stick Left";
  case kFL_RightStickRight:
    return "Right Stick Right";
  case kFL_LeftTrigger:
    return "Left Trigger";
  case kFL_RightTrigger:
    return "Right Trigger";
  case kFL_DPadUp:
    return "D-Pad Up   ";
  case kFL_DPadDown:
    return "D-Pad Down ";
  case kFL_DPadLeft:
    return "D-Pad Left ";
  case kFL_DPadRight:
    return "D-Pad Right";
  case kFL_AButton:
    return "A Button";
  case kFL_BButton:
    return "B Button";
  case kFL_XButton:
    return "X Button";
  case kFL_YButton:
    return "Y Button";
  case kFL_ZButton:
    return "Z Button";
  case kFL_LeftTriggerPress:
    return "Left Trigger Press";
  case kFL_RightTriggerPress:
    return "Right Trigger Press";
  case kFL_Start:
    return "Start";
  }

  return "UNKNOWN";
}

const char* ControlMapper::GetDescriptionForCommand(ECommands command) {
  switch (command) {
  case kC_Forward:
    return "Forward";
  case kC_Backward:
    return "Backward";
  case kC_TurnLeft:
    return "Turn Left";
  case kC_TurnRight:
    return "Turn Right";
  case kC_StrafeLeft:
    return "Strafe Left";
  case kC_StrafeRight:
    return "Strafe Right";
  case kC_LookLeft:
    return "Look Left";
  case kC_LookRight:
    return "Look Right";
  case kC_LookUp:
    return "Look Up";
  case kC_LookDown:
    return "Look Down";
  case kC_JumpOrBoost:
    return "Jump/Boost";
  case kC_FireOrBomb:
    return "Fire/Bomb";
  case kC_MissileOrPowerBomb:
    return "Missile/PowerBomb";
  case kC_Morph:
    return "Morph";
  case kC_AimUp:
    return "Aim Up";
  case kC_AimDown:
    return "Aim Down";
  case kC_CycleBeamUp:
    return "Cycle Beam Up";
  case kC_CycleBeamDown:
    return "Cycle Beam Down";
  case kC_CycleItem:
    return "Cycle Item";
  case kC_PowerBeam:
    return "Power Beam";
  case kC_IceBeam:
    return "Ice Beam";
  case kC_WaveBeam:
    return "Wave Beam";
  case kC_PlasmaBeam:
    return "Plasma Beam";
  case kC_ToggleHolster:
    return "Toggle Holster";
  case kC_OrbitClose:
    return "Orbit Close";
  case kC_OrbitFar:
    return "Orbit Far";
  case kC_OrbitObject:
    return "Orbit Object";
  case kC_OrbitSelect:
    return "Orbit Select";
  case kC_OrbitConfirm:
    return "Orbit Confirm";
  case kC_OrbitLeft:
    return "Orbit Left";
  case kC_OrbitRight:
    return "Orbit Right";
  case kC_OrbitUp:
    return "Orbit Up";
  case kC_OrbitDown:
    return "Orbit Down";
  case kC_LookHold1:
    return "Look Hold1";
  case kC_LookHold2:
    return "Look Hold2";
  case kC_LookZoomIn:
    return "Look Zoom In";
  case kC_LookZoomOut:
    return "Look Zoom Out";
  case kC_AimHold:
    return "Aim Hold";
  case kC_MapCircleUp:
    return "Map Circle Up";
  case kC_MapCircleDown:
    return "Map Circle Down";
  case kC_MapCircleLeft:
    return "Map Circle Left";
  case kC_MapCircleRight:
    return "Map Circle Right";
  case kC_MapMoveForward:
    return "Map Move Forward";
  case kC_MapMoveBack:
    return "Map Move Back";
  case kC_MapMoveLeft:
    return "Map Move Left";
  case kC_MapMoveRight:
    return "Map Move Right";
  case kC_MapZoomIn:
    return "Map Zoom In";
  case kC_MapZoomOut:
    return "Map Zoom Out";
  case kC_SpiderBall:
    return "SpiderBall";
  case kC_ChaseCamera:
    return "Chase Camera";
  case kC_XrayVisor:
    return "XRay Visor";
  case kC_ThermoVisor:
    return "Thermo Visor";
  case kC_EnviroVisor:
    return "Enviro Visor";
  case kC_NoVisor:
    return "No Visor";
  case kC_VisorMenu:
    return "Visor Menu";
  case kC_VisorUp:
    return "Visor Up";
  case kC_VisorDown:
    return "Visor Down";
  case kC_UseShield:
    return "Use Shield";
  case kC_ScanItem:
    return "Scan Item";
  default:
    break;
  }

  return "UNKNOWN";
}

float ControlMapper::GetAnalogInput(ECommands command, const CFinalInput& input) {
  if (gCommandFilterFlag[command]) {
    if (gAnalogInputs[gpTweakPlayerControlCurrent->GetMapping(command)] != nullptr) {
      return (input.*gAnalogInputs[gpTweakPlayerControlCurrent->GetMapping(command)])();
    }
  }
  return 0.f;
}

bool ControlMapper::GetDigitalInput(ECommands command, const CFinalInput& input) {
  if (gCommandFilterFlag[command]) {
    if (gDigitalInputs[gpTweakPlayerControlCurrent->GetMapping(command)] != nullptr) {
      return (input.*gDigitalInputs[gpTweakPlayerControlCurrent->GetMapping(command)])();
    }
  }
  return false;
}

bool ControlMapper::GetPressInput(ECommands command, const CFinalInput& input) {
  if (gCommandFilterFlag[command]) {
    if (gPressInputs[gpTweakPlayerControlCurrent->GetMapping(command)] != nullptr) {
      return (input.*gPressInputs[gpTweakPlayerControlCurrent->GetMapping(command)])();
    }
  }
  return false;
}

void ControlMapper::ResetCommandFilters() {
  for (int i = 0; i < gCommandFilterFlag.size(); ++i) {
    gCommandFilterFlag[i] = true;
  }
}

void ControlMapper::SetCommandFiltered(ECommands cmd, bool filtered) {
  gCommandFilterFlag[cmd] = filtered;
}

#endif
