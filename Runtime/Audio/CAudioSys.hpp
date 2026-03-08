#pragma once

#include <vector>

#include "Runtime/CToken.hpp"
#include "Runtime/GCNTypes.hpp"
#include "Runtime/RetroTypes.hpp"

#include <musyx/musyx.h>
#include <zeus/CVector3f.hpp>

namespace metaforce {
class CAudioGroupSet;
class CSimplePool;

CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                               CObjectReference* selfRef);

class CAudioSys {
public:
  enum class ESurroundModes { Mono, Stereo, Surround };

  struct CEmitterData {
    SND_EMITTER x0_emitter{};
    bool x50_inUse = false;
    bool x51_important = false;
    u8 x52_prio = 0x7f;
  };

  struct C3DEmitterParmData {
    zeus::CVector3f x0_pos;
    zeus::CVector3f xc_dir;
    float x18_maxDist = 150.f;
    float x1c_distComp = 0.1f;
    u32 x20_flags = 1;
    u16 x24_sfxId = 0;
    float x26_maxVol = 1.f;
    float x27_minVol = 0.f;
    bool x28_important = false;
    u8 x29_prio = 0x7f;
  };

private:
  static CAudioSys* g_SharedSys;
  static bool mInitialized;
  static bool mIsListenerActive;
  static u32 mUnusedEmitterHandle;
  static u8 mMaxNumEmitters;
  static std::vector<CEmitterData>* mpEmitterDB;
  static SND_LISTENER* mpListener;
  static ESurroundModes mSurroundMode;
  static s16 mVolumeScale;
  static s16 mDefaultVolumeScale;

  static void _UpdateVolume();

public:
  static constexpr u8 kMaxVolume = 0x7f;
  static constexpr u8 kEmitterMedPriority = 0x7f;

  CAudioSys(u8 numVoices, u8 numMusic, u8 numSfx, u8 maxNumEmitters, u32 aramSize, bool proLogic2);
  ~CAudioSys();

  static void SetSurroundMode(ESurroundModes mode);
  static TLockedToken<CAudioGroupSet> FindGroupSet(std::string_view name);
  static std::string_view SysGetGroupSetName(CAssetId id);
  static bool SysLoadGroupSet(CSimplePool* pool, CAssetId id);
  static bool SysLoadGroupSet(const TLockedToken<CAudioGroupSet>& set, std::string_view name, CAssetId id);
  static void SysUnloadAudioGroupSet(std::string_view name);
  static bool SysIsGroupSetLoaded(std::string_view name);
  static bool SysPushGroupIntoARAM(std::string_view name, u8 groupId);
  static void SysPopGroupFromARAM();
  static void SysSetVolume(u8 volume, u16 time = 0, u8 group = SND_ALL_VOLGROUPS);
  static void SysSetSfxVolume(u8 volume, u16 time, u8 music, u8 fx);

  static s16 GetDefaultVolumeScale();
  static void SetDefaultVolumeScale(s16 scale);
  static void SetVolumeScale(s16 scale);

  // SFX API
  static SND_VOICEID SfxStart(SND_FXID sfxId, u8 vol, u8 pan, u8 prio);
  static void SfxStop(SND_VOICEID handle);
  static SND_VOICEID SfxCheck(SND_VOICEID handle);
  static void SfxVolume(SND_VOICEID handle, u8 vol);
  static void SfxSpan(SND_VOICEID handle, u8 span);
  static void SfxPitchBend(SND_VOICEID handle, u16 pitch);
  static void SfxCtrl(SND_VOICEID handle, u8 ctrl, u8 val);

  // 3D API
  static void S3dAddListener(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                             const zeus::CVector3f& heading, const zeus::CVector3f& up,
                             float frontSur, float backSur, float soundSpeed, u32 flags, u8 vol);
  static bool S3dUpdateListener(const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                                const zeus::CVector3f& heading, const zeus::CVector3f& up, u8 vol);
  static bool S3dRemoveListener();
  static u32 S3dAddEmitterParaEx(const C3DEmitterParmData& params, u16 groupId, SND_PARAMETER_INFO* paraInfo);
  static bool S3dUpdateEmitter(u32 handle, const zeus::CVector3f& pos, const zeus::CVector3f& dir, float maxVol);
  static bool S3dRemoveEmitter(u32 handle);
  static bool S3dCheckEmitter(u32 handle);
  static SND_VOICEID S3dEmitterVoiceID(u32 handle);
  static void S3dFlushAllEmitters();
  static void S3dFlushUnusedEmitters();

  // Sequence API
  static u32 SeqPlayEx(u16 gid, u16 sid, void* arrfile, SND_PLAYPARA* para, u8 studio);
  static void SeqStop(u32 seqId);
  static void SeqVolume(u8 volume, u16 time, u32 seqId, u8 mode);

private:
  static u32 S3dFindUnusedHandle();
  static u32 S3dFindLowerPriorityHandle(u32 prio);
};

} // namespace metaforce
