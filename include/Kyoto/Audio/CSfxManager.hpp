#ifndef _CSFXMANAGER
#define _CSFXMANAGER

#include "rstl/optional_object.hpp"
#include "types.h"

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CSfxHandle.hpp"

#include <rstl/auto_ptr.hpp>
#include <rstl/reserved_vector.hpp>
#include <rstl/single_ptr.hpp>

#include <musyx/musyx.h>

#include <Kyoto/CFactoryFnReturn.hpp>

class CVector3f;
class CSfxManager {
public:
  enum ESfxChannels {
    kSC_Invalid = -1,
    kSC_Default = 0,
    kSC_Game,
    kSC_PauseScreen,
  };

  enum ESfxAudibility {
    kSA_Aud0,
    kSA_Aud1,
    kSA_Aud2,
    kSA_Aud3,
  };

  enum EAuxEffect {
    kAE_None = -1,
    kAE_ReverbHi = 0,
    kAE_Chorus,
    kAE_ReverbStd,
    kAE_Delay,
  };

  class CBaseSfxWrapper {
  public:
    CBaseSfxWrapper(const bool looped, const short prio, const CSfxHandle handle,
                    const bool useAcoustics, const int area);
    virtual ~CBaseSfxWrapper() = 0;
    virtual void SetActive(bool v);
    virtual void SetPlaying(bool v);
    virtual void SetRank(const short v);
    virtual void SetInArea(bool v);
    virtual const bool IsLooped() const;
    virtual bool IsPlaying() const;
    virtual const bool IsActive() const;
    virtual const bool IsInArea() const;
    virtual const bool UseAcoustics() const;
    virtual const int GetRank() const;
    virtual const int GetPriority() const;
    virtual const int GetArea() const;
    virtual const CSfxHandle GetSfxHandle() const;
    virtual void Play() = 0;
    virtual void Stop() = 0;
    virtual const bool Ready() = 0;
    virtual short GetAudible(const CVector3f&) = 0;
    virtual const SND_VOICEID GetVoice() const = 0;
    virtual const SND_FXID GetSfxId() = 0;
    virtual void UpdateEmitterSilent() = 0;
    virtual void UpdateEmitter() = 0;
    virtual void SetReverb(const char rev) = 0;
    const bool Available() const;
    void Release();
    const float GetTimeRemaining();
    void SetTimeRemaining(float t);

  private:
    float mTimeRemaining;
    short mRank;
    short mPrio;
    CSfxHandle mHandle;
    int mArea;
    bool mIsActive : 1;
    bool mIsPlaying : 1;
    bool mLooped : 1;
    bool mInArea : 1;
    bool mIsReleased : 1;
    bool mUseAcoustics : 1;
  };

  class CSfxEmitterWrapper : public CBaseSfxWrapper {
  public:
    CSfxEmitterWrapper(const bool looped, short prio, CAudioSys::C3DEmitterParmData& emitterData,
                       const CSfxHandle handle, const bool useAcoustics, const int area);
    ~CSfxEmitterWrapper() {}
    bool IsPlaying() const override;
    void Play() override;
    void Stop() override;
    const bool Ready() override;
    short GetAudible(const CVector3f&) override;
    const SND_VOICEID GetVoice() const override;
    const SND_FXID GetSfxId() override;
    void UpdateEmitterSilent() override;
    void UpdateEmitter() override;
    void SetReverb(const char rev) override;
    CAudioSys::C3DEmitterParmData& GetEmitter();
    const SND_VOICEID GetHandle() const;

  private:
    SND_PARAMETER mPara;
    SND_PARAMETER_INFO mParameterInfo;
    CAudioSys::C3DEmitterParmData mEmitterData;
    SND_VOICEID mEmitterHandle;
    bool mReady;
    char mCachedMaxVol;
  };

  class CSfxWrapper : public CBaseSfxWrapper {
  public:
    CSfxWrapper(const bool looped, const short prio, const ushort sfxId, const short vol,
                const short pan, const CSfxHandle handle, const bool useAcoustics, const int area);
    ~CSfxWrapper() {}

    bool IsPlaying() const override;
    void Play() override;
    void Stop() override;
    const bool Ready() override;
    short GetAudible(const CVector3f&) override;
    const SND_VOICEID GetVoice() const override;
    const SND_FXID GetSfxId() override;
    void UpdateEmitterSilent() override;
    void UpdateEmitter() override;
    void SetReverb(const char rev) override;
    void SetVolume(short vol);

  private:
    SND_FXID mSfxId;
    SND_VOICEID mVoiceHandle;
    short mVol;
    short mPan;
    bool mReady;
  };
  class CSfxListener {
  public:
    explicit CSfxListener(const CVector3f pos = CVector3f::Zero(),
                          const CVector3f dir = CVector3f::Zero(),
                          const CVector3f vec1 = CVector3f::Zero(),
                          const CVector3f vec2 = CVector3f::Zero(), const float f1 = 0.f,
                          const float f2 = 0.f, const float f3 = 0.f, const uint w1 = 0,
#if VERSION >= VERSION_GM8E_02
                          uchar maxVolume = 0)
#else
                          const uchar maxVolume = 0)
#endif
    : x0_(pos)
    , xc_(dir)
    , x18_(vec1)
    , x24_(vec2)
    , x30_(f1)
    , x34_(f2)
    , x38_(f3)
    , x3c_(w1)
    , x40_(maxVolume) {}

    CVector3f x0_;
    CVector3f xc_;
    CVector3f x18_;
    CVector3f x24_;
    float x30_;
    float x34_;
    float x38_;
    int x3c_;
    char x40_;
  };

  class CSfxChannel {
  public:
    CSfxChannel() : x44_(false) {}
    CSfxListener mListener;
    bool x44_;
    rstl::reserved_vector< CBaseSfxWrapper*, 72 > x48_;
  };

  static void Update(float dt);
  static void RemoveEmitter(CSfxHandle handle);
  static void UpdateEmitter(CSfxHandle handle, const CVector3f& pos, const CVector3f& dir,
                            const uchar maxVol);
  static void UpdateListener(const CVector3f& pos, const CVector3f& dir, const CVector3f&,
                             const CVector3f&, const uchar);

  static const short kMaxPriority;           // 0xFF
  static const short kMedPriority;           // 0x7F
  static const ushort kInternalInvalidSfxId; // 0xFFFF
  static const int kAllAreas;                // 0xFFFFFFFF
  static CSfxHandle AddEmitter(const SND_FXID id, const CVector3f& pos, const CVector3f& dir,
                               const bool useAcoustics = false, const bool looped = false,
                               const short prio = kMedPriority, const int areaId = kAllAreas);
  static CSfxHandle AddEmitter(const SND_FXID id, const CVector3f& pos, const CVector3f& dir,
                               const uchar vol, const bool useAcoustics = false,
                               const bool looped = false, const short prio = kMedPriority,
                               const int areaId = kAllAreas);
  static CSfxHandle AddEmitter(CAudioSys::C3DEmitterParmData& parmData,
                               bool useAcoustics = false, const short prio = kMedPriority,
                               const bool looped = false, const int areaId = kAllAreas);

  static void AddListener(ESfxChannels channel, const CVector3f& pos, const CVector3f& dir,
                          const CVector3f& vec1, const CVector3f& vec2, float f1, float f2,
                          float f3, uint w1, const uchar maxVolume);

  static void Shutdown();
  static void StopAndRemoveAllEmitters();
  static bool LoadTranslationTable(CSimplePool* pool, const SObjectTag* tag);
  static ushort TranslateSFXID(ushort);
  static void SetActiveAreas(const rstl::reserved_vector< int, 10 >& areas);

  static void PitchBend(CSfxHandle handle, int pitch);
  static void SetDuration(CSfxHandle handle, float duration);
  static const short GetReverbAmount();

  static CSfxHandle SfxStart(const ushort id, const short vol = 127, const short pan = 64,
                             bool useAcoustics = false, const short prio = kMedPriority,
                             const bool looped = false, const int areaId = kAllAreas);
  static void SfxStop(CSfxHandle handle);
  static void SfxVolume(CSfxHandle handle, uchar volume);
  static void SfxSpan(CSfxHandle, uchar);

  static bool IsPlaying(CSfxHandle handle);
  static void StopSound(CSfxHandle handle);

  static void SetChannel(ESfxChannels);
  static void KillAll(ESfxChannels);
  static void TurnOnChannel(ESfxChannels);
  static void TurnOffChannel(ESfxChannels);
  static CSfxHandle LocateHandle(const short id);

  static CSfxEmitterWrapper* AllocateCSfxEmitterWrapper(const CSfxEmitterWrapper& wrapper);
  static CSfxWrapper* AllocateCSfxWrapper(const CSfxWrapper& wrapper);
  static const bool IsAuxProcessingEnabled();
  static void ApplyReverb();
  static void DisableAuxCallbacks();
  static void EnableAuxCallbacks();
  static void PrepareDelayCallback(const SND_AUX_DELAY& info);
  static void PrepareReverbStdCallback(const SND_AUX_REVERBSTD& info);
  static void PrepareChorusCallback(const SND_AUX_CHORUS& info);
  static void PrepareReverbHiCallback(const SND_AUX_REVERBHI& info);
  static void DisableAuxProcessing();
  static CSfxChannel mChannels[4];
  static ESfxChannels mCurrentChannel;
  static bool mDoUpdate;
  static EAuxEffect mCurrentAuxEffect;
  static EAuxEffect mRequestedAuxEffect;
  static float mReverbAmount;
  static float mReverbScale;
  static rstl::vector< short >* mTranslationTable;
  static bool mMuted;
  static rstl::auto_ptr< CToken > mTranslationTableTok;
  static bool mAuxProcessingEnabled;
  static rstl::reserved_vector< CSfxEmitterWrapper, 64 > mEmitterWrapperPool;
  static rstl::reserved_vector< CSfxWrapper, 64 > mWrapperPool;

  static bool IsHandleValid(CSfxHandle handle);
  static int GetRank(CBaseSfxWrapper* wrapper);
};

inline CSfxManager::CBaseSfxWrapper::~CBaseSfxWrapper() {}

const CFactoryFnReturn FAudioTranslationTableFactory(const SObjectTag& obj, CInputStream& in,
                                               const CVParamTransfer& xfer);

#endif // _CSFXMANAGER
