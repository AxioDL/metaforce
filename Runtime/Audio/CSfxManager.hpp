#pragma once

#include <array>
#include <memory>
#include <unordered_set>
#include <vector>

#include "DNAMP1/SFX/SFX.h"
#include "Runtime/RetroTypes.hpp"
#include "Runtime/Audio/CAudioSys.hpp"

#include <zeus/CVector3f.hpp>

namespace urde {

class CSfxManager {
  static std::vector<u16>* mpSfxTranslationTable;

public:
  enum class ESfxChannels { Invalid = -1, Default = 0, Game, PauseScreen };

  enum class ESfxAudibility { Aud0, Aud1, Aud2, Aud3 };

  enum class EAuxEffect { None = -1, ReverbHi = 0, Chorus, ReverbStd, Delay };

  class CBaseSfxWrapper;
  using CSfxHandle = std::shared_ptr<CBaseSfxWrapper>;

  /* Original imp, kept for reference
  class CSfxHandle
  {
      static u32 mRefCount;
      u32 x0_idx;
  public:
      CSfxHandle(u32 id)
          : x0_idx(++mRefCount << 14 | (id & 0xFFFF)) {}
  };
  */

  class CSfxChannel {
    friend class CSfxManager;
    zeus::CVector3f x0_pos;
    zeus::CVector3f xc_;
    zeus::CVector3f x18_;
    zeus::CVector3f x24_;
    /*
    float x30_ = 0.f;
    float x34_ = 0.f;
    float x38_ = 0.f;
    u32 x3c_ = 0;
    bool x40_ = false;
    */
    bool x44_listenerActive = false;
    std::unordered_set<CSfxHandle> x48_handles;
  };

  class CBaseSfxWrapper : public std::enable_shared_from_this<CBaseSfxWrapper> {
    float x4_timeRemaining = 15.f;
    s16 x8_rank = 0;
    s16 xa_prio;
    // CSfxHandle xc_handle;
    TAreaId x10_area;
    bool x14_24_isActive : 1 = true;
    bool x14_25_isPlaying : 1 = false;
    bool x14_26_looped : 1;
    bool x14_27_inArea : 1 = true;
    bool x14_28_isReleased : 1 = false;
    bool x14_29_useAcoustics : 1;

  protected:
    bool m_isEmitter : 1 = false;
    bool m_isClosed : 1 = false;

  public:
    virtual ~CBaseSfxWrapper() = default;
    virtual void SetActive(bool v) { x14_24_isActive = v; }
    virtual void SetPlaying(bool v) { x14_25_isPlaying = v; }
    virtual void SetRank(short v) { x8_rank = v; }
    virtual void SetInArea(bool v) { x14_27_inArea = v; }
    virtual bool IsInArea() const { return x14_27_inArea; }
    virtual bool IsPlaying() const { return x14_25_isPlaying; }
    virtual bool UseAcoustics() const { return x14_29_useAcoustics; }
    virtual bool IsLooped() const { return x14_26_looped; }
    virtual bool IsActive() const { return x14_24_isActive; }
    virtual s16 GetRank() const { return x8_rank; }
    virtual s16 GetPriority() const { return xa_prio; }
    virtual TAreaId GetArea() const { return x10_area; }
    virtual CSfxHandle GetSfxHandle() { return shared_from_this(); }
    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual bool Ready() = 0;
    virtual ESfxAudibility GetAudible(const zeus::CVector3f&) = 0;
    virtual amuse::ObjToken<amuse::Voice> GetVoice() const = 0;
    virtual u16 GetSfxId() const = 0;
    virtual void UpdateEmitterSilent() = 0;
    virtual void UpdateEmitter() = 0;
    virtual void SetReverb(float rev) = 0;
    bool IsEmitter() const { return m_isEmitter; }

    void Release() {
      x14_28_isReleased = true;
      x4_timeRemaining = 15.f;
    }
    bool IsReleased() const { return x14_28_isReleased; }

    void Close() { m_isClosed = true; }
    bool IsClosed() const { return m_isClosed; }

    float GetTimeRemaining() const { return x4_timeRemaining; }
    void SetTimeRemaining(float t) { x4_timeRemaining = t; }

    CBaseSfxWrapper(bool looped, s16 prio, /*const CSfxHandle& handle,*/ bool useAcoustics, TAreaId area)
    : xa_prio(prio)
    , /*xc_handle(handle),*/ x10_area(area)
    , x14_26_looped(looped)
    , x14_29_useAcoustics(useAcoustics) {}
  };

  class CSfxEmitterWrapper : public CBaseSfxWrapper {
    float x1a_reverb = 0.0f;
    CAudioSys::C3DEmitterParmData x24_parmData;
    amuse::ObjToken<amuse::Emitter> x50_emitterHandle;
    bool x54_ready = true;
    float x55_cachedMaxVol = 0.0f;

  public:
    bool IsPlaying() const override;
    void Play() override;
    void Stop() override;
    bool Ready() override;
    ESfxAudibility GetAudible(const zeus::CVector3f&) override;
    amuse::ObjToken<amuse::Voice> GetVoice() const override { return x50_emitterHandle->getVoice(); }
    u16 GetSfxId() const override;
    void UpdateEmitterSilent() override;
    void UpdateEmitter() override;
    void SetReverb(float rev) override;
    CAudioSys::C3DEmitterParmData& GetEmitterData() { return x24_parmData; }

    amuse::ObjToken<amuse::Emitter> GetHandle() const { return x50_emitterHandle; }

    CSfxEmitterWrapper(bool looped, s16 prio, const CAudioSys::C3DEmitterParmData& data,
                       /*const CSfxHandle& handle,*/ bool useAcoustics, TAreaId area)
    : CBaseSfxWrapper(looped, prio, /*handle,*/ useAcoustics, area), x24_parmData(data) {
      m_isEmitter = true;
    }
  };

  class CSfxWrapper : public CBaseSfxWrapper {
    u16 x18_sfxId;
    amuse::ObjToken<amuse::Voice> x1c_voiceHandle;
    float x20_vol;
    float x22_pan;
    bool x24_ready = true;

  public:
    bool IsPlaying() const override;
    void Play() override;
    void Stop() override;
    bool Ready() override;
    ESfxAudibility GetAudible(const zeus::CVector3f&) override { return ESfxAudibility::Aud3; }
    amuse::ObjToken<amuse::Voice> GetVoice() const override { return x1c_voiceHandle; }
    u16 GetSfxId() const override;
    void UpdateEmitterSilent() override;
    void UpdateEmitter() override;
    void SetReverb(float rev) override;
    void SetVolume(float vol) { x20_vol = vol; }

    CSfxWrapper(bool looped, s16 prio, u16 sfxId, float vol, float pan,
                /*const CSfxHandle& handle,*/ bool useAcoustics, TAreaId area)
    : CBaseSfxWrapper(looped, prio, /*handle,*/ useAcoustics, area), x18_sfxId(sfxId), x20_vol(vol), x22_pan(pan) {
      m_isEmitter = false;
    }
  };

  static std::array<CSfxChannel, 4> m_channels;
  static ESfxChannels m_currentChannel;
  static bool m_doUpdate;
  static void* m_usedSounds;
  static bool m_muted;
  static bool m_auxProcessingEnabled;
  static float m_reverbAmount;
  static EAuxEffect m_activeEffect;
  static EAuxEffect m_nextEffect;
  static amuse::ObjToken<amuse::Listener> m_listener;

  static u16 kMaxPriority;
  static u16 kMedPriority;
  static u16 kInternalInvalidSfxId;
  static u32 kAllAreas;

  static bool LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag);
  static bool IsAuxProcessingEnabled() { return m_auxProcessingEnabled; }
  static void SetChannel(ESfxChannels);
  static void KillAll(ESfxChannels);
  static void TurnOnChannel(ESfxChannels);
  static void TurnOffChannel(ESfxChannels);
  static ESfxChannels GetCurrentChannel() { return m_currentChannel; }
  static void AddListener(ESfxChannels channel, const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                          const zeus::CVector3f& heading, const zeus::CVector3f& up, float frontRadius,
                          float surroundRadius, float soundSpeed, u32 flags /* 0x1 for doppler */, float vol);
  static void UpdateListener(const zeus::CVector3f& pos, const zeus::CVector3f& dir, const zeus::CVector3f& heading,
                             const zeus::CVector3f& up, float vol);

  static bool PlaySound(const CSfxHandle& handle);
  static void StopSound(const CSfxHandle& handle);
  static s16 GetRank(CBaseSfxWrapper* sfx);
  static void ApplyReverb();
  static float GetReverbAmount();
  static void PitchBend(const CSfxHandle& handle, float pitch);
  static void SfxVolume(const CSfxHandle& handle, float vol);
  static void SfxSpan(const CSfxHandle& handle, float span);
  static u16 TranslateSFXID(u16);
  static void SfxStop(const CSfxHandle& handle);
  static CSfxHandle SfxStart(u16 id, float vol, float pan, bool useAcoustics, s16 prio, bool looped, s32 areaId);
  static bool IsPlaying(const CSfxHandle& handle);
  static void RemoveEmitter(const CSfxHandle& handle);
  static void UpdateEmitter(const CSfxHandle& handle, const zeus::CVector3f& pos, const zeus::CVector3f& dir,
                            float maxVol);
  static CSfxHandle AddEmitter(u16 id, const zeus::CVector3f& pos, const zeus::CVector3f& dir, bool useAcoustics,
                               bool looped, s16 prio, s32 areaId);
  static CSfxHandle AddEmitter(u16 id, const zeus::CVector3f& pos, const zeus::CVector3f& dir, float vol,
                               bool useAcoustics, bool looped, s16 prio, s32 areaId);
  static CSfxHandle AddEmitter(const CAudioSys::C3DEmitterParmData& parmData, bool useAcoustics, s16 prio, bool looped,
                               s32 areaId);
  static void StopAndRemoveAllEmitters();
  static void DisableAuxCallback();
  static void EnableAuxCallback();
  static void PrepareDelayCallback(const amuse::EffectDelayInfo& info);
  static void PrepareReverbStdCallback(const amuse::EffectReverbStdInfo& info);
  static void PrepareChorusCallback(const amuse::EffectChorusInfo& info);
  static void PrepareReverbHiCallback(const amuse::EffectReverbHiInfo& info);
  static void DisableAuxProcessing();

  static void SetActiveAreas(const rstl::reserved_vector<TAreaId, 10>& areas);

  static void Update(float dt);
  static void Shutdown();
};

using CSfxHandle = CSfxManager::CSfxHandle;

} // namespace urde
