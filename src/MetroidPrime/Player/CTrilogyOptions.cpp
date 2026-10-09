#include "MetroidPrime/Player/CTrilogyOptions.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Kyoto/Streams/COutputStream.hpp"

CTrilogyOptions::EControlPreset CTrilogyOptions::kDefaultControlPreset = kCP_Standard;

CTrilogyOptions::CTrilogyOptions()
: mScreenBrightness(50)
, mScreenPositionX(0)
, mScreenPositionY(0)
, mSfxVolume(100)
, mMusicVolume(100)
, mVoiceVolume(100)
, mHudAlpha(255)
, mHelmetAlpha(255)
, mHudLag(true)
, mControllerRumble(true)
, mRedundantHintSystem(true)
, mHudEnglish(false)
, mFireAndJumpSwapped(false)
, mSwitchVisorBeamControls(false)
, mLockOnFreeAim(true)
, mReservedFlag(false)
, mNotifyAchievementEarned(true)
, mControlPreset(kDefaultControlPreset) {}

CTrilogyOptions::CTrilogyOptions(CInputStream& in)
: mScreenBrightness(50)
, mScreenPositionX(0)
, mScreenPositionY(0)
, mSfxVolume(100)
, mMusicVolume(100)
, mVoiceVolume(100)
, mHudAlpha(255)
, mHelmetAlpha(255)
, mHudLag(true)
, mControllerRumble(true)
, mRedundantHintSystem(true)
, mHudEnglish(false)
, mFireAndJumpSwapped(false)
, mSwitchVisorBeamControls(false)
, mLockOnFreeAim(true)
, mReservedFlag(false)
, mNotifyAchievementEarned(true)
, mControlPreset(kDefaultControlPreset) {
  in.ReadLong();
  mScreenBrightness = in.ReadLong();
  mScreenPositionX = in.ReadLong();
  mScreenPositionY = in.ReadLong();
  mSfxVolume = in.ReadLong();
  mMusicVolume = in.ReadLong();
  mVoiceVolume = in.ReadLong();
  mHudAlpha = in.ReadLong();
  mHelmetAlpha = in.ReadLong();
  mControlPreset = static_cast< EControlPreset >(in.ReadLong());
  mHudLag = in.ReadBool();
  mRedundantHintSystem = in.ReadBool();
  mControllerRumble = in.ReadBool();
  mHudEnglish = in.ReadBool();
  mFireAndJumpSwapped = in.ReadBool();
  mSwitchVisorBeamControls = in.ReadBool();
  mLockOnFreeAim = in.ReadBool();
  mReservedFlag = in.ReadBool();
  mNotifyAchievementEarned = in.ReadBool();
}

void CTrilogyOptions::PutTo(COutputStream& out) const {
  out.WriteLong('OPTN');
  out.WriteLong(mScreenBrightness);
  out.WriteLong(mScreenPositionX);
  out.WriteLong(mScreenPositionY);
  out.WriteLong(mSfxVolume);
  out.WriteLong(mMusicVolume);
  out.WriteLong(mVoiceVolume);
  out.WriteLong(mHudAlpha);
  out.WriteLong(mHelmetAlpha);
  out.WriteLong(mControlPreset);
  out.WriteBool(mHudLag);
  out.WriteBool(mRedundantHintSystem);
  out.WriteBool(mControllerRumble);
  out.WriteBool(mHudEnglish);
  out.WriteBool(mFireAndJumpSwapped);
  out.WriteBool(mSwitchVisorBeamControls);
  out.WriteBool(mLockOnFreeAim);
  out.WriteBool(mReservedFlag);
  out.WriteBool(mNotifyAchievementEarned);
}

void CTrilogyOptions::ResetControlDefaults() {
  mControllerRumble = true;
  mLockOnFreeAim = true;
  mReservedFlag = false;
  mFireAndJumpSwapped = false;
  mSwitchVisorBeamControls = false;
  mControlPreset = kDefaultControlPreset;
}

void CTrilogyOptions::ResetDisplayDefaults() {
  mScreenBrightness = 50;
  mScreenPositionX = 0;
  mScreenPositionY = 0;
  mRedundantHintSystem = true;
  mNotifyAchievementEarned = true;
}

void CTrilogyOptions::ResetSoundDefaults() {
  mSfxVolume = 100;
  mMusicVolume = 100;
  mVoiceVolume = 100;
}

void CTrilogyOptions::ResetVisorDefaults() {
  mHudAlpha = 255;
  mHelmetAlpha = 255;
  mHudLag = true;
  mHudEnglish = false;
}

void CTrilogyOptions::SetScreenBrightness(int value) {
  mScreenBrightness = value < 0 ? 0 : value > 100 ? 100 : value;
}

void CTrilogyOptions::SetScreenPositionX(int value) {
  mScreenPositionX = value < -30 ? -30 : value > 30 ? 30 : value;
}

void CTrilogyOptions::SetScreenPositionY(int value) {
  mScreenPositionY = value < -19 ? -19 : value > 19 ? 19 : value;
}

void CTrilogyOptions::SetSfxVolume(int value) {
  mSfxVolume = value < 0 ? 0 : value > 100 ? 100 : value;
}

void CTrilogyOptions::SetMusicVolume(int value) {
  mMusicVolume = value < 0 ? 0 : value > 100 ? 100 : value;
}

void CTrilogyOptions::SetHudAlpha(int value) {
  mHudAlpha = value < 0 ? 0 : value > 255 ? 255 : value;
}

void CTrilogyOptions::SetHelmetAlpha(int value) {
  mHelmetAlpha = value < 0 ? 0 : value > 255 ? 255 : value;
}
