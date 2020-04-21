#pragma once

#include <memory>
#include <vector>

#include "Runtime/CDvdFile.hpp"
#include "Runtime/RetroTypes.hpp"

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>
#include <specter/View.hpp>
#include <zeus/CVector3f.hpp>

namespace urde {

class CMoviePlayer : public CDvdFile {
public:
  enum class EPlayMode { Stopped, Playing };

private:
  struct THPHeader {
    u32 magic;
    u32 version;
    u32 maxBufferSize;
    u32 maxAudioSamples;
    float fps;
    u32 numFrames;
    u32 firstFrameSize;
    u32 dataSize;
    u32 componentDataOffset;
    u32 offsetsDataOffset;
    u32 firstFrameOffset;
    u32 lastFrameOffset;
    void swapBig();
  } x28_thpHead;

  struct THPComponents {
    u32 numComponents;
    enum class Type : u8 { Video = 0x0, Audio = 0x1, None = 0xff } comps[16];
    void swapBig();
  } x58_thpComponents;

  struct THPVideoInfo {
    u32 width;
    u32 height;
    void swapBig();
  } x6c_videoInfo;

  struct THPAudioInfo {
    u32 numChannels;
    u32 sampleRate;
    u32 numSamples;
    void swapBig();
  } x74_audioInfo;

  struct THPFrameHeader {
    u32 nextSize;
    u32 prevSize;
    u32 imageSize;
    u32 audioSize;
    void swapBig();
  };

  struct THPAudioFrameHeader {
    u32 channelSize;
    u32 numSamples;
    s16 channelCoefs[2][8][2];
    s16 channelPrevs[2][2];
    void swapBig();
  };

  struct CTHPTextureSet {
    boo::ObjToken<boo::ITextureD> Y[2];
    boo::ObjToken<boo::ITextureD> U;
    boo::ObjToken<boo::ITextureD> V;
    u32 playedSamples = 0;
    u32 audioSamples = 0;
    std::unique_ptr<s16[]> audioBuf;
    boo::ObjToken<boo::IShaderDataBinding> binding[2];
  };
  std::vector<CTHPTextureSet> x80_textures;
  std::unique_ptr<uint8_t[]> x90_requestBuf;
  std::shared_ptr<IDvdRequest> x98_request;
  std::vector<std::unique_ptr<uint8_t[]>> xa0_bufferQueue;

  u32 xb0_nextReadSize = 0;
  u32 xb4_nextReadOff = 0;
  u32 xb8_readSizeWrapped = 0;
  u32 xbc_readOffWrapped = 0;
  u32 xc0_curLoadFrame = 0;
  u32 xc4_requestFrameWrapped = 0;
  u32 xc8_curFrame = 0;
  u32 xcc_decodedTexSlot = 0;
  u32 xd0_drawTexSlot = -1;
  u32 xd4_audioSlot = -1;
  s32 xd8_decodedTexCount = 0;
  float xdc_frameRem = 0.f;
  EPlayMode xe0_playMode = EPlayMode::Playing;
  float xe4_totalSeconds = 0.f;
  float xe8_curSeconds = 0.f;
  float xec_preLoadSeconds;
  u32 xf0_preLoadFrames = 0;
  bool xf4_24_loop : 1;
  bool xf4_25_hasAudio : 1 = false;
  bool xf4_26_fieldFlip : 1 = false;
  bool m_deinterlace : 1;
  u32 xf8_ = 0;
  u32 xfc_fieldIndex = 0;

  std::unique_ptr<uint8_t[]> m_yuvBuf;

  specter::View::ViewBlock m_viewVertBlock;
  boo::ObjToken<boo::IGraphicsBufferD> m_blockBuf;
  boo::ObjToken<boo::IGraphicsBufferD> m_vertBuf;

  specter::View::TexShaderVert m_frame[4];

  static u32 THPAudioDecode(s16* buffer, const u8* audioFrame, bool stereo);
  void DecodeFromRead(const void* data);
  void ReadCompleted();
  void PostDVDReadRequestIfNeeded();

public:
  CMoviePlayer(const char* path, float preLoadSeconds, bool loop, bool deinterlace);

  static void DisableStaticAudio() { SetStaticAudio(nullptr, 0, 0, 0); }
  static void SetStaticAudioVolume(int vol);
  static void SetStaticAudio(const void* data, u32 size, u32 loopBegin, u32 loopEnd);
  static void SetSfxVolume(u8 volume);
  static void MixStaticAudio(s16* out, const s16* in, u32 samples);
  void MixAudio(s16* out, const s16* in, u32 samples);
  void Rewind();

  bool GetIsMovieFinishedPlaying() const {
    if (xf4_24_loop)
      return false;
    return xc8_curFrame == x28_thpHead.numFrames;
  }
  bool IsLooping() const { return xf4_24_loop; }
  bool GetIsFullyCached() const { return xa0_bufferQueue.size() >= xf0_preLoadFrames; }
  float GetPlayedSeconds() const { return xdc_frameRem + xe8_curSeconds; }
  float GetTotalSeconds() const { return xe4_totalSeconds; }
  void SetPlayMode(EPlayMode mode) { xe0_playMode = mode; }
  void SetFrame(const zeus::CVector3f& a, const zeus::CVector3f& b, const zeus::CVector3f& c, const zeus::CVector3f& d);
  void DrawFrame();
  void Update(float dt);
  std::pair<u32, u32> GetVideoDimensions() const { return {x6c_videoInfo.width, x6c_videoInfo.height}; }

  static void Initialize();
  static void Shutdown();
};

} // namespace urde
