#include "Runtime/Graphics/CMoviePlayer.hpp"

#include "Runtime/CDvdRequest.hpp"
#include "Runtime/Audio/g721.h"
#include "Runtime/Graphics/CGraphics.hpp"

#include <amuse/DSPCodec.hpp>
#include <boo/graphicsdev/GLSLMacros.hpp>
#include <hecl/Pipeline.hpp>
#include <specter/View.hpp>
#include <turbojpeg.h>

namespace urde {

/* used in the original to look up fixed-point dividends on a
 * MIDI-style volume scale (0-127) -> (n/0x8000) */
static const u16 StaticVolumeLookup[] = {
    0x0000, 0x0002, 0x0008, 0x0012, 0x0020, 0x0032, 0x0049, 0x0063, 0x0082, 0x00A4, 0x00CB, 0x00F5, 0x0124,
    0x0157, 0x018E, 0x01C9, 0x0208, 0x024B, 0x0292, 0x02DD, 0x032C, 0x037F, 0x03D7, 0x0432, 0x0492, 0x04F5,
    0x055D, 0x05C9, 0x0638, 0x06AC, 0x0724, 0x07A0, 0x0820, 0x08A4, 0x092C, 0x09B8, 0x0A48, 0x0ADD, 0x0B75,
    0x0C12, 0x0CB2, 0x0D57, 0x0DFF, 0x0EAC, 0x0F5D, 0x1012, 0x10CA, 0x1187, 0x1248, 0x130D, 0x13D7, 0x14A4,
    0x1575, 0x164A, 0x1724, 0x1801, 0x18E3, 0x19C8, 0x1AB2, 0x1BA0, 0x1C91, 0x1D87, 0x1E81, 0x1F7F, 0x2081,
    0x2187, 0x2291, 0x239F, 0x24B2, 0x25C8, 0x26E2, 0x2801, 0x2923, 0x2A4A, 0x2B75, 0x2CA3, 0x2DD6, 0x2F0D,
    0x3048, 0x3187, 0x32CA, 0x3411, 0x355C, 0x36AB, 0x37FF, 0x3956, 0x3AB1, 0x3C11, 0x3D74, 0x3EDC, 0x4048,
    0x41B7, 0x432B, 0x44A3, 0x461F, 0x479F, 0x4923, 0x4AAB, 0x4C37, 0x4DC7, 0x4F5C, 0x50F4, 0x5290, 0x5431,
    0x55D6, 0x577E, 0x592B, 0x5ADC, 0x5C90, 0x5E49, 0x6006, 0x61C7, 0x638C, 0x6555, 0x6722, 0x68F4, 0x6AC9,
    0x6CA2, 0x6E80, 0x7061, 0x7247, 0x7430, 0x761E, 0x7810, 0x7A06, 0x7C00, 0x7DFE, 0x8000};

/* shared boo resources */
static boo::ObjToken<boo::IShaderPipeline> YUVShaderPipeline;
static tjhandle TjHandle = nullptr;

/* RSF audio state */
static const u8* StaticAudio = nullptr;
static u32 StaticAudioSize = 0;
static u32 StaticAudioOffset = 0;
static u16 StaticVolumeAtten = 0x50F4;
static u32 StaticLoopBegin = 0;
static u32 StaticLoopEnd = 0;
static g72x_state StaticStateLeft = {};
static g72x_state StaticStateRight = {};

/* THP SFX audio */
static float SfxVolume = 1.f;

static const char* BlockNames[] = {"SpecterViewBlock"};
static const char* TexNames[] = {"texY", "texU", "texV"};

void CMoviePlayer::Initialize() {
  YUVShaderPipeline = hecl::conv->convert(Shader_CMoviePlayerShader{});
  TjHandle = tjInitDecompress();
}

void CMoviePlayer::Shutdown() {
  YUVShaderPipeline.reset();
  tjDestroy(TjHandle);
}

void CMoviePlayer::THPHeader::swapBig() {
  magic = hecl::SBig(magic);
  version = hecl::SBig(version);
  maxBufferSize = hecl::SBig(maxBufferSize);
  maxAudioSamples = hecl::SBig(maxAudioSamples);
  fps = hecl::SBig(fps);
  numFrames = hecl::SBig(numFrames);
  firstFrameSize = hecl::SBig(firstFrameSize);
  dataSize = hecl::SBig(dataSize);
  componentDataOffset = hecl::SBig(componentDataOffset);
  offsetsDataOffset = hecl::SBig(offsetsDataOffset);
  firstFrameOffset = hecl::SBig(firstFrameOffset);
  lastFrameOffset = hecl::SBig(lastFrameOffset);
}

void CMoviePlayer::THPComponents::swapBig() { numComponents = hecl::SBig(numComponents); }

void CMoviePlayer::THPVideoInfo::swapBig() {
  width = hecl::SBig(width);
  height = hecl::SBig(height);
}

void CMoviePlayer::THPAudioInfo::swapBig() {
  numChannels = hecl::SBig(numChannels);
  sampleRate = hecl::SBig(sampleRate);
  numSamples = hecl::SBig(numSamples);
}

void CMoviePlayer::THPFrameHeader::swapBig() {
  nextSize = hecl::SBig(nextSize);
  prevSize = hecl::SBig(prevSize);
  imageSize = hecl::SBig(imageSize);
  audioSize = hecl::SBig(audioSize);
}

void CMoviePlayer::THPAudioFrameHeader::swapBig() {
  channelSize = hecl::SBig(channelSize);
  numSamples = hecl::SBig(numSamples);
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 8; ++j) {
      channelCoefs[i][j][0] = hecl::SBig(channelCoefs[i][j][0]);
      channelCoefs[i][j][1] = hecl::SBig(channelCoefs[i][j][1]);
    }
    channelPrevs[i][0] = hecl::SBig(channelPrevs[i][0]);
    channelPrevs[i][1] = hecl::SBig(channelPrevs[i][1]);
  }
}

/* Slightly modified from THPAudioDecode present in SDK; always interleaves */
u32 CMoviePlayer::THPAudioDecode(s16* buffer, const u8* audioFrame, bool stereo) {
  THPAudioFrameHeader header = *((const THPAudioFrameHeader*)audioFrame);
  header.swapBig();
  audioFrame += sizeof(THPAudioFrameHeader);

  if (stereo) {
    for (int i = 0; i < 2; ++i) {
      unsigned samples = header.numSamples;
      s16* bufferCur = buffer + i;
      int16_t prev1 = header.channelPrevs[i][0];
      int16_t prev2 = header.channelPrevs[i][1];
      for (u32 f = 0; f < (header.numSamples + 13) / 14; ++f) {
        DSPDecompressFrameStereoStride(bufferCur, audioFrame, header.channelCoefs[i], &prev1, &prev2, samples);
        samples -= 14;
        bufferCur += 28;
        audioFrame += 8;
      }
    }
  } else {
    unsigned samples = header.numSamples;
    s16* bufferCur = buffer;
    int16_t prev1 = header.channelPrevs[0][0];
    int16_t prev2 = header.channelPrevs[0][1];
    for (u32 f = 0; f < (header.numSamples + 13) / 14; ++f) {
      DSPDecompressFrameStereoDupe(bufferCur, audioFrame, header.channelCoefs[0], &prev1, &prev2, samples);
      samples -= 14;
      bufferCur += 28;
      audioFrame += 8;
    }
  }

  return header.numSamples;
}

CMoviePlayer::CMoviePlayer(const char* path, float preLoadSeconds, bool loop, bool deinterlace)
: CDvdFile(path)
, xec_preLoadSeconds(preLoadSeconds)
, xf4_24_loop(loop)
, m_deinterlace(deinterlace) {
  /* Read THP header information */
  u8 buf[64];
  SyncRead(buf, 64);
  memmove(&x28_thpHead, buf, 48);
  x28_thpHead.swapBig();

  u32 cur = x28_thpHead.componentDataOffset;
  SyncSeekRead(buf, 32, ESeekOrigin::Begin, cur);
  memmove(&x58_thpComponents, buf, 20);
  cur += 20;
  x58_thpComponents.swapBig();

  for (u32 i = 0; i < x58_thpComponents.numComponents; ++i) {
    switch (x58_thpComponents.comps[i]) {
    case THPComponents::Type::Video:
      SyncSeekRead(buf, 32, ESeekOrigin::Begin, cur);
      memmove(&x6c_videoInfo, buf, 8);
      cur += 8;
      x6c_videoInfo.swapBig();
      break;
    case THPComponents::Type::Audio:
      SyncSeekRead(buf, 32, ESeekOrigin::Begin, cur);
      memmove(&x74_audioInfo, buf, 12);
      cur += 12;
      x74_audioInfo.swapBig();
      xf4_25_hasAudio = true;
      break;
    default:
      break;
    }
  }

  /* Initial read state */
  xb4_nextReadOff = x28_thpHead.firstFrameOffset;
  xb0_nextReadSize = x28_thpHead.firstFrameSize;
  xb8_readSizeWrapped = x28_thpHead.firstFrameSize;
  xbc_readOffWrapped = x28_thpHead.firstFrameOffset;

  xe4_totalSeconds = x28_thpHead.numFrames / x28_thpHead.fps;
  if (xec_preLoadSeconds < 0.f) {
    /* Load whole video */
    xec_preLoadSeconds = xe4_totalSeconds;
    xf0_preLoadFrames = x28_thpHead.numFrames;
  } else if (xec_preLoadSeconds > 0.f) {
    /* Pre-load video portion */
    u32 frame = xec_preLoadSeconds * x28_thpHead.fps;
    xf0_preLoadFrames = std::min(frame, x28_thpHead.numFrames);
    xec_preLoadSeconds = std::min(xe4_totalSeconds, xec_preLoadSeconds);
  }

  if (xf0_preLoadFrames > 0)
    xa0_bufferQueue.reserve(xf0_preLoadFrames);

  /* All set for GPU resources */
  CGraphics::CommitResources([&](boo::IGraphicsDataFactory::Context& ctx) {
    m_blockBuf = ctx.newDynamicBuffer(boo::BufferUse::Uniform, sizeof(m_viewVertBlock), 1);
    m_vertBuf = ctx.newDynamicBuffer(boo::BufferUse::Vertex, sizeof(specter::View::TexShaderVert), 4);

    /* Allocate textures here (rather than at decode time) */
    x80_textures.reserve(3);
    for (int i = 0; i < 3; ++i) {
      CTHPTextureSet& set = x80_textures.emplace_back();
      if (deinterlace) {
        /* urde addition: this way interlaced THPs don't look horrible */
        set.Y[0] = ctx.newDynamicTexture(x6c_videoInfo.width, x6c_videoInfo.height / 2, boo::TextureFormat::I8,
                                         boo::TextureClampMode::Repeat);
        set.Y[1] = ctx.newDynamicTexture(x6c_videoInfo.width, x6c_videoInfo.height / 2, boo::TextureFormat::I8,
                                         boo::TextureClampMode::Repeat);
        set.U = ctx.newDynamicTexture(x6c_videoInfo.width / 2, x6c_videoInfo.height / 2, boo::TextureFormat::I8,
                                      boo::TextureClampMode::Repeat);
        set.V = ctx.newDynamicTexture(x6c_videoInfo.width / 2, x6c_videoInfo.height / 2, boo::TextureFormat::I8,
                                      boo::TextureClampMode::Repeat);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {m_blockBuf.get()};
        for (int j = 0; j < 2; ++j) {
          boo::ObjToken<boo::ITexture> texs[] = {set.Y[j].get(), set.U.get(), set.V.get()};
          set.binding[j] = ctx.newShaderDataBinding(YUVShaderPipeline, m_vertBuf.get(), nullptr, nullptr, 1, bufs,
                                                    nullptr, 3, texs, nullptr, nullptr);
        }
      } else {
        /* normal progressive presentation */
        set.Y[0] = ctx.newDynamicTexture(x6c_videoInfo.width, x6c_videoInfo.height, boo::TextureFormat::I8,
                                         boo::TextureClampMode::Repeat);
        set.U = ctx.newDynamicTexture(x6c_videoInfo.width / 2, x6c_videoInfo.height / 2, boo::TextureFormat::I8,
                                      boo::TextureClampMode::Repeat);
        set.V = ctx.newDynamicTexture(x6c_videoInfo.width / 2, x6c_videoInfo.height / 2, boo::TextureFormat::I8,
                                      boo::TextureClampMode::Repeat);

        boo::ObjToken<boo::IGraphicsBuffer> bufs[] = {m_blockBuf.get()};
        boo::ObjToken<boo::ITexture> texs[] = {set.Y[0].get(), set.U.get(), set.V.get()};
        set.binding[0] = ctx.newShaderDataBinding(YUVShaderPipeline, m_vertBuf.get(), nullptr, nullptr, 1, bufs,
                                                  nullptr, 3, texs, nullptr, nullptr);
      }
      if (xf4_25_hasAudio)
        set.audioBuf.reset(new s16[x28_thpHead.maxAudioSamples * 2]);
    }
    return true;
  } BooTrace);

  /* Temporary planar YUV decode buffer, resulting planes copied to Boo */
  m_yuvBuf.reset(new uint8_t[tjBufSizeYUV(x6c_videoInfo.width, x6c_videoInfo.height, TJ_420)]);

  /* Schedule initial read */
  PostDVDReadRequestIfNeeded();

  m_frame[0].m_uv = {0.f, 0.f};
  m_frame[1].m_uv = {0.f, 1.f};
  m_frame[2].m_uv = {1.f, 0.f};
  m_frame[3].m_uv = {1.f, 1.f};
  SetFrame({-0.5f, 0.5f, 0.f}, {-0.5f, -0.5f, 0.f}, {0.5f, -0.5f, 0.f}, {0.5f, 0.5f, 0.f});

  m_viewVertBlock.finalAssign(m_viewVertBlock);
  m_blockBuf->load(&m_viewVertBlock, sizeof(m_viewVertBlock));
}

void CMoviePlayer::SetStaticAudioVolume(int vol) {
  StaticVolumeAtten = StaticVolumeLookup[std::max(0, std::min(127, vol))];
}

void CMoviePlayer::SetStaticAudio(const void* data, u32 size, u32 loopBegin, u32 loopEnd) {
  StaticAudio = reinterpret_cast<const u8*>(data);
  StaticAudioSize = size;
  StaticLoopBegin = loopBegin;
  StaticLoopEnd = loopEnd;
  StaticAudioOffset = 0;

  g72x_init_state(&StaticStateLeft);
  g72x_init_state(&StaticStateRight);
}

void CMoviePlayer::SetSfxVolume(u8 volume) { SfxVolume = std::min(volume, u8(127)) / 127.f; }

void CMoviePlayer::MixAudio(s16* out, const s16* in, u32 samples) {
  /* No audio frames ready */
  if (xd4_audioSlot == UINT32_MAX) {
    if (in)
      memmove(out, in, samples * 4);
    else
      memset(out, 0, samples * 4);
    return;
  }

  while (samples) {
    CTHPTextureSet* tex = &x80_textures[xd4_audioSlot];
    u32 thisSamples = std::min(tex->audioSamples - tex->playedSamples, samples);
    if (!thisSamples) {
      /* Advance frame */
      ++xd4_audioSlot;
      if (xd4_audioSlot >= x80_textures.size())
        xd4_audioSlot = 0;
      tex = &x80_textures[xd4_audioSlot];
      thisSamples = std::min(tex->audioSamples - tex->playedSamples, samples);
    }

    if (thisSamples) {
      /* mix samples with `in` or no mix */
      if (in) {
        for (u32 i = 0; i < thisSamples; ++i, out += 2, in += 2) {
          out[0] = DSPSampClamp(in[0] + s32(tex->audioBuf[(i + tex->playedSamples) * 2]) * 0x50F4 / 0x8000 * SfxVolume);
          out[1] =
              DSPSampClamp(in[1] + s32(tex->audioBuf[(i + tex->playedSamples) * 2 + 1]) * 0x50F4 / 0x8000 * SfxVolume);
        }
      } else {
        for (u32 i = 0; i < thisSamples; ++i, out += 2) {
          out[0] = DSPSampClamp(s32(tex->audioBuf[(i + tex->playedSamples) * 2]) * 0x50F4 / 0x8000 * SfxVolume);
          out[1] = DSPSampClamp(s32(tex->audioBuf[(i + tex->playedSamples) * 2 + 1]) * 0x50F4 / 0x8000 * SfxVolume);
        }
      }
      tex->playedSamples += thisSamples;
      samples -= thisSamples;
    } else {
      /* urde addition: failsafe for buffer overrun */
      if (in)
        memmove(out, in, samples * 4);
      else
        memset(out, 0, samples * 4);
      // fprintf(stderr, "dropped %d samples\n", samples);
      return;
    }
  }
}

void CMoviePlayer::MixStaticAudio(s16* out, const s16* in, u32 samples) {
  if (!StaticAudio)
    return;
  while (samples) {
    u32 thisSamples = std::min(StaticLoopEnd - StaticAudioOffset, samples);
    const u8* thisOffsetLeft = &StaticAudio[StaticAudioOffset / 2];
    const u8* thisOffsetRight = &StaticAudio[StaticAudioSize / 2 + StaticAudioOffset / 2];

    /* urde addition: mix samples with `in` or no mix */
    if (in) {
      for (u32 i = 0; i < thisSamples; i += 2) {
        out[0] = DSPSampClamp(
            in[0] + s32(g721_decoder(thisOffsetLeft[0] & 0xf, &StaticStateLeft) * StaticVolumeAtten / 0x8000));
        out[1] = DSPSampClamp(
            in[1] + s32(g721_decoder(thisOffsetRight[0] & 0xf, &StaticStateRight) * StaticVolumeAtten / 0x8000));
        out[2] = DSPSampClamp(
            in[2] + s32(g721_decoder(thisOffsetLeft[0] >> 4 & 0xf, &StaticStateLeft) * StaticVolumeAtten / 0x8000));
        out[3] = DSPSampClamp(
            in[3] + s32(g721_decoder(thisOffsetRight[0] >> 4 & 0xf, &StaticStateRight) * StaticVolumeAtten / 0x8000));
        thisOffsetLeft += 1;
        thisOffsetRight += 1;
        out += 4;
        in += 4;
      }
    } else {
      for (u32 i = 0; i < thisSamples; i += 2) {
        out[0] =
            DSPSampClamp(s32(g721_decoder(thisOffsetLeft[0] & 0xf, &StaticStateLeft) * StaticVolumeAtten / 0x8000));
        out[1] =
            DSPSampClamp(s32(g721_decoder(thisOffsetRight[0] & 0xf, &StaticStateRight) * StaticVolumeAtten / 0x8000));
        out[2] = DSPSampClamp(
            s32(g721_decoder(thisOffsetLeft[0] >> 4 & 0xf, &StaticStateLeft) * StaticVolumeAtten / 0x8000));
        out[3] = DSPSampClamp(
            s32(g721_decoder(thisOffsetRight[0] >> 4 & 0xf, &StaticStateRight) * StaticVolumeAtten / 0x8000));
        thisOffsetLeft += 1;
        thisOffsetRight += 1;
        out += 4;
      }
    }

    StaticAudioOffset += thisSamples;
    if (StaticAudioOffset == StaticLoopEnd)
      StaticAudioOffset = StaticLoopBegin;
    samples -= thisSamples;
  }
}

void CMoviePlayer::Rewind() {
  if (x98_request) {
    x98_request->PostCancelRequest();
    x98_request.reset();
  }

  xb0_nextReadSize = x28_thpHead.firstFrameSize;
  xb4_nextReadOff = x28_thpHead.firstFrameOffset;
  xb8_readSizeWrapped = x28_thpHead.firstFrameSize;
  xbc_readOffWrapped = x28_thpHead.firstFrameOffset;

  xc0_curLoadFrame = 0;
  xc4_requestFrameWrapped = 0;
  xc8_curFrame = 0;
  xcc_decodedTexSlot = 0;
  xd0_drawTexSlot = -1;
  xd4_audioSlot = -1;
  xd8_decodedTexCount = 0;
  xdc_frameRem = 0.f;
  xe8_curSeconds = 0.f;
}

void CMoviePlayer::SetFrame(const zeus::CVector3f& a, const zeus::CVector3f& b, const zeus::CVector3f& c,
                            const zeus::CVector3f& d) {
  m_frame[0].m_pos = a;
  m_frame[1].m_pos = b;
  m_frame[2].m_pos = d;
  m_frame[3].m_pos = c;
  m_vertBuf->load(m_frame, sizeof(m_frame));
}

void CMoviePlayer::DrawFrame() {
  if (xd0_drawTexSlot == UINT32_MAX)
    return;
  SCOPED_GRAPHICS_DEBUG_GROUP("CMoviePlayer::DrawFrame", zeus::skYellow);

  /* draw appropriate field */
  CTHPTextureSet& tex = x80_textures[xd0_drawTexSlot];
  CGraphics::SetShaderDataBinding(tex.binding[m_deinterlace ? (xfc_fieldIndex != 0) : 0]);
  CGraphics::DrawArray(0, 4);

  /* ensure second field is being displayed by VI to signal advance
   * (faked in urde with continuous xor) */
  if (!xfc_fieldIndex && CGraphics::g_LastFrameUsedAbove)
    xf4_26_fieldFlip = true;

  ++xfc_fieldIndex;
}

void CMoviePlayer::Update(float dt) {
  if (xc0_curLoadFrame < xf0_preLoadFrames) {
    /* in buffering phase, ensure read data is stored for mem-cache access */
    if (x98_request && x98_request->IsComplete()) {
      ReadCompleted();
      if (xc0_curLoadFrame >= xa0_bufferQueue.size() && xc0_curLoadFrame < xf0_preLoadFrames &&
          xa0_bufferQueue.size() < x28_thpHead.numFrames) {
        PostDVDReadRequestIfNeeded();
      }
    }
  } else {
    /* out of buffering phase, skip mem-cache straight to decode */
    if (x98_request) {
      bool flag = false;
      if (xc4_requestFrameWrapped >= xa0_bufferQueue.size() && xc0_curLoadFrame >= xa0_bufferQueue.size())
        flag = true;
      if (x98_request->IsComplete() && xd8_decodedTexCount < 2 && flag) {
        DecodeFromRead(x90_requestBuf.get());
        ReadCompleted();
        PostDVDReadRequestIfNeeded();
        ++xd8_decodedTexCount;
        ++xc4_requestFrameWrapped;
        if (xc4_requestFrameWrapped >= x28_thpHead.numFrames && xf4_24_loop)
          xc4_requestFrameWrapped = 0;
      }
    }
  }

  /* submit request for additional read to keep stream-consumer happy
   * (if buffer slot is available) */
  if (!x98_request && xe0_playMode == EPlayMode::Playing && xa0_bufferQueue.size() < x28_thpHead.numFrames) {
    PostDVDReadRequestIfNeeded();
  }

  /* decode frame directly from mem-cache if needed */
  if (xd8_decodedTexCount < 2) {
    if (xe0_playMode == EPlayMode::Playing && xc4_requestFrameWrapped < xf0_preLoadFrames) {
      u32 minFrame = std::min(u32(xa0_bufferQueue.size()) - 1, xc4_requestFrameWrapped);
      if (minFrame == UINT32_MAX)
        return;
      std::unique_ptr<uint8_t[]>& frameData = xa0_bufferQueue[minFrame];
      DecodeFromRead(frameData.get());
      ++xd8_decodedTexCount;
      ++xc4_requestFrameWrapped;
      if (xc4_requestFrameWrapped >= x28_thpHead.numFrames && xf4_24_loop)
        xc4_requestFrameWrapped = 0;
    }
  }

  /* paused THPs shall not pass */
  if (xd8_decodedTexCount <= 0 || xe0_playMode != EPlayMode::Playing)
    return;

  /* timing update */
  xe8_curSeconds += dt;
  if (xf4_24_loop)
    xe8_curSeconds = std::fmod(xe8_curSeconds, xe4_totalSeconds);
  else
    xe8_curSeconds = std::min(xe4_totalSeconds, xe8_curSeconds);

  /* test remainder threshold, determine if frame needs to be advanced */
  float frameDt = 1.f / x28_thpHead.fps;
  float rem = xdc_frameRem - dt;
  if (rem <= 0.f) {
    if (!xf4_26_fieldFlip) {
      /* second field has drawn, advance consumer-queue to next THP frame */
      ++xd0_drawTexSlot;
      if (xd0_drawTexSlot >= x80_textures.size())
        xd0_drawTexSlot = 0;
      if (xd4_audioSlot == UINT32_MAX)
        xd4_audioSlot = 0;
      --xd8_decodedTexCount;
      ++xc8_curFrame;
      if (xc8_curFrame == x28_thpHead.numFrames && xf4_24_loop)
        xc8_curFrame = 0;
      rem += frameDt;
      xfc_fieldIndex = 0;
    } else {
      /* advance timing within second field */
      rem += dt;
      xf4_26_fieldFlip = false;
    }
  }
  xdc_frameRem = rem;
}

void CMoviePlayer::DecodeFromRead(const void* data) {
  const u8* inptr = (u8*)data;
  CTHPTextureSet& tex = x80_textures[xcc_decodedTexSlot];

  THPFrameHeader frameHeader = *static_cast<const THPFrameHeader*>(data);
  frameHeader.swapBig();
  inptr += 8 + x58_thpComponents.numComponents * 4;

  for (u32 i = 0; i < x58_thpComponents.numComponents; ++i) {
    switch (x58_thpComponents.comps[i]) {
    case THPComponents::Type::Video: {
      tjDecompressToYUV(TjHandle, (u8*)inptr, frameHeader.imageSize, m_yuvBuf.get(), 0);
      inptr += frameHeader.imageSize;

      uintptr_t planeSize = x6c_videoInfo.width * x6c_videoInfo.height;
      uintptr_t planeSizeHalf = planeSize / 2;
      uintptr_t planeSizeQuarter = planeSizeHalf / 2;

      if (m_deinterlace) {
        /* Deinterlace into 2 discrete 60-fps half-res textures */
        u8* mappedData = (u8*)tex.Y[0]->map(planeSizeHalf);
        for (unsigned y = 0; y < x6c_videoInfo.height / 2; ++y) {
          memmove(mappedData + x6c_videoInfo.width * y, m_yuvBuf.get() + x6c_videoInfo.width * (y * 2),
                  x6c_videoInfo.width);
        }
        tex.Y[0]->unmap();

        mappedData = (u8*)tex.Y[1]->map(planeSizeHalf);
        for (unsigned y = 0; y < x6c_videoInfo.height / 2; ++y) {
          memmove(mappedData + x6c_videoInfo.width * y, m_yuvBuf.get() + x6c_videoInfo.width * (y * 2 + 1),
                  x6c_videoInfo.width);
        }
        tex.Y[1]->unmap();

        tex.U->load(m_yuvBuf.get() + planeSize, planeSizeQuarter);
        tex.V->load(m_yuvBuf.get() + planeSize + planeSizeQuarter, planeSizeQuarter);
      } else {
        /* Direct planar load */
        tex.Y[0]->load(m_yuvBuf.get(), planeSize);
        tex.U->load(m_yuvBuf.get() + planeSize, planeSizeQuarter);
        tex.V->load(m_yuvBuf.get() + planeSize + planeSizeQuarter, planeSizeQuarter);
      }

      break;
    }
    case THPComponents::Type::Audio:
      memset(tex.audioBuf.get(), 0, x28_thpHead.maxAudioSamples * 4);
      tex.audioSamples = THPAudioDecode(tex.audioBuf.get(), (u8*)inptr, x74_audioInfo.numChannels == 2);
      tex.playedSamples = 0;
      inptr += frameHeader.audioSize;
      break;
    default:
      break;
    }
  }

  /* advance YUV producer-queue slot */
  ++xcc_decodedTexSlot;
  if (xcc_decodedTexSlot == x80_textures.size())
    xcc_decodedTexSlot = 0;
}

void CMoviePlayer::ReadCompleted() {
  std::unique_ptr<uint8_t[]> buffer = std::move(x90_requestBuf);
  x98_request.reset();
  const THPFrameHeader* frameHeader = reinterpret_cast<const THPFrameHeader*>(buffer.get());

  /* transfer request buffer to mem-cache if needed */
  if (xc0_curLoadFrame == xa0_bufferQueue.size() && xf0_preLoadFrames > xc0_curLoadFrame)
    xa0_bufferQueue.push_back(std::move(buffer));

  /* store params of next read operation */
  xb4_nextReadOff += xb0_nextReadSize;
  xb0_nextReadSize = hecl::SBig(frameHeader->nextSize);
  ++xc0_curLoadFrame;

  if (xc0_curLoadFrame == xf0_preLoadFrames) {
    if (x28_thpHead.numFrames == xc0_curLoadFrame) {
      xb8_readSizeWrapped = x28_thpHead.firstFrameSize;
      xbc_readOffWrapped = x28_thpHead.firstFrameOffset;
    } else {
      xb8_readSizeWrapped = xb0_nextReadSize;
      xbc_readOffWrapped = xb4_nextReadOff;
    }
  }

  /* handle loop-event read */
  if (xc0_curLoadFrame >= x28_thpHead.numFrames && xf4_24_loop) {
    xb4_nextReadOff = xbc_readOffWrapped;
    xb0_nextReadSize = xb8_readSizeWrapped;
    xc0_curLoadFrame = xf0_preLoadFrames;
  }
}

void CMoviePlayer::PostDVDReadRequestIfNeeded() {
  if (xc0_curLoadFrame < x28_thpHead.numFrames) {
    x90_requestBuf.reset(new uint8_t[xb0_nextReadSize]);
    x98_request = AsyncSeekRead(x90_requestBuf.get(), xb0_nextReadSize, ESeekOrigin::Begin, xb4_nextReadOff);
  }
}

} // namespace urde
