#include "Kyoto/Basics/CBasics.hpp"
#include "Kyoto/Math/CVector3f.hpp"

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

#include "dolphin/gx.h"
#include "dolphin/thp.h"
#if defined(TARGET_PC)
#include "Metaforce/Audio.hpp"
#include "Metaforce/Display.hpp"
#include <algorithm>
#endif

static int sNumReferences = 0;
static CMoviePlayer* sAudioPlayer;
#if !defined(TARGET_PC)
static const short* curAudioBuffer;
static int soundBufferIndex;
ATTRIBUTE_ALIGN_DECL(32, static short soundBuffer[2][320]);
#endif
static bool sAudioEnabled = true;
static uchar sSfxVolume = 127;

struct CMoviePlayer::SIndexLoad {
  rstl::single_ptr< CDvdRequest > mHeaderRequest;
  rstl::single_ptr< CDvdRequest > mVideoRequest;
  rstl::single_ptr< CDvdRequest > mAudioRequest;
  rstl::single_ptr< uchar > mBuffer;
  int mState;

  SIndexLoad()
  : mBuffer(static_cast< uchar* >(CMemory::Alloc(64, IAllocator::kHI_RoundUpLen)))
  , mState(0) {}
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
#if defined(TARGET_PC)
    GXDestroyTexObj(&obj);
#endif
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
#if defined(TARGET_PC)
  GXDestroyTexObj(&yTex);
  GXDestroyTexObj(&uTex);
  GXDestroyTexObj(&vTex);
#endif

  CTexture::InvalidateTexmap(GX_TEXMAP0);
  CTexture::InvalidateTexmap(GX_TEXMAP1);
  CTexture::InvalidateTexmap(GX_TEXMAP2);
}

CMoviePlayer::CMoviePlayer(const char* path, const float preLoadSeconds, const bool loop,
                           const bool deinterlace)
: mDvdFile(path)
, mIndexLoad(rs_new SIndexLoad)
, mNextReadSize(0)
, mNextReadOff(0)
, mReadSizeWrapped(0)
, mReadOffWrapped(0)
, mCurLoadFrame(0)
, mRequestFrameWrapped(0)
, mCurFrame(0)
, mDecodedTexSlot(0)
, mDrawTexSlot(-1)
, mAudioSlot(-1)
, mDecodedTexCount(0)
, mFrameRem(0.f)
, mPlayMode(kPM_Playing)
, mTotalSeconds(0.f)
, mCurSeconds(0.f)
, mPreLoadSeconds(preLoadSeconds)
, mPreLoadFrames(0)
, mLoop(loop)
, mDeinterlace(deinterlace)
, mHasAudio(false)
, mFieldFlip(false)
, mCachedBytes(0)
, mFieldIndex(0) {

  static bool sThpInitialized = false;
  if (!sThpInitialized) {
    sThpInitialized = true;
    THPInit();
  }
#if defined(TARGET_PC)
  {
    const metaforce::AudioLockGuard lock;
#endif
    ++sNumReferences;
    VerifyCallbackStatus();
#if defined(TARGET_PC)
  }
#endif
  mIndexLoad->mHeaderRequest = mDvdFile.SyncRead(mIndexLoad->mBuffer.get(), 64);
}

bool CMoviePlayer::PumpIndexLoad() {
  if (mIndexLoad.null()) {
    return false;
  }
  uchar* const buffer = mIndexLoad->mBuffer.get();
  bool hasVideo;
  bool hasAudio;
  switch (mIndexLoad->mState) {
  case 0:
    if (mIndexLoad->mHeaderRequest->IsComplete()) {
      memcpy(&mHeader, buffer, sizeof(THPHeader));
      mHeader.mVersion = CBasics::SwapBytes(static_cast< uint >(mHeader.mVersion));
      mHeader.mBufferSize = CBasics::SwapBytes(static_cast< uint >(mHeader.mBufferSize));
      mHeader.mAudioMaxSamples =
          CBasics::SwapBytes(static_cast< uint >(mHeader.mAudioMaxSamples));
      mHeader.mFrameRate = CBasics::SwapBytes(mHeader.mFrameRate);
      mHeader.mNumFrames = CBasics::SwapBytes(static_cast< uint >(mHeader.mNumFrames));
      mHeader.mFirstFrameSize =
          CBasics::SwapBytes(static_cast< uint >(mHeader.mFirstFrameSize));
      mHeader.mMovieDataSize =
          CBasics::SwapBytes(static_cast< uint >(mHeader.mMovieDataSize));
      mHeader.mCompInfoDataOffsets =
          CBasics::SwapBytes(static_cast< uint >(mHeader.mCompInfoDataOffsets));
      mHeader.mOffsetDataOffsets =
          CBasics::SwapBytes(static_cast< uint >(mHeader.mOffsetDataOffsets));
      mHeader.mMovieDataOffsets =
          CBasics::SwapBytes(static_cast< uint >(mHeader.mMovieDataOffsets));
      mHeader.mFinalFrameDataOffsets =
          CBasics::SwapBytes(static_cast< uint >(mHeader.mFinalFrameDataOffsets));
      mIndexLoad->mHeaderRequest =
          mDvdFile.AsyncSeekRead(buffer, 32, kSO_Begin, mHeader.mCompInfoDataOffsets);
      ++mIndexLoad->mState;
    } else {
      return true;
    }
  case 1: {
    if (mIndexLoad->mHeaderRequest->IsComplete()) {
      memcpy(&mThpComponents, buffer, sizeof(THPFrameCompInfo));
      mThpComponents.mNumComponents =
          CBasics::SwapBytes(static_cast< uint >(mThpComponents.mNumComponents));
      mIndexLoad->mHeaderRequest = nullptr;
      uchar* audioBuffer = buffer + 32;
      int offset = mHeader.mCompInfoDataOffsets + sizeof(THPFrameCompInfo);
      for (uint i = 0; i < mThpComponents.mNumComponents; ++i) {
        switch (mThpComponents.mFrameComp[i]) {
        case 0:
          mIndexLoad->mVideoRequest = mDvdFile.AsyncSeekRead(buffer, 32, kSO_Begin, offset);
          offset += sizeof(THPVideoInfoOld);
          break;
        case 1:
          mIndexLoad->mAudioRequest =
              mDvdFile.AsyncSeekRead(audioBuffer, 32, kSO_Begin, offset);
          offset += sizeof(THPAudioInfoOld);
          mHasAudio = true;
          break;
        }
      }
      ++mIndexLoad->mState;
    } else {
      return true;
    }
  }
  case 2: {
    bool complete = true;
    hasVideo = mIndexLoad->mVideoRequest.get() != nullptr;
    hasAudio = mIndexLoad->mAudioRequest.get() != nullptr;
    if (hasVideo && !mIndexLoad->mVideoRequest->IsComplete()) {
      complete = false;
    }
    if (hasAudio && !mIndexLoad->mAudioRequest->IsComplete()) {
      complete = false;
    }
    if (!complete) {
      return true;
    }
    if (hasVideo) {
      memcpy(&mVideoInfo, buffer, sizeof(THPVideoInfoOld));
      mVideoInfo.mXSize = CBasics::SwapBytes(static_cast< uint >(mVideoInfo.mXSize));
      mVideoInfo.mYSize = CBasics::SwapBytes(static_cast< uint >(mVideoInfo.mYSize));
    }
    if (hasAudio) {
      memcpy(&mAudioInfo, buffer + 32, sizeof(THPAudioInfoOld));
      mAudioInfo.mSndChannels =
          CBasics::SwapBytes(static_cast< uint >(mAudioInfo.mSndChannels));
      mAudioInfo.mSndFrequency =
          CBasics::SwapBytes(static_cast< uint >(mAudioInfo.mSndFrequency));
      mAudioInfo.mSndNumSamples =
          CBasics::SwapBytes(static_cast< uint >(mAudioInfo.mSndNumSamples));
    }
  }
  }

  mIndexLoad = nullptr;
  mTextures.reserve(3);
  mNextReadOff = mHeader.mMovieDataOffsets;
  mNextReadSize = mHeader.mFirstFrameSize;
  mReadSizeWrapped = mHeader.mFirstFrameSize;
  mReadOffWrapped = mHeader.mMovieDataOffsets;
  mTotalSeconds = mHeader.mNumFrames / mHeader.mFrameRate;
  if (mPreLoadSeconds < 0.f) {
    mPreLoadSeconds = mTotalSeconds;
    mPreLoadFrames = mHeader.mNumFrames;
  } else if (mPreLoadSeconds > 0.f) {
    mPreLoadFrames =
        rstl::min_val(static_cast< uint >(mHeader.mNumFrames),
                      static_cast< uint >(mPreLoadSeconds * mHeader.mFrameRate));
    mPreLoadSeconds = rstl::min_val(mPreLoadSeconds, mTotalSeconds);
  }
  if (mPreLoadFrames > 0) {
    mRequestQueue.reserve(mPreLoadFrames);
  }
  PostDVDReadRequestIfNeeded();
  return false;
}

CMoviePlayer::~CMoviePlayer() {
#if defined(TARGET_PC)
  const metaforce::AudioLockGuard lock;
#endif
  --sNumReferences;
  VerifyCallbackStatus();
  if (sAudioPlayer == this) {
    sAudioPlayer = nullptr;
  }
}

void CMoviePlayer::InitializeTextures() {
#if defined(TARGET_PC)
  const metaforce::AudioLockGuard lock;
#endif
  const uint ySize = OSRoundUp32B(mVideoInfo.mXSize * mVideoInfo.mYSize);
  const uint uvSize = OSRoundUp32B(mVideoInfo.mXSize * mVideoInfo.mYSize / 4);
  const uint audioSize = mHeader.mAudioMaxSamples * 4;
  for (int i = 0; i < mTextures.capacity(); ++i) {
    void* y = CMemory::Alloc(ySize, IAllocator::kHI_RoundUpLen);
    void* u = CMemory::Alloc(uvSize, IAllocator::kHI_RoundUpLen);
    void* v = CMemory::Alloc(uvSize, IAllocator::kHI_RoundUpLen);
    void* audio = CMemory::Alloc(audioSize, IAllocator::kHI_RoundUpLen);
    DCFlushRangeNoSync(y, ySize);
    DCFlushRangeNoSync(u, uvSize);
    DCFlushRangeNoSync(v, uvSize);
    DCFlushRangeNoSync(audio, audioSize);
    mTextures.push_back(CTHPTextureSet(y, u, v, audio));
  }

  PPCSync();
  mDecodedTexSlot = 0;
  mDrawTexSlot = -1;
  mAudioSlot = -1;
}

void CMoviePlayer::PostDVDReadRequestIfNeeded() {
  if (mCurLoadFrame < mHeader.mNumFrames) {
    mRequestBuffer = rstl::auto_ptr< uchar >(
        static_cast< uchar* >(CMemory::Alloc(mNextReadSize, IAllocator::kHI_RoundUpLen)));
    mRequest = mDvdFile.AsyncSeekRead(mRequestBuffer.get(), mNextReadSize, kSO_Begin,
                                           mNextReadOff);
  }
}

void CMoviePlayer::ReadCompleted() {
  mRequest = nullptr;
  if (mCurLoadFrame == mRequestQueue.size() && mPreLoadFrames > mCurLoadFrame) {
    mRequestQueue.push_back(mRequestBuffer);
    mCachedBytes += mNextReadSize;
  }
  mNextReadOff += mNextReadSize;
  mNextReadSize = CBasics::SwapBytes(*reinterpret_cast< const uint* >(mRequestBuffer.get()));
  ++mCurLoadFrame;
  if (mCurLoadFrame == mPreLoadFrames) {
    if (mCurLoadFrame == mHeader.mNumFrames) {
      mReadSizeWrapped = mHeader.mFirstFrameSize;
      mReadOffWrapped = mHeader.mMovieDataOffsets;
    } else {
      mReadSizeWrapped = mNextReadSize;
      mReadOffWrapped = mNextReadOff;
    }
  }
  if (mCurLoadFrame >= mHeader.mNumFrames && mLoop) {
    mNextReadOff = mReadOffWrapped;
    mNextReadSize = mReadSizeWrapped;
    mCurLoadFrame = mPreLoadFrames;
  }
}

void CMoviePlayer::DecodeFromRead(const void* ptr) {
  uchar work[4096 + 32];
  void* alignedWork = reinterpret_cast< void* >((reinterpret_cast< uintptr_t >(work) + 31) & ~31);
  if (mTextures.empty()) {
    InitializeTextures();
  }
  CTHPTextureSet& texture = mTextures[mDecodedTexSlot];
  const uint* sizes = static_cast< const uint* >(ptr) + 2;
  const uchar* dataStart =
      static_cast< const uchar* >(ptr) + 8 + mThpComponents.mNumComponents * 4;
  uint offset = 0;
#if !defined(TARGET_PC)
  texture.SetAudioSamplesConsumed(0);
  texture.SetAudioSamples(0);
#endif
  for (uint i = 0; i < mThpComponents.mNumComponents; ++i) {
    const uchar* data = dataStart + offset;
    if (mThpComponents.mFrameComp[i] == 0) {
      THPVideoDecode(const_cast< uchar* >(data), texture.Y(), texture.U(), texture.V(),
                     alignedWork);
    } else if (mThpComponents.mFrameComp[i] == 1) {
#if defined(TARGET_PC)
      // Publish PCM and its counters together; the mixer may be reading this
      // slot while the game thread decodes the next video frame.
      const metaforce::AudioLockGuard lock;
#endif
      const uint samples =
          THPAudioDecode(static_cast< short* >(texture.Audio()), const_cast< uchar* >(data), 0);
      const BOOL interrupts = OSDisableInterrupts();
      texture.SetAudioSamples(samples);
      texture.SetAudioSamplesConsumed(0);
      OSRestoreInterrupts(interrupts);
    }
    offset += CBasics::SwapBytes(*sizes++);
  }
  if (++mDecodedTexSlot == mTextures.size()) {
    mDecodedTexSlot = 0;
  }
}

void CMoviePlayer::Update(float dt) {
  if (mCurLoadFrame < mPreLoadFrames) {
    if (!mRequest.null() && mRequest->IsComplete()) {
      ReadCompleted();
      if (mCurLoadFrame >= mRequestQueue.size() && mCurLoadFrame < mPreLoadFrames &&
          mRequestQueue.size() < mHeader.mNumFrames) {
        PostDVDReadRequestIfNeeded();
      }
    }
  } else if (!mRequest.null()) {
    bool canDecode = false;
    if (mRequestFrameWrapped >= mRequestQueue.size() &&
        mCurLoadFrame >= mRequestQueue.size()) {
      canDecode = true;
    }
    if (mRequest->IsComplete() && mDecodedTexCount < 2 && canDecode) {
      DecodeFromRead(mRequestBuffer.get());
      ReadCompleted();
      PostDVDReadRequestIfNeeded();
      ++mDecodedTexCount;
      ++mRequestFrameWrapped;
      if (mRequestFrameWrapped >= mHeader.mNumFrames && mLoop) {
        mRequestFrameWrapped = 0;
      }
    }
  }
  if (mRequest.null() && mPlayMode == kPM_Playing &&
      mRequestQueue.size() < mHeader.mNumFrames) {
    PostDVDReadRequestIfNeeded();
  }
  if (mDecodedTexCount < 2 && mPlayMode == kPM_Playing &&
      mRequestFrameWrapped < mPreLoadFrames) {
    const int frame = rstl::min_val(mRequestFrameWrapped, mRequestQueue.size() - 1);
    if (frame == -1) {
      return;
    }
    DecodeFromRead(mRequestQueue[frame].get());
    ++mDecodedTexCount;
    ++mRequestFrameWrapped;
    if (mRequestFrameWrapped >= mHeader.mNumFrames && mLoop) {
      mRequestFrameWrapped = 0;
    }
  }
  if (mDecodedTexCount > 0 && mPlayMode == kPM_Playing) {
    mCurSeconds += dt;
    if (mLoop) {
      mCurSeconds = CMath::ModF(mCurSeconds, mTotalSeconds);
    } else {
      mCurSeconds = rstl::min_val(mCurSeconds, mTotalSeconds);
    }
    float remainder = mFrameRem - dt;
    const float frameDt = 1.f / mHeader.mFrameRate;
    if (remainder <= 0.f) {
      if (!mFieldFlip) {
        if (++mDrawTexSlot >= mTextures.size()) {
          mDrawTexSlot = 0;
        }
        const BOOL interrupts = OSDisableInterrupts();
        if (mAudioSlot == -1) {
          mAudioSlot = 0;
        }
        OSRestoreInterrupts(interrupts);
        --mDecodedTexCount;
        ++mCurFrame;
        if (mCurFrame == mHeader.mNumFrames && mLoop) {
          mCurFrame = 0;
        }
        remainder += frameDt;
        mFieldIndex = 0;
      } else {
        remainder += dt;
        mFieldFlip = false;
      }
    }
    mFrameRem = remainder;
  }
}

void CMoviePlayer::DrawFrame(const CVector3f& v1, const CVector3f& v2, const CVector3f& v3,
                             const CVector3f& v4) {
  if (mDrawTexSlot == -1) {
    return;
  }
#if defined(TARGET_PC)
  const CGraphics::CProjectionState projection = CGraphics::GetProjectionState();
  metaforce::AdjustUiProjection();
#endif
  CGraphics::SetUseVideoFilter(mDeinterlace);
  const BOOL interrupts = OSDisableInterrupts();
  sAudioPlayer = this;
  OSRestoreInterrupts(interrupts);
  CTHPTextureSet& texture = mTextures[mDrawTexSlot];
  const bool field = CGraphics::GetDolphinLastFrameAbove();
  const bool deinterlace = mDeinterlace;
  MyTHPGXYuv2RgbSetup(field, deinterlace);
  MyTHPYuv2RgbTextureSetup(texture.Y(), texture.U(), texture.V(), mVideoInfo.mXSize,
                           mVideoInfo.mYSize);
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
#if defined(TARGET_PC)
  CGraphics::SetProjectionState(projection);
#endif
  if (mFieldIndex == 0 && field) {
    mFieldFlip = true;
  }
  ++mFieldIndex;
}

void CMoviePlayer::SetPlayMode(const EPlayMode mode) {
#if defined(TARGET_PC)
  const metaforce::AudioLockGuard lock;
#endif
mPlayMode = mode;
}

float CMoviePlayer::GetTotalSeconds() const { return mTotalSeconds; }

float CMoviePlayer::GetPlayedSeconds() const { return mCurSeconds + mFrameRem; }

bool CMoviePlayer::GetIsFullyCached() const { return mRequestQueue.size() >= mPreLoadFrames; }

bool CMoviePlayer::GetIsMovieFinishedPlaying() const {
  return !mLoop && mCurFrame == mHeader.mNumFrames;
}

void CMoviePlayer::Rewind() {
  if (!mRequest.null()) {
    mRequest->PostCancelRequest();
    mRequest = nullptr;
  }
  
#if defined(TARGET_PC)
  const metaforce::AudioLockGuard lock;
  m_audioPhase = 0;
  m_audioPrimed = false;
#endif
  mRequestBuffer = rstl::auto_ptr< uchar >(nullptr);
  mNextReadSize = mHeader.mFirstFrameSize;
  mNextReadOff = mHeader.mMovieDataOffsets;
  mReadSizeWrapped = mHeader.mFirstFrameSize;
  mReadOffWrapped = mHeader.mMovieDataOffsets;
  mCurLoadFrame = 0;
  mRequestFrameWrapped = 0;
  mCurFrame = 0;
  mDecodedTexSlot = 0;
  mDrawTexSlot = -1;
  mAudioSlot = -1;
  mDecodedTexCount = 0;
  mFrameRem = 0.f;
  mCurSeconds = 0.f;
  mTextures.clear();
}

#if defined(TARGET_PC)
void CMoviePlayer::StaticMyAudioCallback(short* output, size_t frames, u32 rate, u32 channels) {
  CMoviePlayer* player = sAudioPlayer;
  if (!player || player->mPlayMode != kPM_Playing ||
      player->mAudioSlot < 0 || player->mTextures.empty()) {
    return;
  }
  const u32 sourceRate = player->mAudioInfo.mSndFrequency;
  if (!sourceRate || sourceRate > 96000) {
    return;
  }

  for (size_t frame = 0; frame < frames; ++frame, output += channels) {
    short audio[2];
    if (sourceRate == rate) {
      player->MixAudio(audio, nullptr, 1);
    } else {
      if (!player->m_audioPrimed) {
        player->MixAudio(player->m_audioHistory[0], nullptr, 1);
        player->MixAudio(player->m_audioHistory[1], nullptr, 1);
        player->m_audioPrimed = true;
      }
      for (u32 channel = 0; channel < 2; ++channel) {
        const s32 a = player->m_audioHistory[0][channel];
        const s32 b = player->m_audioHistory[1][channel];
        audio[channel] = a + (s64(b - a) * player->m_audioPhase) / rate;
      }
      player->m_audioPhase += sourceRate;
      while (player->m_audioPhase >= rate) {
        player->m_audioPhase -= rate;
        for (u32 channel = 0; channel < 2; ++channel) {
          player->m_audioHistory[0][channel] = player->m_audioHistory[1][channel];
        }
        player->MixAudio(player->m_audioHistory[1], nullptr, 1);
      }
    }
    // THPAudioDecode follows the console's R,L ordering. Native output is L,R;
    // movies remain a stereo source in every speaker layout.
    if (sAudioEnabled && sSfxVolume != 0) {
      output[0] = std::clamp(s32(output[0]) + audio[1], -32768, 32767);
      output[1] = std::clamp(s32(output[1]) + audio[0], -32768, 32767);
    }
  }
}
#else
void CMoviePlayer::StaticMyAudioCallback() {
  if (sAudioPlayer != nullptr && sAudioPlayer->mHasAudio) {
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
#endif

void CMoviePlayer::MixAudio(short* out, const short* in, unsigned long samples) {
  const short* input = in;
  if (mAudioSlot == -1) {
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
    CTHPTextureSet& texture = mTextures[mAudioSlot];
    uint count = texture.GetAudioSamples() - texture.GetAudioSamplesConsumed();
    if (count > samples) {
      count = samples;
    } else {
      if (++mAudioSlot == mTextures.size()) {
        mAudioSlot = 0;
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

uint CMoviePlayer::GetWidth() const { return mVideoInfo.mXSize; }

uint CMoviePlayer::GetHeight() const { return mVideoInfo.mYSize; }

void CMoviePlayer::SetAudioEnabled(bool enabled) {
#if defined(TARGET_PC)
  const metaforce::AudioLockGuard lock;
#endif
  sAudioEnabled = enabled;
}

bool CMoviePlayer::GetAudioEnabled() {
#if defined(TARGET_PC)
  const metaforce::AudioLockGuard lock;
#endif
  return sAudioEnabled;
}

void CMoviePlayer::SetSfxVolume(uchar volume) {
#if defined(TARGET_PC)
  const metaforce::AudioLockGuard lock;
#endif
  sSfxVolume = rstl::min_val(uchar(127), volume);
}
