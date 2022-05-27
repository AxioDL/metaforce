#include "Graphics/CMoviePlayer.hpp"

#include "Audio/g721.h"
#include "CDvdRequest.hpp"
#include "Graphics/CGraphics.hpp"
#include "Graphics/CCubeRenderer.hpp"
#include "Graphics/CGX.hpp"
#include "GameGlobalObjects.hpp"

#include <amuse/DSPCodec.hpp>
#include <turbojpeg.h>

namespace metaforce {

static void MyTHPYuv2RgbTextureSetup(void* dataY, void* dataU, void* dataV, u16 width, u16 height) {
  GXTexObj texV;
  GXTexObj texU;
  GXTexObj texY;
  GXInitTexObj(&texY, dataY, width, height, GX::TF_I8, GX_CLAMP, GX_CLAMP, false);
  GXInitTexObjLOD(&texY, GX_NEAR, GX_NEAR, 0.0, 0.0, 0.0, false, false, GX_ANISO_1);
  GXLoadTexObj(&texY, GX::TEXMAP0);
  GXInitTexObj(&texU, dataU, width / 2, height / 2, GX::TF_I8, GX_CLAMP, GX_CLAMP, false);
  GXInitTexObjLOD(&texU, GX_NEAR, GX_NEAR, 0.0, 0.0, 0.0, false, false, GX_ANISO_1);
  GXLoadTexObj(&texU, GX::TEXMAP1);
  GXInitTexObj(&texV, dataV, width / 2, height / 2, GX::TF_I8, GX_CLAMP, GX_CLAMP, false);
  GXInitTexObjLOD(&texV, GX_NEAR, GX_NEAR, 0.0, 0.0, 0.0, false, false, GX_ANISO_1);
  GXLoadTexObj(&texV, GX::TEXMAP2);
  CTexture::InvalidateTexMap(GX::TEXMAP0);
  CTexture::InvalidateTexMap(GX::TEXMAP1);
  CTexture::InvalidateTexMap(GX::TEXMAP2);
}

const std::array<u8, 32> InterlaceTex{
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0, 0, 0, 0, 0, 0, 0, 0,
};
static void MyTHPGXYuv2RgbSetup(bool interlaced2ndFrame, bool fieldFlip) {
  CGX::SetZMode(true, GX::ALWAYS, false);
  CGX::SetBlendMode(GX::BM_NONE, GX::BL_ONE, GX::BL_ZERO, GX::LO_CLEAR);
  CGX::SetNumChans(0);
  CGX::SetTexCoordGen(GX::TEXCOORD0, GX::TG_MTX2x4, GX::TG_TEX0, GX::IDENTITY, false, GX::PTIDENTITY);
  CGX::SetTexCoordGen(GX::TEXCOORD1, GX::TG_MTX2x4, GX::TG_TEX0, GX::IDENTITY, false, GX::PTIDENTITY);
  if (!fieldFlip) {
    CGX::SetNumTexGens(3);
    CGX::SetTexCoordGen(GX::TEXCOORD2, GX::TG_MTX2x4, GX::TG_POS, GX::TEXMTX0, false, GX::PTIDENTITY);
    aurora::Mat4x2<float> mtx;
    mtx.m0.x = 0.125f;
    mtx.m2.y = 0.25f;
    if (interlaced2ndFrame) {
      mtx.m3.y = 0.25f;
    }
    GXLoadTexMtxImm(&mtx, GX::TEXMTX0, GX::MTX2x4);
    GXTexObj texObj;
    GXInitTexObj(&texObj, InterlaceTex.data(), 8, 4, GX::TF_I8, GX_REPEAT, GX_REPEAT, false);
    GXInitTexObjLOD(&texObj, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, false, false, GX_ANISO_1);
    GXLoadTexObj(&texObj, GX::TEXMAP3);
    CTexture::InvalidateTexMap(GX::TEXMAP3);
    CGX::SetTevOrder(GX::TEVSTAGE4, GX::TEXCOORD2, GX::TEXMAP3, GX::COLOR_NULL);
    CGX::SetStandardTevColorAlphaOp(GX::TEVSTAGE4);
    CGX::SetTevColorIn(GX::TEVSTAGE4, GX::CC_ZERO, GX::CC_ZERO, GX::CC_ZERO, GX::CC_CPREV);
    CGX::SetTevAlphaIn(GX::TEVSTAGE4, GX::CA_ZERO, GX::CA_ZERO, GX::CA_ZERO, GX::CA_TEXA);
    CGX::SetAlphaCompare(GX::LESS, 128, GX::AOP_AND, GX::ALWAYS, 0);
    CGX::SetNumTevStages(5);
  } else {
    CGX::SetNumTexGens(2);
    CGX::SetNumTevStages(4);
  }
  constexpr std::array vtxDescList{
      GX::VtxDescList{GX::VA_POS, GX::DIRECT},
      GX::VtxDescList{GX::VA_TEX0, GX::DIRECT},
      GX::VtxDescList{},
  };
  CGX::SetVtxDescv(vtxDescList.data());
  GXSetColorUpdate(true);
  GXSetAlphaUpdate(false);
  GXInvalidateTexAll();
  GXSetVtxAttrFmt(GX::VTXFMT7, GX::VA_POS, GX::CLR_RGBA, GX::F32, 0);
  GXSetVtxAttrFmt(GX::VTXFMT7, GX::VA_TEX0, GX::CLR_RGBA, GX::RGBX8, 0);
  CGX::SetTevOrder(GX::TEVSTAGE0, GX::TEXCOORD1, GX::TEXMAP1, GX::COLOR_NULL);
  CGX::SetTevColorIn(GX::TEVSTAGE0, GX::CC_ZERO, GX::CC_TEXC, GX::CC_KONST, GX::CC_C0);
  CGX::SetTevColorOp(GX::TEVSTAGE0, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, false, GX::TEVPREV);
  CGX::SetTevAlphaIn(GX::TEVSTAGE0, GX::CA_ZERO, GX::CA_TEXA, GX::CA_KONST, GX::CA_A0);
  CGX::SetTevAlphaOp(GX::TEVSTAGE0, GX::TEV_SUB, GX::TB_ZERO, GX::CS_SCALE_1, false, GX::TEVPREV);
  CGX::SetTevKColorSel(GX::TEVSTAGE0, GX::TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX::TEVSTAGE0, GX::TEV_KASEL_K0_A);
  CGX::SetTevOrder(GX::TEVSTAGE1, GX::TEXCOORD1, GX::TEXMAP2, GX::COLOR_NULL);
  CGX::SetTevColorIn(GX::TEVSTAGE1, GX::CC_ZERO, GX::CC_TEXC, GX::CC_KONST, GX::CC_CPREV);
  CGX::SetTevColorOp(GX::TEVSTAGE1, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_2, false, GX::TEVPREV);
  CGX::SetTevAlphaIn(GX::TEVSTAGE1, GX::CA_ZERO, GX::CA_TEXA, GX::CA_KONST, GX::CA_APREV);
  CGX::SetTevAlphaOp(GX::TEVSTAGE1, GX::TEV_SUB, GX::TB_ZERO, GX::CS_SCALE_1, false, GX::TEVPREV);
  CGX::SetTevKColorSel(GX::TEVSTAGE1, GX::TEV_KCSEL_K1);
  CGX::SetTevKAlphaSel(GX::TEVSTAGE1, GX::TEV_KASEL_K1_A);
  CGX::SetTevOrder(GX::TEVSTAGE2, GX::TEXCOORD0, GX::TEXMAP0, GX::COLOR_NULL);
  CGX::SetTevColorIn(GX::TEVSTAGE2, GX::CC_ZERO, GX::CC_TEXC, GX::CC_ONE, GX::CC_CPREV);
  CGX::SetTevColorOp(GX::TEVSTAGE2, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, GX::TEVPREV);
  CGX::SetTevAlphaIn(GX::TEVSTAGE2, GX::CA_TEXA, GX::CA_ZERO, GX::CA_ZERO, GX::CA_APREV);
  CGX::SetTevAlphaOp(GX::TEVSTAGE2, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, GX::TEVPREV);
  CGX::SetTevOrder(GX::TEVSTAGE3, GX::TEXCOORD_NULL, GX::TEXMAP_NULL, GX::COLOR_NULL);
  CGX::SetTevColorIn(GX::TEVSTAGE3, GX::CC_APREV, GX::CC_CPREV, GX::CC_KONST, GX::CC_ZERO);
  CGX::SetTevColorOp(GX::TEVSTAGE3, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, GX::TEVPREV);
  CGX::SetTevAlphaIn(GX::TEVSTAGE3, GX::CA_ZERO, GX::CA_ZERO, GX::CA_ZERO, GX::CA_ZERO);
  CGX::SetTevAlphaOp(GX::TEVSTAGE3, GX::TEV_ADD, GX::TB_ZERO, GX::CS_SCALE_1, true, GX::TEVPREV);
  CGX::SetTevKColorSel(GX::TEVSTAGE3, GX::TEV_KCSEL_K2);
  GXSetTevColorS10(GX::TEVREG0, 0xffa60000ff8e0087u);
  CGX::SetTevKColor(GX::KCOLOR0, zeus::Comp32(0x0000e258));
  CGX::SetTevKColor(GX::KCOLOR1, zeus::Comp32(0xb30000b6));
  CGX::SetTevKColor(GX::KCOLOR2, zeus::Comp32(0xff00ff80));
}
static void MyTHPGXRestore() {
  CGX::SetZMode(true, GX::ALWAYS, false);
  CGX::SetBlendMode(GX::BM_NONE, GX::BL_ONE, GX::BL_ZERO, GX::LO_SET);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGX::SetNumTevStages(1);
  CGX::SetTevOrder(GX::TEVSTAGE0, GX::TEXCOORD0, GX::TEXMAP0, GX::COLOR_NULL);
  CGX::SetAlphaCompare(GX::ALWAYS, 0, GX::AOP_AND, GX::ALWAYS, 0);
}

/* used in the original to look up fixed-point dividends on a
 * MIDI-style volume scale (0-127) -> (n/0x8000) */
static const std::array<u16, 128> StaticVolumeLookup = {
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

/* shared resources */
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

void CMoviePlayer::Initialize() { TjHandle = tjInitDecompress(); }

void CMoviePlayer::Shutdown() {
  tjDestroy(TjHandle);
  TjHandle = nullptr;
}

void CMoviePlayer::THPHeader::swapBig() {
  magic = SBig(magic);
  version = SBig(version);
  maxBufferSize = SBig(maxBufferSize);
  maxAudioSamples = SBig(maxAudioSamples);
  fps = SBig(fps);
  numFrames = SBig(numFrames);
  firstFrameSize = SBig(firstFrameSize);
  dataSize = SBig(dataSize);
  componentDataOffset = SBig(componentDataOffset);
  offsetsDataOffset = SBig(offsetsDataOffset);
  firstFrameOffset = SBig(firstFrameOffset);
  lastFrameOffset = SBig(lastFrameOffset);
}

void CMoviePlayer::THPComponents::swapBig() { numComponents = SBig(numComponents); }

void CMoviePlayer::THPVideoInfo::swapBig() {
  width = SBig(width);
  height = SBig(height);
}

void CMoviePlayer::THPAudioInfo::swapBig() {
  numChannels = SBig(numChannels);
  sampleRate = SBig(sampleRate);
  numSamples = SBig(numSamples);
}

void CMoviePlayer::THPFrameHeader::swapBig() {
  nextSize = SBig(nextSize);
  prevSize = SBig(prevSize);
  imageSize = SBig(imageSize);
  audioSize = SBig(audioSize);
}

void CMoviePlayer::THPAudioFrameHeader::swapBig() {
  channelSize = SBig(channelSize);
  numSamples = SBig(numSamples);
  for (int i = 0; i < 2; ++i) {
    for (int j = 0; j < 8; ++j) {
      channelCoefs[i][j][0] = SBig(channelCoefs[i][j][0]);
      channelCoefs[i][j][1] = SBig(channelCoefs[i][j][1]);
    }
    channelPrevs[i][0] = SBig(channelPrevs[i][0]);
    channelPrevs[i][1] = SBig(channelPrevs[i][1]);
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
: CDvdFile(path), xec_preLoadSeconds(preLoadSeconds), xf4_24_loop(loop), m_deinterlace(deinterlace) {
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

  /* Allocate textures here (rather than at decode time) */
  x80_textures.reserve(3);
  for (int i = 0; i < 3; ++i) {
    CTHPTextureSet& set = x80_textures.emplace_back();
    if (deinterlace) {
      /* metaforce addition: this way interlaced THPs don't look horrible */
      set.Y[0] = aurora::gfx::new_dynamic_texture_2d(x6c_videoInfo.width, x6c_videoInfo.height / 2, 1, GX::TF_I8,
                                                     fmt::format(FMT_STRING("Movie {} Texture Set {} Y[0]"), path, i));
      set.Y[1] = aurora::gfx::new_dynamic_texture_2d(x6c_videoInfo.width, x6c_videoInfo.height / 2, 1, GX::TF_I8,
                                                     fmt::format(FMT_STRING("Movie {} Texture Set {} Y[1]"), path, i));
      set.U = aurora::gfx::new_dynamic_texture_2d(x6c_videoInfo.width / 2, x6c_videoInfo.height / 2, 1, GX::TF_I8,
                                                  fmt::format(FMT_STRING("Movie {} Texture Set {} U"), path, i));
      set.V = aurora::gfx::new_dynamic_texture_2d(x6c_videoInfo.width / 2, x6c_videoInfo.height / 2, 1, GX::TF_I8,
                                                  fmt::format(FMT_STRING("Movie {} Texture Set {} V"), path, i));
    } else {
      /* normal progressive presentation */
      set.Y[0] = aurora::gfx::new_dynamic_texture_2d(x6c_videoInfo.width, x6c_videoInfo.height, 1, GX::TF_I8,
                                                     fmt::format(FMT_STRING("Movie {} Texture Set {} Y"), path, i));
      set.U = aurora::gfx::new_dynamic_texture_2d(x6c_videoInfo.width / 2, x6c_videoInfo.height / 2, 1, GX::TF_I8,
                                                  fmt::format(FMT_STRING("Movie {} Texture Set {} U"), path, i));
      set.V = aurora::gfx::new_dynamic_texture_2d(x6c_videoInfo.width / 2, x6c_videoInfo.height / 2, 1, GX::TF_I8,
                                                  fmt::format(FMT_STRING("Movie {} Texture Set {} V"), path, i));
    }
    if (xf4_25_hasAudio)
      set.audioBuf.reset(new s16[x28_thpHead.maxAudioSamples * 2]);
  }

  /* Temporary planar YUV decode buffer, resulting planes copied to Boo */
  m_yuvBuf.reset(new uint8_t[tjBufSizeYUV(x6c_videoInfo.width, x6c_videoInfo.height, TJ_420)]);

  /* Schedule initial read */
  PostDVDReadRequestIfNeeded();

  m_hpad = 0.5f;
  m_vpad = 0.5f;
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
      /* metaforce addition: failsafe for buffer overrun */
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

    /* metaforce addition: mix samples with `in` or no mix */
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

bool CMoviePlayer::DrawVideo() {
  // TODO
  // if (!xa0_bufferQueue.empty()) {
  //   return false;
  // }

  g_Renderer->SetDepthReadWrite(false, false);
  g_Renderer->SetViewportOrtho(false, -4096.f, 4096.f);

  const s32 vpHeight = CGraphics::GetViewportHeight();
  const s32 vpWidth = CGraphics::GetViewportWidth();
  const s32 vpTop = CGraphics::GetViewportTop();
  const s32 vpLeft = CGraphics::GetViewportLeft();
  const s32 vidWidth = x6c_videoInfo.width;
  const s32 vidHeight = x6c_videoInfo.height;
  const s32 centerX = (vidWidth - vpWidth) / 2;
  const s32 centerY = (vidHeight - vpHeight) / 2;
  const s32 vl = vpLeft - centerX;
  const s32 vr = vpLeft + vpWidth + centerX;
  const s32 vb = vpTop + vpHeight + centerY;
  const s32 vt = vpTop - centerY;
  aurora::Vec3<float> v1;
  aurora::Vec3<float> v2;
  aurora::Vec3<float> v3;
  aurora::Vec3<float> v4;
  v1.x = vl;
  v1.y = 0.0;
  v1.z = vb;
  v2.x = vr;
  v2.y = 0.0;
  v2.z = vb;
  v3.x = vl;
  v3.y = 0.0;
  v3.z = vt;
  v4.x = vr;
  v4.y = 0.0;
  v4.z = vt;

  DrawFrame(v1, v2, v3, v4);
  return true;
}

void CMoviePlayer::DrawFrame(const zeus::CVector3f& v1, const zeus::CVector3f& v2, const zeus::CVector3f& v3,
                             const zeus::CVector3f& v4) {
  if (xd0_drawTexSlot == UINT32_MAX || !GetIsFullyCached()) {
    return;
  }
  SCOPED_GRAPHICS_DEBUG_GROUP("CMoviePlayer::DrawFrame", zeus::skYellow);

  CGraphics::SetUseVideoFilter(xf4_26_fieldFlip);

  /* Correct movie aspect ratio */
  float hPad, vPad;
  if (CGraphics::GetViewportAspect() >= 1.78f) {
    hPad = 1.78f / CGraphics::GetViewportAspect();
    vPad = 1.78f / 1.33f;
  } else {
    hPad = 1.f;
    vPad = CGraphics::GetViewportAspect() / 1.33f;
  }

  /* draw appropriate field */
  CTHPTextureSet& tex = x80_textures[xd0_drawTexSlot];
  aurora::gfx::queue_movie_player(tex.Y[m_deinterlace ? (xfc_fieldIndex != 0) : 0], tex.U, tex.V, hPad, vPad);

//  MyTHPGXYuv2RgbSetup(CGraphics::g_LastFrameUsedAbove, xf4_26_fieldFlip);
//  uintptr_t planeSize = x6c_videoInfo.width * x6c_videoInfo.height;
//  uintptr_t planeSizeQuarter = planeSize / 4;
//  MyTHPYuv2RgbTextureSetup(m_yuvBuf.get(), m_yuvBuf.get() + planeSize, m_yuvBuf.get() + planeSize + planeSizeQuarter,
//                           x6c_videoInfo.width, x6c_videoInfo.height);
//
//  CGX::Begin(GX::TRIANGLEFAN, GX::VTXFMT7, 4);
//  GXPosition3f32(v1);
//  GXTexCoord2f32(0.f, 0.f);
//  GXPosition3f32(v3);
//  GXTexCoord2f32(0.f, 1.f);
//  GXPosition3f32(v4);
//  GXTexCoord2f32(1.f, 1.f);
//  GXPosition3f32(v2);
//  GXTexCoord2f32(1.f, 0.f);
//  CGX::End();
//  MyTHPGXRestore();

  /* ensure second field is being displayed by VI to signal advance
   * (faked in metaforce with continuous xor) */
  if (xfc_fieldIndex == 0 && CGraphics::g_LastFrameUsedAbove)
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
        auto buffer = std::make_unique<u8[]>(planeSizeHalf);
        for (unsigned y = 0; y < x6c_videoInfo.height / 2; ++y) {
          memcpy(buffer.get() + x6c_videoInfo.width * y, m_yuvBuf.get() + x6c_videoInfo.width * (y * 2),
                 x6c_videoInfo.width);
        }
        aurora::gfx::write_texture(*tex.Y[0], {buffer.get(), planeSizeHalf});
        for (unsigned y = 0; y < x6c_videoInfo.height / 2; ++y) {
          memcpy(buffer.get() + x6c_videoInfo.width * y, m_yuvBuf.get() + x6c_videoInfo.width * (y * 2 + 1),
                 x6c_videoInfo.width);
        }
        aurora::gfx::write_texture(*tex.Y[1], {buffer.get(), planeSizeHalf});
      } else {
        /* Direct planar load */
        aurora::gfx::write_texture(*tex.Y[0], {m_yuvBuf.get(), planeSize});
      }
      aurora::gfx::write_texture(*tex.U, {m_yuvBuf.get() + planeSize, planeSizeQuarter});
      aurora::gfx::write_texture(*tex.V, {m_yuvBuf.get() + planeSize + planeSizeQuarter, planeSizeQuarter});

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
  xb0_nextReadSize = SBig(frameHeader->nextSize);
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

} // namespace metaforce
