#ifndef _CTEXTURE
#define _CTEXTURE

#include "types.h"

#include "Kyoto/CARAMToken.hpp"
#include "rstl/single_ptr.hpp"

#include "dolphin/gx/GXEnum.h"
#include "dolphin/gx/GXStruct.h"

class CResFactory;
class CDvdRequest;
class CInputStream;
class CGraphicsPalette;

enum ETexelFormat {
  kTF_Invalid = -1,
  kTF_I4 = 0,
  kTF_I8 = 1,
  kTF_IA4 = 2,
  kTF_IA8 = 3,
  kTF_C4 = 4,
  kTF_C8 = 5,
  kTF_C14X2 = 6,
  kTF_RGB565 = 7,
  kTF_RGB5A3 = 8,
  kTF_RGBA8 = 9,
  kTF_CMPR = 10,
};

class CTexture {
public:
  class CDumpedBitmapDataReloader {
    int x0_;
    uint x4_;
    int x8_;
    uint xc_;
    bool x10_;
    rstl::single_ptr< CDvdRequest > x14_;
    rstl::single_ptr< uchar > x18_;

  public:
    CDumpedBitmapDataReloader(uint unk1, uint unk2, bool unk3);

    void BeginReloadBitmapData(CResFactory& factory);
    void* TryBuildReloadedBitmapData(CResFactory& factory);
    int GetStatus() const { return x0_; }
    const bool GetX10() const { return x10_; }
  };

  enum EClampMode {
    kCM_Clamp,
    kCM_Repeat,
    kCM_Mirror,
  };
  enum EAutoMipmap {
    kAM_Zero,
  };
  enum EBlackKey {
    kBK_Zero,
  };

  CTexture(ETexelFormat fmt, short w, short h, int mips);
  CTexture(CInputStream& stream, EAutoMipmap mip, EBlackKey bk);
  ~CTexture();

  // Used in certain destructors
  void ScheduleDeletion();
  void LoadMipLevel(int, GXTexMapID tex, EClampMode) const;
  void Load(GXTexMapID texMapId, EClampMode clampMode) const;
  bool HasPalette() const { return IsCITextureFormat(mTexelFormat); }
  const void* GetConstBitMapData(const int mip) const;
  void* GetBitMapData(int);
  ETexelFormat GetTexelFormat() const { return mTexelFormat; }
  const short GetWidth() const { return mWidth; }
  const short GetHeight() const { return mHeight; }
  int GetNumberOfMipMaps() const { return mNumMips; }
  uint GetMemoryAllocated() const { return mMemoryAllocated; }
  bool GetNoSwap() const { return mNoSwap; }
#if defined(TARGET_PC)
  void* Lock();
#else
  void* Lock() {
    mLocked = true;
    return GetBitMapData(0);
  }
#endif

  void MakeSwappable() const;
  void CountMemory() const;
  void UncountMemory() const;
  void SetFlag1(bool b) { mLocked = b; }
  void MangleMipmap(int mip);
  const char GetBitsPerPixel() const { return mBitsPerPixel; }
  void UnloadBitmapData(uint unk) const;
  bool TryReloadBitmapData(CResFactory& factory) const;
  int fn_8030F088() const;
  bool LoadToMRAM() const;
  bool LoadToARAM() const;
  bool IsARAMTransferInProgress() const;
  static int TexelFormatBitsPerPixel(ETexelFormat fmt);
  void InitBitmapBuffers(const ETexelFormat fmt, const short w, const short h, const int mips);
  void InitTextureObjects();
  void UnLock();
  CGraphicsPalette* GetPalette() { return mGraphicsPalette.get(); }
  const CGraphicsPalette* GetPalette() const { return mGraphicsPalette.get(); }

  static void InvalidateTexmap(GXTexMapID id);
  static bool IsCITextureFormat(ETexelFormat fmt) {
    return fmt == kTF_C4 ? true : fmt == kTF_C8 ? true : fmt == kTF_C14X2 ? true : false;
  }

  static void SetMangleMipmaps(bool v) { sMangleMips = true; }
  static const bool GetMangleMipmaps() { return sMangleMips; }
  static int sCurrentFrameCount;
  static int sTotalAllocatedMemory;
  static bool sMangleMips;

private:
  ETexelFormat mTexelFormat;
  short mWidth;
  short mHeight;
  char mNumMips;
  char mBitsPerPixel;
  bool mLocked : 1;
  mutable bool mCanLoadPalette : 1;
  bool mIsPowerOfTwo : 1;
  mutable bool mNoSwap : 1;
  mutable bool mCounted : 1;
  mutable bool mCanLoadObj : 1;
  uint mMemoryAllocated;
  rstl::single_ptr< CGraphicsPalette > mGraphicsPalette;
  mutable rstl::single_ptr< CDumpedBitmapDataReloader > mBitmapReloader;
  GXTexFmt mNativeFormat;
  GXCITexFmt mNativeCIFormat;
  mutable GXTexObj mTexObj;
  mutable EClampMode mClampMode;
  mutable CARAMToken mARAMToken;
  mutable uint mFrameAllocated;
};
CHECK_SIZEOF(CTexture, 0x68)

#endif // _CTEXTURE
