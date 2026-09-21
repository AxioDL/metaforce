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
    : x0_y(static_cast< uchar* >(y))
    , x8_u(static_cast< uchar* >(u))
    , x10_v(static_cast< uchar* >(v))
    , x18_audio(static_cast< uchar* >(audio))
    , x20_audioSamples(0)
    , x24_audioSamplesConsumed(0) {}

    void* Y() { return x0_y.get(); }

    void* U() { return x8_u.get(); }

    void* V() { return x10_v.get(); }

    void* Audio() { return x18_audio.get(); }

    uint GetAudioSamples() const { return x20_audioSamples; }

    void SetAudioSamples(uint samples) { x20_audioSamples = samples; }
    uint GetAudioSamplesConsumed() const { return x24_audioSamplesConsumed; }

    void SetAudioSamplesConsumed(uint samples) { x24_audioSamplesConsumed = samples; }

  private:
    rstl::auto_ptr< uchar > x0_y;
    rstl::auto_ptr< uchar > x8_u;
    rstl::auto_ptr< uchar > x10_v;
    rstl::auto_ptr< uchar > x18_audio;
    uint x20_audioSamples;
    uint x24_audioSamplesConsumed;
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
  void DrawFrame(const CVector3f&, const CVector3f&, const CVector3f&, const CVector3f&);
  void MixAudio(short* out, const short* in, unsigned long samples);

  EPlayMode GetPlayMode() const { return xe0_playMode; }
  void SetPlayMode(EPlayMode mode);
  float GetTotalSeconds() const;
  float GetPlayedSeconds() const;
  uint GetWidth() const;
  uint GetHeight() const;
  bool CanDrawVideo() const { return xac_indexLoad.null(); }
  bool PumpIndexLoad();
  void Rewind();
  bool GetIsFullyCached() const;
  bool GetIsMovieFinishedPlaying() const;
  void DisableLoop() { xf4_24_loop = false; }
  bool IsLooping() const { return xf4_24_loop; }

private:
  struct SIndexLoad;
  static void VerifyCallbackStatus();
  static void StaticMyAudioCallback();
  void InitializeTextures();
  void ReadCompleted();
  void PostDVDReadRequestIfNeeded();
  CDvdFile x0_dvdFile;
  THPHeader x28_header;
  THPFrameCompInfo x58_thpComponents;
  THPVideoInfoOld x6c_videoInfo;
  THPAudioInfoOld x74_audioInfo;
  rstl::vector< CTHPTextureSet > x80_textures;
  rstl::auto_ptr< uchar > x90_requestBuffer;
  rstl::single_ptr< CDvdRequest > x98_request;
  rstl::vector< rstl::auto_ptr< uchar > > x9c_requestQueue;
  rstl::single_ptr< SIndexLoad > xac_indexLoad;
  uint xb0_nextReadSize;
  uint xb4_nextReadOff;
  uint xb8_readSizeWrapped;
  uint xbc_readOffWrapped;
  int xc0_curLoadFrame;
  int xc4_requestFrameWrapped;
  int xc8_curFrame;
  int xcc_decodedTexSlot;
  int xd0_drawTexSlot;
  int xd4_audioSlot;
  int xd8_decodedTexCount;
  float xdc_frameRem;
  EPlayMode xe0_playMode;
  float xe4_totalSeconds;
  float xe8_curSeconds;
  float xec_preLoadSeconds;
  int xf0_preLoadFrames;
  bool xf4_24_loop : 1;
  bool xf4_25_deinterlace : 1;
  bool xf4_26_hasAudio : 1;
  bool xf4_27_fieldFlip : 1;
  uint xf8_cachedBytes;
  int xfc_fieldIndex;
};
CHECK_SIZEOF(CMoviePlayer, 0x100)

#endif // _CMOVIEPLAYER
