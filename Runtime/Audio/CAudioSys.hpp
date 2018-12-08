#pragma once

#include "../GCNTypes.hpp"
#include "zeus/CVector3f.hpp"
#include "amuse/amuse.hpp"
#include "boo/audiodev/IAudioVoiceEngine.hpp"
#include "RetroTypes.hpp"
#include "CToken.hpp"

namespace urde {
class CSimplePool;
class CAudioGroupSet;

CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag& tag, CInputStream& in, const CVParamTransfer& vparms,
                                               CObjectReference* selfRef);

class CAudioSys {
public:
  enum class ESurroundModes { Mono, Stereo, Surround };

private:
  static CAudioSys* g_SharedSys;
  boo::IAudioVoiceEngine* m_voiceEngine;
  amuse::Engine m_engine;
  static void _UpdateVolume();

public:
  struct C3DEmitterParmData {
    zeus::CVector3f x0_pos;
    zeus::CVector3f xc_dir;
    float x18_maxDist;
    float x1c_distComp;
    u32 x20_flags;
    u16 x24_sfxId;
    float x26_maxVol;
    float x27_minVol;
    bool x28_important; // Can't be allocated over, regardless of priority
    u8 x29_prio;
  };
  CAudioSys(boo::IAudioVoiceEngine* voiceEngine, amuse::IBackendVoiceAllocator& backend, u8, u8, u8, u8, u32)
  : m_voiceEngine(voiceEngine), m_engine(backend) {
    g_SharedSys = this;
  }
  ~CAudioSys() { g_SharedSys = nullptr; }

  static void AddAudioGroup(const amuse::AudioGroupData& data) { g_SharedSys->m_engine.addAudioGroup(data); }
  static void RemoveAudioGroup(const amuse::AudioGroupData& data) { g_SharedSys->m_engine.removeAudioGroup(data); }
  static boo::IAudioVoiceEngine* GetVoiceEngine() { return g_SharedSys->m_voiceEngine; }
  static amuse::Engine& GetAmuseEngine() { return g_SharedSys->m_engine; }
  static void SetSurroundMode(ESurroundModes mode) {}
  static TLockedToken<CAudioGroupSet> FindGroupSet(std::string_view name);
  static std::string_view SysGetGroupSetName(CAssetId id);
  static bool SysLoadGroupSet(CSimplePool* pool, CAssetId id);
  static bool SysLoadGroupSet(const TLockedToken<CAudioGroupSet>& set, std::string_view name, CAssetId id);
  static void SysUnloadAudioGroupSet(std::string_view name);
  static bool SysIsGroupSetLoaded(std::string_view name);
  static void SysAddGroupIntoAmuse(std::string_view name);
  static void SysRemoveGroupFromAmuse(std::string_view name);
  static void SysSetVolume(u8 volume);
  static void SysSetSfxVolume(u8 volume, u16 time, bool music, bool fx);

  static s16 GetDefaultVolumeScale();
  static void SetDefaultVolumeScale(s16 scale);
  static void SetVolumeScale(s16 scale);
};

} // namespace urde
