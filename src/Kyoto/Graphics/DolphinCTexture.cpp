#include "Kyoto/Alloc/IAllocator.hpp"
#include "Kyoto/CARAMToken.hpp"
#include "Kyoto/CFrameDelayedKiller.hpp"
#include "Kyoto/Graphics/CTexture.hpp"
#include "Kyoto/SObjectTag.hpp"
#include "dolphin/base/PPCArch.h"
#include "dolphin/gx/GXEnum.h"
#include "dolphin/gx/GXStruct.h"
#include "dolphin/gx/GXTexture.h"
#include "dolphin/os.h"
#include "dolphin/os/OSCache.h"
#include "rstl/single_ptr.hpp"
#include "types.h"

#include <Kyoto/Alloc/CMemory.hpp>
#include <Kyoto/CDvdRequest.hpp>
#include <Kyoto/CResFactory.hpp>
#include <Kyoto/Graphics/CGraphicsPalette.hpp>
#include <Kyoto/Math/CMath.hpp>
#include <Kyoto/Streams/CInputStream.hpp>

int CTexture::sCurrentFrameCount = 0;
int CTexture::sTotalAllocatedMemory = 0;
bool CTexture::sMangleMips = false;

#define ROUND_UP_4(v) (((v) + 3) & ~3)

static const CTexture* sLoadedTextures[GX_MAX_TEXMAP];

CTexture::CTexture(ETexelFormat fmt, const short w, const short h, int mips)
: mTexelFormat(fmt)
, mWidth(w)
, mHeight(h)
, mNumMips(mips)
, mBitsPerPixel(TexelFormatBitsPerPixel(fmt))
, mLocked(false)
, mCanLoadPalette(false)
, mIsPowerOfTwo(false)
, mNoSwap(true)
, mCounted(false)
, mCanLoadObj(false)
, mMemoryAllocated(0)
, mFrameAllocated(sCurrentFrameCount)
#if NONMATCHING
, mTexObj()
#endif
, mNativeFormat(GX_TF_RGB565)
, mNativeCIFormat(GX_TF_C8)
, mClampMode(kCM_Repeat) {
  InitBitmapBuffers(fmt, w, h, mips);
  InitTextureObjects();
}

CTexture::CTexture(CInputStream& in, EAutoMipmap automip, EBlackKey blackKey)
: mTexelFormat(kTF_Invalid)
, mWidth(0)
, mHeight(0)
, mNumMips(0)
, mBitsPerPixel(0)
, mLocked(false)
, mCanLoadPalette(false)
, mIsPowerOfTwo(false)
, mNoSwap(true)
, mCounted(false)
, mCanLoadObj(false)
, mMemoryAllocated(0)
, mFrameAllocated(sCurrentFrameCount)
#if NONMATCHING
, mTexObj()
#endif
, mNativeFormat(GX_TF_RGB565)
, mNativeCIFormat(GX_TF_C8)
, mClampMode(kCM_Repeat) {
  mTexelFormat = ETexelFormat(in.Get< uint >());
  mWidth = in.ReadUint16();
  mHeight = in.ReadUint16();
  mNumMips = in.ReadLong();
  if (IsCITextureFormat(mTexelFormat)) {
    mGraphicsPalette = rs_new CGraphicsPalette(in);
    mCanLoadPalette = true;
  }
  mBitsPerPixel = TexelFormatBitsPerPixel(mTexelFormat);
  InitBitmapBuffers(mTexelFormat, mWidth, mHeight, mNumMips);
  int bufLen = 0;
  for (int i = 0; i < mNumMips;) {
    int width = ((GetWidth() >> i) + 3) & ~3;
    int height = ((GetHeight() >> i) + 3) & ~3;
    int page = width * height;
    i++;
    bufLen += (GetBitsPerPixel() * page) >> 3;
  }

  uchar* buf = (uchar*)mARAMToken.GetMRAMSafe();
  for (int off = 0, len = 0; off < bufLen; off += len) {
    len = bufLen - off;
    if (len > 256) {
      len = 256;
    }

    in.Get(buf + off, len);
    DCFlushRangeNoSync(buf + off, OSRoundUp32B(len));
  }

  if (sMangleMips != false) {
    for (int i = 1; i < mNumMips; ++i) {
      MangleMipmap(i);
    }
  }

  InitTextureObjects();
  PPCSync();
}

CTexture::~CTexture() { UncountMemory(); }

void CTexture::InitTextureObjects() {
  mIsPowerOfTwo =
      CMath::FloorPowerOfTwo(mWidth) == mWidth && CMath::FloorPowerOfTwo(mHeight) == mHeight;

  if (!mIsPowerOfTwo) {
    mClampMode = kCM_Clamp;
  }
  bool hasMips = mNumMips > 1;
  GXTexWrapMode wrap = (GXTexWrapMode)mClampMode;
  short width = mWidth;
  short height = mHeight;
  void* buf = mARAMToken.GetMRAMSafe();
  CountMemory();

  if (IsCITextureFormat(mTexelFormat)) {
    GXInitTexObjCI(&mTexObj, buf, width, height, mNativeCIFormat, wrap, wrap, hasMips, 0);
  } else {
    GXInitTexObj(&mTexObj, buf, width, height, mNativeFormat, wrap, wrap, hasMips);
    GXInitTexObjLOD(&mTexObj, mNumMips > 1 ? GX_LIN_MIP_LIN : GX_LINEAR, GX_LINEAR, 0.f,
                    mNumMips - 1.f, 0.f, false, false, mNumMips > 1 ? GX_ANISO_4 : GX_ANISO_1);
  }

  mCanLoadObj = true;
}

void CTexture::Load(GXTexMapID tex, EClampMode clamp) const {
  if (sLoadedTextures[tex] != this || mCanLoadObj) {
    void* ptr = mARAMToken.GetMRAMSafe();
    CountMemory();

    if (!mGraphicsPalette.null()) {
      mGraphicsPalette->Load();
      mCanLoadPalette = false;
    }

    mCanLoadObj = false;

    if (mClampMode != clamp) {
      if (!mIsPowerOfTwo) {
        mClampMode = kCM_Clamp;
      } else {
        mClampMode = clamp;
      }

      GXInitTexObjWrapMode(&mTexObj, (GXTexWrapMode)mClampMode, (GXTexWrapMode)mClampMode);
    }

    GXInitTexObjData(&mTexObj, ptr);
    GXLoadTexObj(&mTexObj, tex);
    sLoadedTextures[tex] = this;
    mFrameAllocated = sCurrentFrameCount;
  }
}
void CTexture::LoadMipLevel(int mip, GXTexMapID tex, EClampMode clamp) const {
  char* ptr = (char*)mARAMToken.GetMRAMSafe();
  GXTexObj obj = mTexObj;
  int width = GetWidth();
  int height = GetHeight();
  int offset = 0;
  GXTexWrapMode wrap = (GXTexWrapMode)clamp;
  for (int i = 0; i < mip; i++) {
    int w = ROUND_UP_4(width);
    int h = ROUND_UP_4(height);
    offset += OSRoundUp32B(((mBitsPerPixel * (w * h)) / 8));
    width /= 2;
    height /= 2;
  }

  GXInitTexObj(&obj, ptr + offset, width, height, mNativeFormat, wrap, wrap, false);
  GXInitTexObjLOD(&obj, GX_LINEAR, GX_LINEAR, 0.f, 0.f, 0.f, false, false, GX_ANISO_1);
  if (!mGraphicsPalette.null()) {
    mGraphicsPalette->Load();
    mCanLoadPalette = false;
  }

  GXLoadTexObj(&obj, tex);
  sLoadedTextures[tex] = nullptr;
  mFrameAllocated = sCurrentFrameCount;
}

void CTexture::UnloadBitmapData(CAssetId textureId) const {
  if (!mBitmapReloader.null()) {
    bool b = mBitmapReloader->GetX10();
    mBitmapReloader = rs_new CDumpedBitmapDataReloader(textureId, mMemoryAllocated, b);
  } else {
    bool complete = mARAMToken.GetStatus() == CARAMToken::kS_Zero ||
                    mARAMToken.GetStatus() == CARAMToken::kS_Two ||
                    mARAMToken.GetStatus() == CARAMToken::kS_Five;

    mARAMToken = CARAMToken();
    mBitmapReloader = rs_new CDumpedBitmapDataReloader(textureId, mMemoryAllocated, complete);
  }
}

bool CTexture::TryReloadBitmapData(CResFactory& factory) const {
  if (mBitmapReloader.null()) {
    return true;
  }

  mBitmapReloader->BeginReloadBitmapData(factory);
  uchar* ptr = (uchar*)mBitmapReloader->TryBuildReloadedBitmapData(factory);
  if (ptr != nullptr) {

    bool bVar1 = mBitmapReloader->GetX10();
    mBitmapReloader = nullptr;

    mARAMToken.PostConstruct(ptr, mMemoryAllocated, 1);
    const_cast< CTexture& >(*this).InitTextureObjects();

    if (bVar1) {
      LoadToARAM();
    }

    return true;
  }
  return false;
}

int CTexture::fn_8030F088() const {
  if (!mBitmapReloader.null()) {
    if (mBitmapReloader->GetStatus() == 0) {
      return 2;
    }

    return 5;
  }

  switch (mARAMToken.GetStatus()) {
  case CARAMToken::kS_Zero:
    return 1;
  case CARAMToken::kS_One:
    return 0;
  case CARAMToken::kS_Five:
  case CARAMToken::kS_Two:
    return 3;
  case CARAMToken::kS_Three:
  case CARAMToken::kS_Four:
    return 4;
  default:
    return -1;
  }
}

CTexture::CDumpedBitmapDataReloader::CDumpedBitmapDataReloader(uint unk1, uint unk2, bool unk3)
: x0_(0), x4_(unk1), x8_(0), xc_(unk2), x10_(unk3) {}

void CTexture::CDumpedBitmapDataReloader::BeginReloadBitmapData(CResFactory& factory) {
  if (x0_ != 0) {
    return;
  }
  SObjectTag tag('TXTR', x4_);
  x8_ = factory.ResourceSize(tag);
  x18_ = (uchar*)CMemory::Alloc(x8_, IAllocator::kHI_RoundUpLen);
  x14_ = factory.GetResLoader().LoadResourceAsync(tag, (char*)x18_.get());
  x0_ = 1;
}

void* CTexture::CDumpedBitmapDataReloader::TryBuildReloadedBitmapData(CResFactory& factory) {
  if (x14_->IsComplete()) {
    x0_ = 2;
    x14_ = nullptr;

    SObjectTag tag('TXTR', x4_);
    rstl::single_ptr< CInputStream > buf = factory.LoadResourceFromMemorySync(tag, x18_.get());
    CInputStream* in = buf.get();
    ETexelFormat format = ETexelFormat(in->ReadInt32());
    const int w = in->ReadInt16();
    const int h = in->ReadInt16();
    const int numMips = in->ReadInt32();
    const int bitsPerPixel = TexelFormatBitsPerPixel(format);

    if (IsCITextureFormat(format)) {
      CGraphicsPalette tmp(*in);
    }

    int bufLen = 0;
    for (int i = 0; i < numMips;) {
      int width = ((w >> i) + 3) & ~3;
      int height = ((h >> i) + 3) & ~3;
      int page = (width * height);
      i++;
      bufLen += (page * bitsPerPixel) >> 3;
    }

    void* ptr = CMemory::Alloc(xc_, IAllocator::kHI_RoundUpLen);

    for (int off = 0, len = 0; off < bufLen; off += len) {
      len = bufLen - off;
      if (len > 256) {
        len = 256;
      }

      in->Get((char*)ptr + off, len);
      DCFlushRangeNoSync((char*)ptr + off, OSRoundUp32B(len));
    }
    PPCSync();
    x18_ = nullptr;
    return ptr;
  }
  return nullptr;
}

bool CTexture::LoadToMRAM() const {
  if (mARAMToken.GetStatus() == CARAMToken::kS_Six) {
    return false;
  }

  mFrameAllocated = sCurrentFrameCount;
  CountMemory();
  return mARAMToken.LoadToMRAM();
}

bool CTexture::LoadToARAM() const {
  if (mARAMToken.GetStatus() == CARAMToken::kS_Six) {
    return false;
  }

  if (mNoSwap) {
    return false;
  }

  if (mFrameAllocated < sCurrentFrameCount - 1) {
    bool ret = mARAMToken.LoadToARAM();

    if (mARAMToken.GetStatus() != CARAMToken::kS_One) {
      UncountMemory();
      mCanLoadObj = true;
    }
    return ret;
  }
  return false;
}

bool CTexture::IsARAMTransferInProgress() const {
  if (mNoSwap) {
    return false;
  }
  return mARAMToken.GetStatus() >= CARAMToken::kS_Two &&
         mARAMToken.GetStatus() <= CARAMToken::kS_Five;
}

int CTexture::TexelFormatBitsPerPixel(ETexelFormat fmt) {
  switch (fmt) {
  case kTF_I4:
  case kTF_C4:
  case kTF_CMPR:
    return 4;
  case kTF_I8:
  case kTF_IA4:
  case kTF_C8:
    return 8;
  case kTF_IA8:
  case kTF_C14X2:
  case kTF_RGB565:
  case kTF_RGB5A3:
    return 16;
  case kTF_RGBA8:
    return 32;
  default:
    return 0;
  }
}

void CTexture::InitBitmapBuffers(ETexelFormat fmt, short width, short height, int mips) {
  switch (fmt) {
  case kTF_C4:
    mNativeCIFormat = GX_TF_C4;
    break;
  case kTF_C8:
    mNativeCIFormat = GX_TF_C8;
    break;
  case kTF_C14X2:
    mNativeCIFormat = GX_TF_C14X2;
    break;
  case kTF_I4:
    mNativeFormat = GX_TF_I4;
    break;
  case kTF_I8:
    mNativeFormat = GX_TF_I8;
    break;
  case kTF_IA4:
    mNativeFormat = GX_TF_IA4;
    break;
  case kTF_IA8:
    mNativeFormat = GX_TF_IA8;
    break;
  case kTF_RGB565:
    mNativeFormat = GX_TF_RGB565;
    break;
  case kTF_RGB5A3:
    mNativeFormat = GX_TF_RGB5A3;
    break;
  case kTF_RGBA8:
    mNativeFormat = GX_TF_RGBA8;
    break;
  case kTF_CMPR:
    mNativeFormat = GX_TF_CMPR;
    break;
  default:
    break;
  }

  bool hasMips = mips > 1;
  mMemoryAllocated = GXGetTexBufferSize(
      width, height, HasPalette() ? mNativeCIFormat : mNativeFormat, hasMips, hasMips ? 11 : 0);
  mARAMToken.PostConstruct(CMemory::Alloc(mMemoryAllocated, IAllocator::kHI_RoundUpLen),
                           mMemoryAllocated, 1);
  CountMemory();
}

void CTexture::UnLock() {
  mLocked = false;
  CountMemory();
  DCFlushRange(mARAMToken.GetMRAMSafe(), OSRoundUp32B(mMemoryAllocated));
}

const CFactoryFnReturn FTextureFactory(const SObjectTag& tag, CInputStream& in,
                                 const CVParamTransfer& xfer) {
  return rs_new CTexture(in, CTexture::kAM_Zero, CTexture::kBK_Zero);
}

const void* CTexture::GetConstBitMapData(const int mip) const {
  int offset = 0;
  for (int i = 0; i < mip; i++) {
    offset += (GetBitsPerPixel() >> 3) * ((GetWidth() >> i) * (GetHeight() >> i));
  }

  return (const uchar*)mARAMToken.GetMRAMSafe() + offset;
}

void* CTexture::GetBitMapData(int mip) { return const_cast< void* >(GetConstBitMapData(mip)); }

void CTexture::MangleMipmap(int mip) {
  if (mip >= mNumMips) {
    return;
  }

  const uint colors[4] = {
      0x000000FF,
      0x0000FF00,
      0x00FF0000,
      0x0000FFFF,
  };
  const uint color = colors[(mip - 1) & 3];
  ushort rgb565Color = ((color >> 3) & 0x001F) | // B
                       ((color >> 5) & 0x07E0) | // G
                       ((color >> 8) & 0xF800);  // R
  ushort rgb555Color = ((color >> 3) & 0x001F) | // B
                       ((color >> 6) & 0x03E0) | // G
                       ((color >> 9) & 0x7C00);  // R
  ushort rgb4Color = ((color >> 4) & 0x000F) |   // B
                     ((color >> 8) & 0x00F0) |   // G
                     ((color >> 12) & 0x0F00);   // R

  int width = GetWidth();
  int height = GetHeight();

  int offset = 0;
  for (int i = 0; i < mip; i++) {
    offset += width * height;
    width /= 2;
    height /= 2;
  }

  switch (GetTexelFormat()) {
  case kTF_RGB565: {
    ushort* ptr = static_cast< ushort* >(mARAMToken.GetMRAMSafe());
    for (int i = 0; i < width * height; ++i) {
      ptr[i + offset] = rgb565Color;
    }
    break;
  }
  case kTF_CMPR: {
    ushort* ptr = static_cast< ushort* >(mARAMToken.GetMRAMSafe()) + offset / 4;
    for (int i = 0; i < width * height / 16; ++i, ptr += 4) {
      ptr[0] = rgb565Color;
      ptr[1] = rgb565Color;
      ptr[2] = 0;
      ptr[3] = 0;
    }
    break;
  }
  case kTF_RGB5A3: {
    ushort* ptr = static_cast< ushort* >(mARAMToken.GetMRAMSafe());
    for (int i = 0; i < width * height; ++i) {
      ushort& val = ptr[i + offset];
      if (val & 0x8000) {
        val = rgb555Color | 0x8000;
      } else {
        val = (val & 0xF000) | rgb4Color;
      }
    }
    break;
  }
  }
}

void CTexture::MakeSwappable() const {
  if (!mNoSwap) {
    return;
  }

  mNoSwap = false;
}

void CTexture::CountMemory() const {
  if (mCounted) {
    return;
  }

  mCounted = true;
  sTotalAllocatedMemory += mMemoryAllocated;
}

void CTexture::UncountMemory() const {
  if (!mCounted) {
    return;
  }

  mCounted = false;
  sTotalAllocatedMemory -= mMemoryAllocated;
}

void CTexture::InvalidateTexmap(GXTexMapID texmap) { sLoadedTextures[texmap] = nullptr; }

void CTexture::ScheduleDeletion() {
  if (mARAMToken.GetStatus() != CARAMToken::kS_Six) {
    CFrameDelayedKiller::ScheduleDeletion(CFrameDelayedKiller::kWhichFrame_NextFrame,
                                          mARAMToken.ForceSyncMRAM());
  }
}
