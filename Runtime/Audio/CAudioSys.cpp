#include "Runtime/Audio/CAudioSys.hpp"

#include <algorithm>
#include <cmath>
#include <string>
#include <unordered_map>

#include "Runtime/CSimplePool.hpp"
#include "Runtime/Audio/CAudioGroupSet.hpp"

namespace metaforce {
namespace {
std::unordered_map<std::string, TLockedToken<CAudioGroupSet>> mpGroupSetDB;
std::unordered_map<CAssetId, std::string> mpGroupSetResNameDB;
constexpr std::string_view mpDefaultInvalidString = "NULL";

u8 ClampEmitterVol(float vol) {
  if (!std::isfinite(vol))
    return 0;
  return static_cast<u8>(std::lround(std::clamp(vol, 0.f, 1.f) * 127.f));
}
} // Anonymous namespace

CAudioSys* CAudioSys::g_SharedSys = nullptr;
bool CAudioSys::mInitialized = false;
bool CAudioSys::mIsListenerActive = false;
u32 CAudioSys::mUnusedEmitterHandle = 0;
u8 CAudioSys::mMaxNumEmitters = 0;
std::vector<CAudioSys::CEmitterData>* CAudioSys::mpEmitterDB = nullptr;
SND_LISTENER* CAudioSys::mpListener = nullptr;
CAudioSys::ESurroundModes CAudioSys::mSurroundMode = CAudioSys::ESurroundModes::Stereo;
s16 CAudioSys::mVolumeScale = 0x7f;
s16 CAudioSys::mDefaultVolumeScale = 0x7f;

static void* DoMalloc(size_t len) { return new char[len]; }

static void DoFree(void* ptr) {
  if (!ptr) return;
  delete[] static_cast<char*>(ptr);
}

CAudioSys::CAudioSys(u8 numVoices, u8 numMusic, u8 numSfx, u8 maxNumEmitters, u32 aramSize, bool proLogic2) {
  g_SharedSys = this;
  mInitialized = true;

  SND_HOOKS hooks = {DoMalloc, DoFree};
  sndSetHooks(&hooks);
  sndInit(numVoices, numMusic, numSfx, 1, proLogic2 ? SND_FLAGS_DEFAULT_STUDIO_DPL2 : SND_FLAGS_DEFAULT, aramSize);

  mpEmitterDB = new std::vector<CEmitterData>(maxNumEmitters);
  mpListener = new SND_LISTENER{};
  mIsListenerActive = false;
  mMaxNumEmitters = maxNumEmitters;

  sndOutputMode(SND_OUTPUTMODE_SURROUND);
  mSurroundMode = ESurroundModes::Surround;
}

CAudioSys::~CAudioSys() {
  S3dFlushAllEmitters();
  S3dRemoveListener();
  sndQuit();

  delete mpEmitterDB;
  mpEmitterDB = nullptr;
  delete mpListener;
  mpListener = nullptr;
  mInitialized = false;
  g_SharedSys = nullptr;
}

TLockedToken<CAudioGroupSet> CAudioSys::FindGroupSet(std::string_view name) {
  auto search = mpGroupSetDB.find(std::string(name));
  if (search == mpGroupSetDB.cend())
    return {};
  return search->second;
}

std::string_view CAudioSys::SysGetGroupSetName(CAssetId id) {
  auto search = mpGroupSetResNameDB.find(id);
  if (search == mpGroupSetResNameDB.cend())
    return mpDefaultInvalidString;
  return search->second;
}

bool CAudioSys::SysLoadGroupSet(CSimplePool* pool, CAssetId id) {
  if (!FindGroupSet(SysGetGroupSetName(id))) {
    TLockedToken<CAudioGroupSet> set = pool->GetObj(SObjectTag{FOURCC('AGSC'), id});
    mpGroupSetDB.emplace(set->GetName(), set);
    mpGroupSetResNameDB.emplace(id, set->GetName());
    return false;
  } else {
    return true;
  }
}

bool CAudioSys::SysLoadGroupSet(const TLockedToken<CAudioGroupSet>& set, std::string_view name, CAssetId id) {
  if (!FindGroupSet(name)) {
    mpGroupSetDB.emplace(set->GetName(), set);
    mpGroupSetResNameDB.emplace(id, set->GetName());
    return false;
  } else {
    return true;
  }
}

void CAudioSys::SysUnloadAudioGroupSet(std::string_view name) {
  auto set = FindGroupSet(name);
  if (!set)
    return;

  mpGroupSetDB.erase(std::string(name));
  mpGroupSetResNameDB.erase(set.GetObjectTag()->id);
}

bool CAudioSys::SysIsGroupSetLoaded(std::string_view name) { return FindGroupSet(name).operator bool(); }

bool CAudioSys::SysPushGroupIntoARAM(std::string_view name, u8 groupId) {
  auto set = FindGroupSet(name);
  if (!set) {
    std::fprintf(stderr, "[AUDIO] SysPushGroup '%.*s' gid=%d: set not found\n", (int)name.size(), name.data(), groupId);
    return false;
  }
  return sndPushGroup(set->GetProj(), groupId, set->GetSamp(), set->GetSdir(), set->GetPool());
}

void CAudioSys::SysPopGroupFromARAM() {
  sndPopGroup();
}

void CAudioSys::_UpdateVolume() {}

void CAudioSys::SysSetVolume(u8 volume, u16 time, u8 group) {
  sndVolume(volume, time, group);
}

void CAudioSys::SysSetSfxVolume(u8 volume, u16 time, u8 music, u8 fx) {
  sndMasterVolume(volume, time, music, fx);
}

s16 CAudioSys::GetDefaultVolumeScale() { return mDefaultVolumeScale; }

void CAudioSys::SetDefaultVolumeScale(s16 scale) { mDefaultVolumeScale = scale; }

void CAudioSys::SetVolumeScale(s16 scale) { mVolumeScale = scale; }

void CAudioSys::SetSurroundMode(ESurroundModes mode) {
  switch (mode) {
  case ESurroundModes::Mono:
    sndOutputMode(SND_OUTPUTMODE_MONO);
    break;
  case ESurroundModes::Stereo:
    sndOutputMode(SND_OUTPUTMODE_STEREO);
    break;
  case ESurroundModes::Surround:
    sndOutputMode(SND_OUTPUTMODE_SURROUND);
    break;
  }
  mSurroundMode = mode;
}

// SFX API
SND_VOICEID CAudioSys::SfxStart(SND_FXID sfxId, u8 vol, u8 pan, u8 prio) {
  u8 clampedVol = vol > 0x7f ? 0x7f : vol;
  u8 scaledVol = (u8)((mVolumeScale * clampedVol) / 0x7f);
  return sndFXStartEx(sfxId, scaledVol, pan, prio);
}

void CAudioSys::SfxStop(SND_VOICEID handle) { sndFXKeyOff(handle); }

SND_VOICEID CAudioSys::SfxCheck(SND_VOICEID handle) { return sndFXCheck(handle); }

void CAudioSys::SfxVolume(SND_VOICEID handle, u8 vol) { sndFXVolume(handle, vol); }

void CAudioSys::SfxSpan(SND_VOICEID handle, u8 span) { sndFXSurroundPanning(handle, span); }

void CAudioSys::SfxPitchBend(SND_VOICEID handle, u16 pitch) { sndFXPitchBend(handle, pitch); }

void CAudioSys::SfxCtrl(SND_VOICEID handle, u8 ctrl, u8 val) { sndFXCtrl(handle, ctrl, val); }

// 3D Listener
void CAudioSys::S3dAddListener(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                               const zeus::CVector3f& heading, const zeus::CVector3f& up,
                               float frontSur, float backSur, float soundSpeed, u32 flags, u8 vol) {
  if (mIsListenerActive)
    S3dRemoveListener();

  SND_FVECTOR _pos = {pos.x(), pos.y(), pos.z()};
  SND_FVECTOR _dir = {dir.x(), dir.y(), dir.z()};
  SND_FVECTOR _heading = {heading.x(), heading.y(), heading.z()};
  SND_FVECTOR _up = {up.x(), up.y(), up.z()};

  mIsListenerActive = true;
  sndAddListener(mpListener, &_pos, &_dir, &_heading, &_up, frontSur, backSur, soundSpeed, flags, vol, nullptr);
}

bool CAudioSys::S3dUpdateListener(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                  const zeus::CVector3f& heading, const zeus::CVector3f& up, u8 vol) {
  if (!mIsListenerActive) return false;

  SND_FVECTOR _pos = {pos.x(), pos.y(), pos.z()};
  SND_FVECTOR _dir = {dir.x(), dir.y(), dir.z()};
  SND_FVECTOR _heading = {heading.x(), heading.y(), heading.z()};
  SND_FVECTOR _up = {up.x(), up.y(), up.z()};

  return sndUpdateListener(mpListener, &_pos, &_dir, &_heading, &_up, vol, nullptr);
}

bool CAudioSys::S3dRemoveListener() {
  if (!mIsListenerActive) return false;
  mIsListenerActive = false;
  return sndRemoveListener(mpListener);
}

// 3D Emitters
u32 CAudioSys::S3dFindUnusedHandle() {
  for (int i = 0; i < mMaxNumEmitters; ++i) {
    if (!(*mpEmitterDB)[i].x50_inUse)
      return i;
  }
  return static_cast<u32>(-1);
}

u32 CAudioSys::S3dFindLowerPriorityHandle(u32 prio) {
  for (int i = 0; i < mMaxNumEmitters; ++i) {
    CEmitterData& data = (*mpEmitterDB)[i];
    if (!data.x50_inUse)
      return i;
    if (data.x52_prio <= prio && !data.x51_important) {
      S3dRemoveEmitter(i);
      return i;
    }
  }
  return static_cast<u32>(-1);
}

u32 CAudioSys::S3dAddEmitterParaEx(const C3DEmitterParmData& params, u16 groupId, SND_PARAMETER_INFO* paraInfo) {
  u32 handle = mUnusedEmitterHandle;
  if (handle == static_cast<u32>(-1)) {
    handle = S3dFindLowerPriorityHandle(params.x29_prio);
    if (handle == static_cast<u32>(-1))
      return static_cast<u32>(-1);
  }

  CEmitterData& data = (*mpEmitterDB)[handle];

  SND_FVECTOR _pos = {params.x0_pos.x(), params.x0_pos.y(), params.x0_pos.z()};
  SND_FVECTOR _dir = {params.xc_dir.x(), params.xc_dir.y(), params.xc_dir.z()};

  u8 clampedMax = ClampEmitterVol(params.x26_maxVol);
  u8 scaledMaxVol = (u8)((mVolumeScale * clampedMax) / 0x7f);
  u8 clampedMin = ClampEmitterVol(params.x27_minVol);
  u8 scaledMinVol = (u8)((mVolumeScale * clampedMin) / 0x7f);

  sndAddEmitterParaEx(&data.x0_emitter, &_pos, &_dir, params.x18_maxDist, params.x1c_distComp,
                      params.x20_flags, params.x24_sfxId, groupId, scaledMaxVol, scaledMinVol,
                      nullptr, paraInfo);
  data.x50_inUse = true;
  data.x51_important = params.x28_important;
  data.x52_prio = params.x29_prio;
  mUnusedEmitterHandle = S3dFindUnusedHandle();
  return handle;
}

bool CAudioSys::S3dUpdateEmitter(u32 handle, const zeus::CVector3f& pos,
                                 const zeus::CVector3f& dir, float maxVol) {
  if (handle == static_cast<u32>(-1)) return false;

  CEmitterData& data = (*mpEmitterDB)[handle];
  SND_FVECTOR _pos = {pos.x(), pos.y(), pos.z()};
  SND_FVECTOR _dir = {dir.x(), dir.y(), dir.z()};
  return sndUpdateEmitter(&data.x0_emitter, &_pos, &_dir, ClampEmitterVol(maxVol), nullptr);
}

bool CAudioSys::S3dRemoveEmitter(u32 handle) {
  if (handle == static_cast<u32>(-1)) return false;

  CEmitterData& data = (*mpEmitterDB)[handle];
  if (data.x50_inUse) {
    data.x50_inUse = false;
    mUnusedEmitterHandle = handle;
    return sndRemoveEmitter(&data.x0_emitter);
  }
  return true;
}

bool CAudioSys::S3dCheckEmitter(u32 handle) {
  if (handle == static_cast<u32>(-1)) return false;
  CEmitterData& data = (*mpEmitterDB)[handle];
  if (data.x50_inUse)
    return sndCheckEmitter(&data.x0_emitter);
  return false;
}

SND_VOICEID CAudioSys::S3dEmitterVoiceID(u32 handle) {
  if (handle == static_cast<u32>(-1)) return 0;
  CEmitterData& data = (*mpEmitterDB)[handle];
  if (data.x50_inUse)
    return sndEmitterVoiceID(&data.x0_emitter);
  return SND_ID_ERROR;
}

void CAudioSys::S3dFlushAllEmitters() {
  if (!mpEmitterDB) return;
  for (auto& data : *mpEmitterDB) {
    if (!data.x50_inUse) continue;
    data.x50_inUse = false;
    sndRemoveEmitter(&data.x0_emitter);
  }
  mUnusedEmitterHandle = 0;
}

void CAudioSys::S3dFlushUnusedEmitters() {
  if (!mpEmitterDB) return;
  for (auto& data : *mpEmitterDB) {
    if (!data.x50_inUse || sndCheckEmitter(&data.x0_emitter) || data.x51_important)
      continue;
    data.x50_inUse = false;
    sndRemoveEmitter(&data.x0_emitter);
  }
}

// Sequence API
u32 CAudioSys::SeqPlayEx(u16 gid, u16 sid, void* arrfile, SND_PLAYPARA* para, u8 studio) {
  return sndSeqPlayEx(gid, sid, arrfile, para, studio);
}

void CAudioSys::SeqStop(u32 seqId) { sndSeqStop(seqId); }

void CAudioSys::SeqVolume(u8 volume, u16 time, u32 seqId, u8 mode) {
  sndSeqVolume(volume, time, seqId, mode);
}

} // namespace metaforce
