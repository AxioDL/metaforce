#include "Kyoto/Math/CVector3f.hpp"
#include "Kyoto/Basics/CBasics.hpp"

#include <Kyoto/Graphics/CMoviePlayer.hpp>

#include "Kyoto/Audio/CAudioSys.hpp"
#include "Kyoto/Audio/CStaticAudioPlayer.hpp"
#include "Kyoto/Graphics/CGraphics.hpp"
#include "Kyoto/Math/CMath.hpp"
#include "dolphin/base/PPCArch.h"
#include "dolphin/ai.h"
#include "dolphin/gx/GXVert.h"
#include "dolphin/os.h"
#include <Kyoto/Graphics/CGX.hpp>
#include <Kyoto/Graphics/CTexture.hpp>
#include <rstl/math.hpp>
#include <string.h>

#include "dolphin/gx/GXGeometry.h"
#include "dolphin/gx/GXTev.h"
#include "dolphin/gx/GXTexture.h"
#include "dolphin/thp/THPAudio.h"
#include "dolphin/thp/THPPlayer.h"
#include "dolphin/thp/THPVideoDecode.h"

static int sNumReferences = 0;
static CMoviePlayer* sAudioPlayer;
static const short* curAudioBuffer;
static int soundBufferIndex;
ATTRIBUTE_ALIGN_DECL(32, static short soundBuffer[2][320]);
static bool sAudioEnabled = true;
static uchar sSfxVolume = 127;

struct CMoviePlayer::SIndexLoad {
  rstl::single_ptr< CDvdRequest > x0_headerRequest;
  rstl::single_ptr< CDvdRequest > x4_videoRequest;
  rstl::single_ptr< CDvdRequest > x8_audioRequest;
  rstl::single_ptr< uchar > xc_buffer;
  int x10_state;

  SIndexLoad()
  : xc_buffer(static_cast< uchar* >(CMemory::Alloc(64, IAllocator::kHI_RoundUpLen)))
  , x10_state(0) {}
};
const unsigned char skInterlacePattern[32] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};

static void MyTHPGXRestore() {
  CGX::SetZMode(TRUE, GX_ALWAYS, FALSE);
  CGX::SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_SET);
  CGX::SetNumTexGens(1);
  CGX::SetNumChans(0);
  CGX::SetNumTevStages(1);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetAlphaCompare(GX_ALWAYS, 0, GX_AOP_AND, GX_ALWAYS, 0);
}

static void MyTHPGXYuv2RgbSetup(bool field, bool deinterlace) {
  GXVtxDescList attr[3] = {
      {GX_VA_POS, GX_DIRECT},
      {GX_VA_TEX0, GX_DIRECT},
      {GX_VA_NULL, GX_NONE},
  };
  CGX::SetZMode(TRUE, GX_ALWAYS, FALSE);
  CGX::SetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_CLEAR);
  CGX::SetNumChans(0);
  CGX::SetTexCoordGen(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, FALSE, GX_PTIDENTITY);
  CGX::SetTexCoordGen(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, FALSE, GX_PTIDENTITY);

  if (deinterlace) {
    CGX::SetNumTexGens(2);
    CGX::SetNumTevStages(4);
  } else {
    CGX::SetNumTexGens(3);
    CGX::SetTexCoordGen(GX_TEXCOORD2, GX_TG_MTX2x4, GX_TG_POS, GX_TEXMTX0, FALSE, GX_PTIDENTITY);

    float n = field ? 0.25f : 0.f;
    float mtx[8] = {0.125f, 0.f, 0.f, 0.f, 0.f, 0.f, 0.25f, n};
    GXLoadTexMtxImm(reinterpret_cast< MtxPtr >(mtx), GX_TEXMTX0, GX_MTX2x4);
    GXTexObj obj;
    GXInitTexObj(&obj, skInterlacePattern, 8, 4, GX_TF_I8, GX_REPEAT, GX_REPEAT, FALSE);
    GXInitTexObjLOD(&obj, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, FALSE, FALSE, GX_ANISO_1);
    GXLoadTexObj(&obj, GX_TEXMAP3);
    CTexture::InvalidateTexmap(GX_TEXMAP3);
    CGX::SetTevOrder(GX_TEVSTAGE4, GX_TEXCOORD2, GX_TEXMAP3, GX_COLOR_NULL);
    CGX::SetStandardTevColorAlphaOp(GX_TEVSTAGE4);
    CGX::SetTevColorIn(GX_TEVSTAGE4, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_CPREV);
    CGX::SetTevAlphaIn(GX_TEVSTAGE4, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    CGX::SetAlphaCompare(GX_LESS, 128, GX_AOP_AND, GX_ALWAYS, 0);
    CGX::SetNumTevStages(5);
  }

  CGX::SetVtxDescv(attr);
  GXSetColorUpdate(TRUE);
  GXSetAlphaUpdate(FALSE);
  GXInvalidateTexAll();
  GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_POS, GX_CLR_RGBA, GX_F32, 0);
  GXSetVtxAttrFmt(GX_VTXFMT7, GX_VA_TEX0, GX_CLR_RGBA, GX_RGBX8, 0);
  CGX::SetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_C0);
  CGX::SetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, false, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_A0);
  CGX::SetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, false, GX_TEVPREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE0, GX_TEV_KCSEL_K0);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE0, GX_TEV_KASEL_K0_A);
  CGX::SetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE1, GX_CC_ZERO, GX_CC_TEXC, GX_CC_KONST, GX_CC_CPREV);
  CGX::SetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_2, false, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_TEXA, GX_CA_KONST, GX_CA_APREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_SUB, GX_TB_ZERO, GX_CS_SCALE_1, false, GX_TEVPREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE1, GX_TEV_KCSEL_K1);
  CGX::SetTevKAlphaSel(GX_TEVSTAGE1, GX_TEV_KASEL_K1_A);
  CGX::SetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_TEXC, GX_CC_ONE, GX_CC_CPREV);
  CGX::SetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE2, GX_CA_TEXA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_APREV);
  CGX::SetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevOrder(GX_TEVSTAGE3, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR_NULL);
  CGX::SetTevColorIn(GX_TEVSTAGE3, GX_CC_APREV, GX_CC_CPREV, GX_CC_KONST, GX_CC_ZERO);
  CGX::SetTevColorOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevAlphaIn(GX_TEVSTAGE3, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
  CGX::SetTevAlphaOp(GX_TEVSTAGE3, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, true, GX_TEVPREV);
  CGX::SetTevKColorSel(GX_TEVSTAGE3, GX_TEV_KCSEL_K2);
  const GXColorS10 color = {-90, 0, -114, 135};
  GXSetTevColorS10(GX_TEVREG0, color);
  const GXColor kColor0 = {0, 0, 226, 88};
  CGX::SetTevKColor(GX_KCOLOR0, kColor0);
  const GXColor kColor1 = {179, 0, 0, 182};
  CGX::SetTevKColor(GX_KCOLOR1, kColor1);
  const GXColor kColor2 = {255, 0, 255, 128};
  CGX::SetTevKColor(GX_KCOLOR2, kColor2);
}

static void MyTHPYuv2RgbTextureSetup(void* y, void* u, void* v, ushort width, ushort height) {
  GXTexObj yTex;
  GXInitTexObj(&yTex, y, width, height, GX_TF_I8, GX_CLAMP, GX_CLAMP, FALSE);
  GXInitTexObjLOD(&yTex, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, FALSE, FALSE, GX_ANISO_1);
  GXLoadTexObj(&yTex, GX_TEXMAP0);

  GXTexObj uTex;
  GXInitTexObj(&uTex, u, width / 2, height / 2, GX_TF_I8, GX_CLAMP, GX_CLAMP, FALSE);
  GXInitTexObjLOD(&uTex, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, FALSE, FALSE, GX_ANISO_1);
  GXLoadTexObj(&uTex, GX_TEXMAP1);

  GXTexObj vTex;
  GXInitTexObj(&vTex, v, static_cast< s16 >(width / 2), height / 2, GX_TF_I8, GX_CLAMP, GX_CLAMP,
               FALSE);
  GXInitTexObjLOD(&vTex, GX_NEAR, GX_NEAR, 0.f, 0.f, 0.f, FALSE, FALSE, GX_ANISO_1);
  GXLoadTexObj(&vTex, GX_TEXMAP2);

  CTexture::InvalidateTexmap(GX_TEXMAP0);
  CTexture::InvalidateTexmap(GX_TEXMAP1);
  CTexture::InvalidateTexmap(GX_TEXMAP2);
}

CMoviePlayer::CMoviePlayer(const char* path, const float preLoadSeconds, const bool loop,
                           const bool deinterlace)
: x0_dvdFile(path)
, xac_indexLoad(rs_new SIndexLoad)
, xb0_nextReadSize(0)
, xb4_nextReadOff(0)
, xb8_readSizeWrapped(0)
, xbc_readOffWrapped(0)
, xc0_curLoadFrame(0)
, xc4_requestFrameWrapped(0)
, xc8_curFrame(0)
, xcc_decodedTexSlot(0)
, xd0_drawTexSlot(-1)
, xd4_audioSlot(-1)
, xd8_decodedTexCount(0)
, xdc_frameRem(0.f)
, xe0_playMode(kPM_Playing)
, xe4_totalSeconds(0.f)
, xe8_curSeconds(0.f)
, xec_preLoadSeconds(preLoadSeconds)
, xf0_preLoadFrames(0)
, xf4_24_loop(loop)
, xf4_25_deinterlace(deinterlace)
, xf4_26_hasAudio(false)
, xf4_27_fieldFlip(false)
, xf8_cachedBytes(0)
, xfc_fieldIndex(0) {

  static bool sThpInitialized = false;
  if (!sThpInitialized) {
    sThpInitialized = true;
    THPInit();
  }
  ++sNumReferences;
  VerifyCallbackStatus();
  xac_indexLoad->x0_headerRequest = x0_dvdFile.SyncRead(xac_indexLoad->xc_buffer.get(), 64);
}

bool CMoviePlayer::PumpIndexLoad() {
  if (xac_indexLoad.null()) {
    return false;
  }
  uchar* const buffer = xac_indexLoad->xc_buffer.get();
  bool hasVideo;
  bool hasAudio;
  switch (xac_indexLoad->x10_state) {
  case 0:
    if (xac_indexLoad->x0_headerRequest->IsComplete()) {
      memcpy(&x28_header, buffer, sizeof(THPHeader));
      x28_header.mVersion = CBasics::SwapBytes(static_cast< uint >(x28_header.mVersion));
      x28_header.mBufferSize = CBasics::SwapBytes(static_cast< uint >(x28_header.mBufferSize));
      x28_header.mAudioMaxSamples =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mAudioMaxSamples));
      x28_header.mFrameRate = CBasics::SwapBytes(x28_header.mFrameRate);
      x28_header.mNumFrames = CBasics::SwapBytes(static_cast< uint >(x28_header.mNumFrames));
      x28_header.mFirstFrameSize =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mFirstFrameSize));
      x28_header.mMovieDataSize =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mMovieDataSize));
      x28_header.mCompInfoDataOffsets =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mCompInfoDataOffsets));
      x28_header.mOffsetDataOffsets =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mOffsetDataOffsets));
      x28_header.mMovieDataOffsets =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mMovieDataOffsets));
      x28_header.mFinalFrameDataOffsets =
          CBasics::SwapBytes(static_cast< uint >(x28_header.mFinalFrameDataOffsets));
      xac_indexLoad->x0_headerRequest =
          x0_dvdFile.AsyncSeekRead(buffer, 32, kSO_Begin, x28_header.mCompInfoDataOffsets);
      ++xac_indexLoad->x10_state;
    } else {
      return true;
    }
  case 1: {
    if (xac_indexLoad->x0_headerRequest->IsComplete()) {
      memcpy(&x58_thpComponents, buffer, sizeof(THPFrameCompInfo));
      x58_thpComponents.mNumComponents =
          CBasics::SwapBytes(static_cast< uint >(x58_thpComponents.mNumComponents));
      xac_indexLoad->x0_headerRequest = nullptr;
      uchar* audioBuffer = buffer + 32;
      int offset = x28_header.mCompInfoDataOffsets + sizeof(THPFrameCompInfo);
      for (uint i = 0; i < x58_thpComponents.mNumComponents; ++i) {
        switch (x58_thpComponents.mFrameComp[i]) {
        case 0:
          xac_indexLoad->x4_videoRequest = x0_dvdFile.AsyncSeekRead(buffer, 32, kSO_Begin, offset);
          offset += sizeof(THPVideoInfoOld);
          break;
        case 1:
          xac_indexLoad->x8_audioRequest =
              x0_dvdFile.AsyncSeekRead(audioBuffer, 32, kSO_Begin, offset);
          offset += sizeof(THPAudioInfoOld);
          xf4_26_hasAudio = true;
          break;
        }
      }
      ++xac_indexLoad->x10_state;
    } else {
      return true;
    }
  }
  case 2: {
    bool complete = true;
    hasVideo = xac_indexLoad->x4_videoRequest.get() != nullptr;
    hasAudio = xac_indexLoad->x8_audioRequest.get() != nullptr;
    if (hasVideo && !xac_indexLoad->x4_videoRequest->IsComplete()) {
      complete = false;
    }
    if (hasAudio && !xac_indexLoad->x8_audioRequest->IsComplete()) {
      complete = false;
    }
    if (!complete) {
      return true;
    }
    if (hasVideo) {
      memcpy(&x6c_videoInfo, buffer, sizeof(THPVideoInfoOld));
      x6c_videoInfo.mXSize = CBasics::SwapBytes(static_cast< uint >(x6c_videoInfo.mXSize));
      x6c_videoInfo.mYSize = CBasics::SwapBytes(static_cast< uint >(x6c_videoInfo.mYSize));
    }
    if (hasAudio) {
      memcpy(&x74_audioInfo, buffer + 32, sizeof(THPAudioInfoOld));
      x74_audioInfo.mSndChannels =
          CBasics::SwapBytes(static_cast< uint >(x74_audioInfo.mSndChannels));
      x74_audioInfo.mSndFrequency =
          CBasics::SwapBytes(static_cast< uint >(x74_audioInfo.mSndFrequency));
      x74_audioInfo.mSndNumSamples =
          CBasics::SwapBytes(static_cast< uint >(x74_audioInfo.mSndNumSamples));
    }
  }
  }

  xac_indexLoad = nullptr;
  x80_textures.reserve(3);
  xb4_nextReadOff = x28_header.mMovieDataOffsets;
  xb0_nextReadSize = x28_header.mFirstFrameSize;
  xb8_readSizeWrapped = x28_header.mFirstFrameSize;
  xbc_readOffWrapped = x28_header.mMovieDataOffsets;
  xe4_totalSeconds = x28_header.mNumFrames / x28_header.mFrameRate;
  if (xec_preLoadSeconds < 0.f) {
    xec_preLoadSeconds = xe4_totalSeconds;
    xf0_preLoadFrames = x28_header.mNumFrames;
  } else if (xec_preLoadSeconds > 0.f) {
    xf0_preLoadFrames =
        rstl::min_val(static_cast< uint >(x28_header.mNumFrames),
                      static_cast< uint >(xec_preLoadSeconds * x28_header.mFrameRate));
    xec_preLoadSeconds = rstl::min_val(xec_preLoadSeconds, xe4_totalSeconds);
  }
  if (xf0_preLoadFrames > 0) {
    x9c_requestQueue.reserve(xf0_preLoadFrames);
  }
  PostDVDReadRequestIfNeeded();
  return false;
}

CMoviePlayer::~CMoviePlayer() {
  --sNumReferences;
  VerifyCallbackStatus();
  if (sAudioPlayer == this) {
    sAudioPlayer = nullptr;
  }
}

void CMoviePlayer::InitializeTextures() {
  const uint ySize = OSRoundUp32B(x6c_videoInfo.mXSize * x6c_videoInfo.mYSize);
  const uint uvSize = OSRoundUp32B(x6c_videoInfo.mXSize * x6c_videoInfo.mYSize / 4);
  const uint audioSize = x28_header.mAudioMaxSamples * 4;
  for (int i = 0; i < x80_textures.capacity(); ++i) {
    void* y = CMemory::Alloc(ySize, IAllocator::kHI_RoundUpLen);
    void* u = CMemory::Alloc(uvSize, IAllocator::kHI_RoundUpLen);
    void* v = CMemory::Alloc(uvSize, IAllocator::kHI_RoundUpLen);
    void* audio = CMemory::Alloc(audioSize, IAllocator::kHI_RoundUpLen);
    DCFlushRangeNoSync(y, ySize);
    DCFlushRangeNoSync(u, uvSize);
    DCFlushRangeNoSync(v, uvSize);
    DCFlushRangeNoSync(audio, audioSize);
    x80_textures.push_back(CTHPTextureSet(y, u, v, audio));
  }

  PPCSync();
  xcc_decodedTexSlot = 0;
  xd0_drawTexSlot = -1;
  xd4_audioSlot = -1;
}

void CMoviePlayer::PostDVDReadRequestIfNeeded() {
  if (xc0_curLoadFrame < x28_header.mNumFrames) {
    x90_requestBuffer = rstl::auto_ptr< uchar >(
        static_cast< uchar* >(CMemory::Alloc(xb0_nextReadSize, IAllocator::kHI_RoundUpLen)));
    x98_request = x0_dvdFile.AsyncSeekRead(x90_requestBuffer.get(), xb0_nextReadSize, kSO_Begin,
                                           xb4_nextReadOff);
  }
}

void CMoviePlayer::ReadCompleted() {
  x98_request = nullptr;
  if (xc0_curLoadFrame == x9c_requestQueue.size() && xf0_preLoadFrames > xc0_curLoadFrame) {
    x9c_requestQueue.push_back(x90_requestBuffer);
    xf8_cachedBytes += xb0_nextReadSize;
  }
  xb4_nextReadOff += xb0_nextReadSize;
  xb0_nextReadSize = CBasics::SwapBytes(*reinterpret_cast< const uint* >(x90_requestBuffer.get()));
  ++xc0_curLoadFrame;
  if (xc0_curLoadFrame == xf0_preLoadFrames) {
    if (xc0_curLoadFrame == x28_header.mNumFrames) {
      xb8_readSizeWrapped = x28_header.mFirstFrameSize;
      xbc_readOffWrapped = x28_header.mMovieDataOffsets;
    } else {
      xb8_readSizeWrapped = xb0_nextReadSize;
      xbc_readOffWrapped = xb4_nextReadOff;
    }
  }
  if (xc0_curLoadFrame >= x28_header.mNumFrames && xf4_24_loop) {
    xb4_nextReadOff = xbc_readOffWrapped;
    xb0_nextReadSize = xb8_readSizeWrapped;
    xc0_curLoadFrame = xf0_preLoadFrames;
  }
}

void CMoviePlayer::DecodeFromRead(const void* ptr) {
  uchar work[4096 + 32];
  void* alignedWork = reinterpret_cast< void* >((reinterpret_cast< uintptr_t >(work) + 31) & ~31);
  if (x80_textures.empty()) {
    InitializeTextures();
  }
  CTHPTextureSet& texture = x80_textures[xcc_decodedTexSlot];
  const uint* sizes = static_cast< const uint* >(ptr) + 2;
  const uchar* dataStart =
      static_cast< const uchar* >(ptr) + 8 + x58_thpComponents.mNumComponents * 4;
  uint offset = 0;
  texture.SetAudioSamplesConsumed(0);
  texture.SetAudioSamples(0);
  for (uint i = 0; i < x58_thpComponents.mNumComponents; ++i) {
    const uchar* data = dataStart + offset;
    if (x58_thpComponents.mFrameComp[i] == 0) {
      THPVideoDecode(const_cast< uchar* >(data), texture.Y(), texture.U(), texture.V(),
                     alignedWork);
    } else if (x58_thpComponents.mFrameComp[i] == 1) {
      const uint samples =
          THPAudioDecode(static_cast< short* >(texture.Audio()), const_cast< uchar* >(data), 0);
      const BOOL interrupts = OSDisableInterrupts();
      texture.SetAudioSamples(samples);
      texture.SetAudioSamplesConsumed(0);
      OSRestoreInterrupts(interrupts);
    }
    offset += CBasics::SwapBytes(*sizes++);
  }
  if (++xcc_decodedTexSlot == x80_textures.size()) {
    xcc_decodedTexSlot = 0;
  }
}

void CMoviePlayer::Update(float dt) {
  if (xc0_curLoadFrame < xf0_preLoadFrames) {
    if (!x98_request.null() && x98_request->IsComplete()) {
      ReadCompleted();
      if (xc0_curLoadFrame >= x9c_requestQueue.size() && xc0_curLoadFrame < xf0_preLoadFrames &&
          x9c_requestQueue.size() < x28_header.mNumFrames) {
        PostDVDReadRequestIfNeeded();
      }
    }
  } else if (!x98_request.null()) {
    bool canDecode = false;
    if (xc4_requestFrameWrapped >= x9c_requestQueue.size() &&
        xc0_curLoadFrame >= x9c_requestQueue.size()) {
      canDecode = true;
    }
    if (x98_request->IsComplete() && xd8_decodedTexCount < 2 && canDecode) {
      DecodeFromRead(x90_requestBuffer.get());
      ReadCompleted();
      PostDVDReadRequestIfNeeded();
      ++xd8_decodedTexCount;
      ++xc4_requestFrameWrapped;
      if (xc4_requestFrameWrapped >= x28_header.mNumFrames && xf4_24_loop) {
        xc4_requestFrameWrapped = 0;
      }
    }
  }
  if (x98_request.null() && xe0_playMode == kPM_Playing &&
      x9c_requestQueue.size() < x28_header.mNumFrames) {
    PostDVDReadRequestIfNeeded();
  }
  if (xd8_decodedTexCount < 2 && xe0_playMode == kPM_Playing &&
      xc4_requestFrameWrapped < xf0_preLoadFrames) {
    const int frame = rstl::min_val(xc4_requestFrameWrapped, x9c_requestQueue.size() - 1);
    if (frame == -1) {
      return;
    }
    DecodeFromRead(x9c_requestQueue[frame].get());
    ++xd8_decodedTexCount;
    ++xc4_requestFrameWrapped;
    if (xc4_requestFrameWrapped >= x28_header.mNumFrames && xf4_24_loop) {
      xc4_requestFrameWrapped = 0;
    }
  }
  if (xd8_decodedTexCount > 0 && xe0_playMode == kPM_Playing) {
    xe8_curSeconds += dt;
    if (xf4_24_loop) {
      xe8_curSeconds = CMath::ModF(xe8_curSeconds, xe4_totalSeconds);
    } else {
      xe8_curSeconds = rstl::min_val(xe8_curSeconds, xe4_totalSeconds);
    }
    float remainder = xdc_frameRem - dt;
    const float frameDt = 1.f / x28_header.mFrameRate;
    if (remainder <= 0.f) {
      if (!xf4_27_fieldFlip) {
        if (++xd0_drawTexSlot >= x80_textures.size()) {
          xd0_drawTexSlot = 0;
        }
        const BOOL interrupts = OSDisableInterrupts();
        if (xd4_audioSlot == -1) {
          xd4_audioSlot = 0;
        }
        OSRestoreInterrupts(interrupts);
        --xd8_decodedTexCount;
        ++xc8_curFrame;
        if (xc8_curFrame == x28_header.mNumFrames && xf4_24_loop) {
          xc8_curFrame = 0;
        }
        remainder += frameDt;
        xfc_fieldIndex = 0;
      } else {
        remainder += dt;
        xf4_27_fieldFlip = false;
      }
    }
    xdc_frameRem = remainder;
  }
}

void CMoviePlayer::DrawFrame(const CVector3f& v1, const CVector3f& v2, const CVector3f& v3,
                             const CVector3f& v4) {
  if (xd0_drawTexSlot == -1) {
    return;
  }
  CGraphics::SetUseVideoFilter(xf4_25_deinterlace);
  const BOOL interrupts = OSDisableInterrupts();
  sAudioPlayer = this;
  OSRestoreInterrupts(interrupts);
  CTHPTextureSet& texture = x80_textures[xd0_drawTexSlot];
  const bool field = CGraphics::GetDolphinLastFrameAbove();
  const bool deinterlace = xf4_25_deinterlace;
  MyTHPGXYuv2RgbSetup(field, deinterlace);
  MyTHPYuv2RgbTextureSetup(texture.Y(), texture.U(), texture.V(), x6c_videoInfo.mXSize,
                           x6c_videoInfo.mYSize);
  CGX::Begin(GX_TRIANGLEFAN, GX_VTXFMT7, 4);
  GXPosition3f32(v1.GetX(), v1.GetY(), v1.GetZ());
  GXTexCoord2u16(0, 0);
  GXPosition3f32(v3.GetX(), v3.GetY(), v3.GetZ());
  GXTexCoord2u16(0, 1);
  GXPosition3f32(v4.GetX(), v4.GetY(), v4.GetZ());
  GXTexCoord2u16(1, 1);
  GXPosition3f32(v2.GetX(), v2.GetY(), v2.GetZ());
  GXTexCoord2u16(1, 0);
  CGX::End();
  MyTHPGXRestore();
  if (xfc_fieldIndex == 0 && field) {
    xf4_27_fieldFlip = true;
  }
  ++xfc_fieldIndex;
}

void CMoviePlayer::SetPlayMode(const EPlayMode mode) { xe0_playMode = mode; }

float CMoviePlayer::GetTotalSeconds() const { return xe4_totalSeconds; }

float CMoviePlayer::GetPlayedSeconds() const { return xe8_curSeconds + xdc_frameRem; }

bool CMoviePlayer::GetIsFullyCached() const { return x9c_requestQueue.size() >= xf0_preLoadFrames; }

bool CMoviePlayer::GetIsMovieFinishedPlaying() const {
  return !xf4_24_loop && xc8_curFrame == x28_header.mNumFrames;
}

void CMoviePlayer::Rewind() {
  if (!x98_request.null()) {
    x98_request->PostCancelRequest();
    x98_request = nullptr;
  }

  x90_requestBuffer = rstl::auto_ptr< uchar >(nullptr);
  xb0_nextReadSize = x28_header.mFirstFrameSize;
  xb4_nextReadOff = x28_header.mMovieDataOffsets;
  xb8_readSizeWrapped = x28_header.mFirstFrameSize;
  xbc_readOffWrapped = x28_header.mMovieDataOffsets;
  xc0_curLoadFrame = 0;
  xc4_requestFrameWrapped = 0;
  xc8_curFrame = 0;
  xcc_decodedTexSlot = 0;
  xd0_drawTexSlot = -1;
  xd4_audioSlot = -1;
  xd8_decodedTexCount = 0;
  xdc_frameRem = 0.f;
  xe8_curSeconds = 0.f;
  x80_textures.clear();
}

void CMoviePlayer::StaticMyAudioCallback() {
  if (sAudioPlayer != nullptr && sAudioPlayer->xf4_26_hasAudio) {
    curAudioBuffer = static_cast< const short* >(OSPhysicalToCached(AIGetDMAStartAddr()));
    soundBufferIndex ^= 1;
    short* buffer = soundBuffer[soundBufferIndex];
    AIInitDMA(reinterpret_cast< uintptr_t >(buffer), sizeof(soundBuffer[0]));
    const BOOL interrupts = OSEnableInterrupts();
    if (curAudioBuffer != nullptr) {
      DCInvalidateRange(const_cast< short* >(curAudioBuffer), sizeof(soundBuffer[0]));
    }
    sAudioPlayer->MixAudio(buffer, curAudioBuffer, 160);
    DCFlushRange(buffer, sizeof(soundBuffer[0]));
    OSRestoreInterrupts(interrupts);
  }
}

void CMoviePlayer::MixAudio(short* out, const short* in, unsigned long samples) {
  const short* input = in;
  if (xd4_audioSlot == -1) {
    if (in != nullptr) {
      memcpy(out, in, samples * 4);
    } else {
      memset(out, 0, samples * 4);
    }
    return;
  }
  const uchar volume = rstl::min_val(127, sSfxVolume * 100 >> 7);
  const ushort attenuation =
      sAudioEnabled ? static_cast< ushort >(CAudioSys::GetScaledVolume(volume)) : 0;
  for (int frame = 0; samples != 0 && frame < 3; ++frame) {
    CTHPTextureSet& texture = x80_textures[xd4_audioSlot];
    uint count = texture.GetAudioSamples() - texture.GetAudioSamplesConsumed();
    if (count > samples) {
      count = samples;
    } else {
      if (++xd4_audioSlot == x80_textures.size()) {
        xd4_audioSlot = 0;
      }
    }
    const uint consumed = texture.GetAudioSamplesConsumed();
    const short* audio = static_cast< short* >(texture.Audio()) + consumed * 2;
    texture.SetAudioSamplesConsumed(count + consumed);
    if (in != nullptr) {
      for (uint i = 0; i < count * 2; ++i) {
        int sample = *input + ((attenuation * *audio) >> 15);
        if (sample < -32768) {
          sample = -32768;
        } else if (sample > 32767) {
          sample = 32767;
        }
        *out = sample;
        ++out;
        ++input;
        ++audio;
      }
    } else {
      for (uint i = 0; i < count * 2; ++i) {
        int sample = (attenuation * *audio) >> 15;
        if (sample < -32768) {
          sample = -32768;
        } else if (sample > 32767) {
          sample = 32767;
        }
        *out = sample;
        ++out;
        ++audio;
      }
    }
    samples -= count;
  }
  if (samples != 0) {
    if (in != nullptr) {
      memcpy(out, input, samples * 4);
    } else {
      memset(out, 0, samples * 4);
    }
  }
}

void CMoviePlayer::VerifyCallbackStatus() {
  if (sNumReferences > 0) {
    CStaticAudioPlayer::RunDMACallback(StaticMyAudioCallback);
  } else {
    CStaticAudioPlayer::CancelDMACallback(StaticMyAudioCallback);
  }
}

uint CMoviePlayer::GetWidth() const { return x6c_videoInfo.mXSize; }

uint CMoviePlayer::GetHeight() const { return x6c_videoInfo.mYSize; }

void CMoviePlayer::SetAudioEnabled(bool enabled) { sAudioEnabled = enabled; }

bool CMoviePlayer::GetAudioEnabled() { return sAudioEnabled; }

void CMoviePlayer::SetSfxVolume(uchar volume) { sSfxVolume = rstl::min_val(uchar(127), volume); }
