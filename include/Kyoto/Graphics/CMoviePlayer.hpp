#ifndef _CMOVIEPLAYER
#define _CMOVIEPLAYER

#include "Kyoto/CDvdFile.hpp"
#include "Kyoto/CDvdRequest.hpp"
#include "Kyoto/Math/CVector3f.hpp"

#include "rstl/vector.hpp"
#include "types.h"

typedef struct THPHeader {
  char mMagic[4];
  u32 mVersion;
  u32 mBufferSize;
  u32 mAudioMaxSamples;
  f32 mFrameRate;
  u32 mNumFrames;
  u32 mFirstFrameSize;
  u32 mMovieDataSize;
  u32 mCompInfoDataOffsets;
  u32 mOffsetDataOffsets;
  u32 mMovieDataOffsets;
  u32 mFinalFrameDataOffsets;
} THPHeader;
CHECK_SIZEOF(THPVideoInfo, 0x30)

typedef struct THPVideoInfo {
  u32 mXSize;
  u32 mYSize;
  u32 mVideoType;
} THPVideoInfo;
CHECK_SIZEOF(THPVideoInfo, 0xC)

typedef struct THPVideoInfoOld {
  u32 mXSize;
  u32 mYSize;
} THPVideoInfoOld;
CHECK_SIZEOF(THPVideoInfoOld, 0x8)

typedef struct THPAudioInfo {
  u32 mSndChannels;
  u32 mSndFrequency;
  u32 mSndNumSamples;
  u32 mSndNumTracks;
} THPAudioInfo;
CHECK_SIZEOF(THPAudioInfo, 0x18)

typedef struct THPAudioInfoOld {
  u32 mSndChannels;
  u32 mSndFrequency;
  u32 mSndNumSamples;
} THPAudioInfoOld;
CHECK_SIZEOF(THPAudioInfoOld, 0x10)

typedef struct THPFrameCompInfo {
  u32 mNumComponents;
  u8 mFrameComp[16];
} THPFrameCompInfo;
CHECK_SIZEOF(THPFrameCompInfo, 0xC)

class CMoviePlayer {
public:
  enum EPlayMode {
    kPM_Stopped,
    kPM_Playing,
  };

  class CTHPTextureSet {
  public:
    CTHPTextureSet(void* y, void* u, void* v, void* audio)
    : mY(static_cast< uchar* >(y))
    , mU(static_cast< uchar* >(u))
    , mV(static_cast< uchar* >(v))
    , mAudio(static_cast< uchar* >(audio))
    , mAudioSamples(0)
    , mAudioSamplesConsumed(0) {}

    void* Y() { return mY.get(); }

    void* U() { return mU.get(); }

    void* V() { return mV.get(); }

    void* Audio() { return mAudio.get(); }

    uint GetAudioSamples() const { return mAudioSamples; }

    void SetAudioSamples(uint samples) { mAudioSamples = samples; }
    uint GetAudioSamplesConsumed() const { return mAudioSamplesConsumed; }

    void SetAudioSamplesConsumed(uint samples) { mAudioSamplesConsumed = samples; }

  private:
    rstl::auto_ptr< uchar > mY;
    rstl::auto_ptr< uchar > mU;
    rstl::auto_ptr< uchar > mV;
    rstl::auto_ptr< uchar > mAudio;
    uint mAudioSamples;
    uint mAudioSamplesConsumed;
  };

  static void SetSfxVolume(uchar);
  static void SetAudioEnabled(bool enabled);
  static bool GetAudioEnabled();

  CMoviePlayer(const char* path, const float preLoadSeconds, const bool loop,
               const bool deinterlace);
  ~CMoviePlayer();

  void Update(float dt);
  void DecodeFromRead(const void* ptr);

  bool DrawVideo() const;
#if VERSION >= VERSION_GM8P_00
  void DrawFrame(int left, int right, int top, int bottom);
#endif
  void DrawFrame(const CVector3f&, const CVector3f&, const CVector3f&, const CVector3f&);
  void MixAudio(short* out, const short* in, unsigned long samples);

  EPlayMode GetPlayMode() const { return mPlayMode; }
  void SetPlayMode(EPlayMode mode);
  float GetTotalSeconds() const;
  float GetPlayedSeconds() const;
  uint GetWidth() const;
  uint GetHeight() const;
  bool CanDrawVideo() const { return mIndexLoad.null(); }
  bool PumpIndexLoad();
  void Rewind();
  bool GetIsFullyCached() const;
  bool GetIsMovieFinishedPlaying() const;
  void DisableLoop() { mLoop = false; }
  bool IsLooping() const { return mLoop; }

private:
  struct SIndexLoad;
  static void VerifyCallbackStatus();
#if defined(TARGET_PC)
  static void StaticMyAudioCallback(short* output, size_t frames, u32 rate, u32 channels);
#else
  static void StaticMyAudioCallback();
#endif
  void InitializeTextures();
  void ReadCompleted();
  void PostDVDReadRequestIfNeeded();
  CDvdFile mDvdFile;
  THPHeader mHeader;
  THPFrameCompInfo mThpComponents;
  THPVideoInfoOld mVideoInfo;
#if defined(TARGET_PC)
  THPAudioInfoOld mAudioInfo{};
#else
  THPAudioInfoOld mAudioInfo;
#endif
  rstl::vector< CTHPTextureSet > mTextures;
  rstl::auto_ptr< uchar > mRequestBuffer;
  rstl::single_ptr< CDvdRequest > mRequest;
  rstl::vector< rstl::auto_ptr< uchar > > mRequestQueue;
  rstl::single_ptr< SIndexLoad > mIndexLoad;
  uint mNextReadSize;
  uint mNextReadOff;
  uint mReadSizeWrapped;
  uint mReadOffWrapped;
  int mCurLoadFrame;
  int mRequestFrameWrapped;
  int mCurFrame;
  int mDecodedTexSlot;
  int mDrawTexSlot;
  int mAudioSlot;
  int mDecodedTexCount;
  float mFrameRem;
  EPlayMode mPlayMode;
  float mTotalSeconds;
  float mCurSeconds;
  float mPreLoadSeconds;
  int mPreLoadFrames;
  bool mLoop : 1;
  bool mDeinterlace : 1;
  bool mHasAudio : 1;
  bool mFieldFlip : 1;
  uint mCachedBytes;
  int mFieldIndex;
#if defined(TARGET_PC)
  u32 m_audioPhase = 0;
  short m_audioHistory[2][2] = {};
  bool m_audioPrimed = false;
#endif
};
// This is the GameCube instance layout; Trilogy currently uses only the static audio API.
#if VERSION < VERSION_R3IJ_00
CHECK_SIZEOF(CMoviePlayer, 0x100)
#endif

#endif // _CMOVIEPLAYER
