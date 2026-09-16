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
    uint x0_colorInArgs;
    uint x4_alphaInArgs;
    uint x8_colorOps;
    uint xc_alphaOps;
    uint x10_indFlags;
    uint x14_tevOrderFlags;
    uchar x18_kColorSel;
    uchar x19_kAlphaSel;

    STevState()
    : x0_colorInArgs(0)
    , x4_alphaInArgs(0)
    , x8_colorOps(0)
    , xc_alphaOps(0)
    , x10_indFlags(0)
    , x14_tevOrderFlags(UINT_MAX)
    , x18_kColorSel(UCHAR_MAX)
    , x19_kAlphaSel(UCHAR_MAX) {}
  };

  struct STexState {
    uint x0_coordGen;

    STexState() : x0_coordGen(0) {}
  };

  struct SFogParams {
    float x0_fogStartZ;
    float x4_fogEndZ;
    float x8_fogNearZ;
    float xc_fogFarZ;
    GXColor x10_fogColor;

    SFogParams() : x0_fogStartZ(0.f), x4_fogEndZ(1.f), x8_fogNearZ(0.1f), xc_fogFarZ(1.f) {
      x10_fogColor.a = 0;
      x10_fogColor.b = 0;
      x10_fogColor.g = 0;
      x10_fogColor.r = 0;
    }
  };

  struct SGXState {
    const void* x0_arrayPtrs[12];
    ushort x30_prevChanCtrls[2];
    ushort x34_chanCtrls[2];
    GXColor x38_chanAmbColors[2];
    GXColor x40_chanMatColors[2];
    uint x48_descList;
    union {
      uchar x4c_chanFlags;
      struct {
        uchar unused : 5;
        uchar chansDirty : 2;
        uchar numDirty : 1;
      } x4c_flags;
    };
    uchar x4d_prevNumChans;
    uchar x4e_numChans;
    uchar x4f_numTexGens;
    uchar x50_numTevStages;
    uchar x51_numIndStages;
    uchar x52_zmode;
    uchar x53_fogType;
    ushort x54_lineWidthAndOffset;
    ushort x56_blendMode;
    GXColor x58_kColors[4];
    STevState x68_tevStates[16];
    STexState x228_texStates[8];
    uint x248_alphaCompare;
    SFogParams x24c_fogParams;

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
  static void SetArray(GXAttr attr, const void* data, uchar stride);
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

#define RSWrite(T, n) (*(T*)GXFIFO_ADDR) = n
#define RSPosition3f32(x, y, z)                                                                    \
  {                                                                                                \
    RSWrite(f32, x);                                                                               \
    RSWrite(f32, y);                                                                               \
    RSWrite(f32, z);                                                                               \
  }

#endif // _CGX
