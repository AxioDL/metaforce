#ifndef _CGX
#define _CGX

#include "types.h"

#include <limits.h>
#include <stddef.h>

#include <dolphin/gx/GXBump.h>
#include <dolphin/gx/GXEnum.h>
#include <dolphin/gx/GXPixel.h>
#include <dolphin/gx/GXStruct.h>
#include <dolphin/gx/GXTransform.h>
#include <dolphin/gx/GXVert.h>
#include <dolphin/mtx.h>

class CGX {
public:
  struct STevState {
    uint mColorInArgs;
    uint mAlphaInArgs;
    uint mColorOps;
    uint mAlphaOps;
    uint mIndFlags;
    uint mTevOrderFlags;
    uchar mKColorSel;
    uchar mKAlphaSel;

    STevState()
    : mColorInArgs(0)
    , mAlphaInArgs(0)
    , mColorOps(0)
    , mAlphaOps(0)
    , mIndFlags(0)
    , mTevOrderFlags(UINT_MAX)
    , mKColorSel(UCHAR_MAX)
    , mKAlphaSel(UCHAR_MAX) {}
  };

  struct STexState {
    uint mCoordGen;

    STexState() : mCoordGen(0) {}
  };

  struct SFogParams {
    float mFogStartZ;
    float mFogEndZ;
    float mFogNearZ;
    float mFogFarZ;
    GXColor mFogColor;

    SFogParams() : mFogStartZ(0.f), mFogEndZ(1.f), mFogNearZ(0.1f), mFogFarZ(1.f) {
      mFogColor.a = 0;
      mFogColor.b = 0;
      mFogColor.g = 0;
      mFogColor.r = 0;
    }
  };

  struct SGXState {
    const void* mArrayPtrs[12];
    ushort mPrevChanCtrls[2];
    ushort mChanCtrls[2];
    GXColor mChanAmbColors[2];
    GXColor mChanMatColors[2];
    uint mDescList;
    union {
      uchar mChanFlags;
      struct {
        uchar unused : 5;
        uchar chansDirty : 2;
        uchar numDirty : 1;
      } mFlags;
    };
    uchar mPrevNumChans;
    uchar mNumChans;
    uchar mNumTexGens;
    uchar mNumTevStages;
    uchar mNumIndStages;
    uchar mZmode;
    uchar mFogType;
    ushort mLineWidthAndOffset;
    ushort mBlendMode;
    GXColor mKColors[4];
    STevState mTevStates[16];
    STexState mTexStates[8];
    uint mAlphaCompare;
    SFogParams mFogParams;

    SGXState();
  };

  enum EChannelId {
    Channel0, // GX_COLOR0
    Channel1, // GX_COLOR1
  };

  static void SetNumChans(uchar num);
  static void SetNumTexGens(uchar num);
  static void SetNumTevStages(uchar num);
  static void SetNumIndStages(uchar num);
  static void SetChanAmbColor(EChannelId channel, const GXColor& color);
  static void SetChanMatColor(EChannelId channel, const GXColor& color);
  static void SetChanCtrl(EChannelId channel, GXBool enable, GXColorSrc ambSrc, GXColorSrc matSrc,
                          GXLightID lights, GXDiffuseFn diffFn, GXAttnFn attnFn);
  static void SetChanCtrl_Compressed(EChannelId channel, GXLightID lights, uint ctrl);
  static void SetTevKColor(GXTevKColorID id, const GXColor& color);
  static void SetTevColorIn(GXTevStageID stageId, GXTevColorArg a, GXTevColorArg b, GXTevColorArg c,
                            GXTevColorArg d);
  static void SetTevColorIn_Compressed(GXTevStageID stageId, uint flags);
  static void SetTevAlphaIn(GXTevStageID stageId, GXTevAlphaArg a, GXTevAlphaArg b, GXTevAlphaArg c,
                            GXTevAlphaArg d);
  static void SetTevAlphaIn_Compressed(GXTevStageID stageId, uint flags);
  static void SetTevColorOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias, GXTevScale scale,
                            GXBool clamp, GXTevRegID outReg);
  static void SetTevColorOp_Compressed(GXTevStageID stageId, uint flags);
  static void SetTevAlphaOp(GXTevStageID stageId, GXTevOp op, GXTevBias bias, GXTevScale scale,
                            GXBool clamp, GXTevRegID outReg);
  static void SetTevAlphaOp_Compressed(GXTevStageID stageId, uint flags);
  static void SetTevKColorSel(GXTevStageID stageId, GXTevKColorSel sel);
  static void SetTevKAlphaSel(GXTevStageID stageId, GXTevKAlphaSel sel);
  static void SetTevOrder(GXTevStageID stageId, GXTexCoordID texCoord, GXTexMapID texMap,
                          GXChannelID color);
  static void SetBlendMode(GXBlendMode mode, GXBlendFactor srcFac, GXBlendFactor dstFac,
                           GXLogicOp op);
  static void SetZMode(GXBool compareEnable, GXCompare func, GXBool updateEnable);
  static void SetAlphaCompare(GXCompare comp0, uchar ref0, GXAlphaOp op, GXCompare comp1,
                              uchar ref1);
  static void SetTevIndirect(const GXTevStageID stageId, const GXIndTexStageID indStage,
                             const GXIndTexFormat fmt, const GXIndTexBiasSel biasSel,
                             const GXIndTexMtxID mtxSel, const GXIndTexWrap wrapS,
                             const GXIndTexWrap wrapT, GXBool addPrev, const GXBool indLod,
                             const GXIndTexAlphaSel alphaSel);
  static void SetTevIndWarp(const GXTevStageID stageId, const GXIndTexStageID indStage,
                            const uchar signedOffset, const uchar replaceMode,
                            const GXIndTexMtxID mtxSel) {
    const GXIndTexWrap wrap = replaceMode != 0 ? GX_ITW_0 : GX_ITW_OFF;
    SetTevIndirect(stageId, indStage, GX_ITF_8, signedOffset != 0 ? GX_ITB_STU : GX_ITB_NONE,
                   mtxSel, wrap, wrap, GX_FALSE, GX_FALSE, GX_ITBA_OFF);
  }
  static void SetTevDirect(GXTevStageID stageId);
  static void SetTexCoordGen(GXTexCoordID dstCoord, GXTexGenType fn, GXTexGenSrc src, GXTexMtx mtx,
                             GXBool normalize, GXPTTexMtx postMtx);
  static void SetTexCoordGen_Compressed(GXTexCoordID dstCoord, uint flags);
#if defined(TARGET_PC)
  static void SetArray(GXAttr attr, const void* data, uchar stride, size_t size, bool le);
#else
  static void SetArray(GXAttr attr, const void* data, uchar stride);
#endif
  static void SetFog(GXFogType type, float startZ, float endZ, float nearZ, float farZ,
                     const GXColor& color);
  static void SetLineWidth(uchar width, GXTexOffset offset);
  static void SetIndTexMtxSTPointFive(GXIndTexMtxID id, s8 scaleExp);
  static void SetVtxDescv_Compressed(uint flags);
  static void SetVtxDesc(GXAttr attr, GXAttrType type); // name?
  static void ResetVtxDescv();                          // name?
  static void SetVtxDescv(const GXVtxDescList* list);
  static void SetStandardDirectTev_Compressed(GXTevStageID stageId, uint colorArgs, uint alphaArgs,
                                              uint colorOps, uint alphaOps);
  static void SetStandardTevColorAlphaOp(GXTevStageID stageId);

  static void CallDisplayList(const void* ptr, size_t size);
  static void Begin(GXPrimitive prim, GXVtxFmt fmt, ushort numVtx);
  static void End();
  static void ResetGXStates();
  static void ResetGXStatesFull(); // name?

  static inline void LoadTexMtxImm(const float mtx[][4], unsigned long id, GXTexMtxType type) {
    GXLoadTexMtxImm(const_cast< MtxPtr >(mtx), id, type);
  }

  static GXColor GetChanAmbColor(EChannelId channel);
  static void GetFog(GXFogType* fogType, float* fogStartZ, float* fogEndZ, float* fogNearZ,
                     float* fogFarZ, GXColor* fogColor);

  static inline bool CompareGXColors(const GXColor& lhs, const GXColor& rhs) {
    return *reinterpret_cast< const uint* >(&lhs) == *reinterpret_cast< const uint* >(&rhs);
  }
  static inline void CopyGXColor(GXColor& dst, const GXColor& src) {
    *reinterpret_cast< uint* >(&dst) = *reinterpret_cast< const uint* >(&src);
  }
  static inline uint MaskAndShiftLeft(uint v, uint m, uint s) { return (v << s) & (m << s); }
  static inline uint ShiftRightAndMask(uint v, uint m, uint s) { return (v >> s) & m; }

  static void FlushState();

private:
  static void update_fog(uint flags);
  static inline void apply_fog();

  static SGXState sGXState;
};

// Direct FIFO write macros
#ifndef GXFIFO_ADDR
#define GXFIFO_ADDR 0xCC008000
#endif
#if TARGET_PC
#define RSPosition3f32(x, y, z) GXPosition3f32(x, y, z)
#else
#define RSWrite(T, n) (*(T*)GXFIFO_ADDR) = n
#define RSPosition3f32(x, y, z)                                                                    \
  {                                                                                                \
    RSWrite(f32, x);                                                                               \
    RSWrite(f32, y);                                                                               \
    RSWrite(f32, z);                                                                               \
  }
#endif
#endif // _CGX
