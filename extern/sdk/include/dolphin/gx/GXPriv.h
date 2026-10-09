#ifndef _DOLPHIN_GXPRIV
#define _DOLPHIN_GXPRIV

#include "GameVersions.h"
#include "dolphin/gx/GXVert.h"

typedef struct GXLightObjInt {
  u32 padding[3];
  u32 color;
  float a0;
  float a1;
  float a2;
  float k0;
  float k1;
  float k2;
  float px;
  float py;
  float pz;
  float nx;
  float ny;
  float nz;
} GXLightObjInt;

#define XF_LIGHT_BASE 0x0600
#define XF_LIGHT_SIZE 0x10

#define GX_FIFO_ADDR 0xCC008000

volatile PPCWGPipe __GXWGFifo AT_ADDRESS(GX_FIFO_ADDR);

#define GX_WRITE_U8(v) (__GXWGFifo.u8 = (u8)(v))
#define GX_WRITE_U32(v) (__GXWGFifo.u32 = (u32)(v))

typedef struct __GXData_struct {
  u16 vNumNot;
  u16 bpSentNot;
  u16 vNum;
  u16 vLim;
  u32 cpEnable;
  u32 cpStatus;
  u32 cpClr;
  u32 vcdLo;
  u32 vcdHi;
  u32 vatA[8];
  u32 vatB[8];
  u32 vatC[8];
  u32 lpSize;
  u32 matIdxA;
  u32 matIdxB;
  u32 indexBase[4];
  u32 indexStride[4];
  u32 ambColor[2];
  u32 matColor[2];
  u32 suTs0[8];
  u32 suTs1[8];
  u32 suScis0;
  u32 suScis1;
  u32 tref[8];
  u32 iref;
  u32 bpMask;
  u32 IndTexScale0;
  u32 IndTexScale1;
  u32 tevc[16];
  u32 teva[16];
  u32 tevKsel[8];
  u32 cmode0;
  u32 cmode1;
  u32 zmode;
  u32 peCtrl;
  u32 cpDispSrc;
  u32 cpDispSize;
  u32 cpDispStride;
  u32 cpDisp;
  u32 cpTexSrc;
  u32 cpTexSize;
  u32 cpTexStride;
  u32 cpTex;
  GXBool cpTexZ;
  u32 genMode;
  GXTexRegion TexRegions[8];
  GXTexRegion TexRegionsCI[4];
  u32 nextTexRgn;
  u32 nextTexRgnCI;
  GXTlutRegion TlutRegions[20];
  GXTexRegion* (*texRegionCallback)(const GXTexObj*, GXTexMapID);
  GXTlutRegion* (*tlutRegionCallback)(u32);
  GXAttrType nrmType;
  GXBool hasNrms;
  GXBool hasBiNrms;
  u32 projType;
  f32 projMtx[6];
  f32 vpLeft;
  f32 vpTop;
  f32 vpWd;
  f32 vpHt;
  f32 vpNearz;
  f32 vpFarz;
  u8 fgRange;
  f32 fgSideX;
  u32 tImage0[8];
  u32 tMode0[8];
  u32 texmapId[16];
  u32 tcsManEnab;
  u32 tevTcEnab;
  GXPerf0 perf0;
  GXPerf1 perf1;
  u32 perfSel;
  GXBool inDispList;
  GXBool dlSaveContext;
#if VERSION == VERSION_GM8E_02
  GXBool abtWaitPECopy;
#endif
  u8 dirtyVAT;
  u32 dirtyState;
} GXData;

typedef struct __GXFifoObj {
  u8* base;
  u8* top;
  u32 size;
  u32 hiWatermark;
  u32 loWatermark;
  void* rdPtr;
  void* wrPtr;
  s32 count;
  u8 bind_cpu;
  u8 bind_gp;
} __GXFifoObj;

void __GXCleanGPFifo(void);
void __GetImageTileCount(GXTexFmt fmt, u16 wd, u16 ht, u32* rowTiles, u32* colTiles, u32* cmpTiles);

extern GXData* const __GXData;

extern void* __piReg;
extern void* __cpReg;
extern void* __peReg;
extern void* __memReg;

#define GX_WRITE_U16(v) (__GXWGFifo.u16 = (u16)(v))
#define GX_WRITE_F32(v) (__GXWGFifo.f32 = (f32)(v))
#define GX_WRITE_RAS_REG(v) GX_WRITE_RA_REG(v)

#define SET_REG_FIELD(reg, size, shift, val)                                                       \
  do {                                                                                             \
    (reg) = ((u32)(reg) & ~(((1 << (size)) - 1) << (shift))) | ((u32)(val) << (shift));            \
  } while (0)
#define GET_REG_FIELD(reg, size, shift) ((int)((reg) >> (shift)) & ((1 << (size)) - 1))

#define __SET_REG_FIELD(reg, size, shift, val)                                                     \
  ((reg) = __rlwimi((u32)(reg), (val), (shift), 32 - (shift) - (size), 31 - (shift)))

#define GX_WRITE_XF_REG(addr, value)                                                               \
  do {                                                                                             \
    GX_WRITE_U8(0x10);                                                                             \
    GX_WRITE_U32(0x1000 + (addr));                                                                 \
    GX_WRITE_U32(value);                                                                           \
  } while (0)

#define GX_WRITE_SOME_REG4(command, address, value, index)                                         \
  do {                                                                                             \
    s32 regAddr;                                                                                   \
    GX_WRITE_U8(command);                                                                          \
    GX_WRITE_U8(address);                                                                          \
    GX_WRITE_U32(value);                                                                           \
    regAddr = index;                                                                               \
  } while (0)
#define GX_WRITE_SOME_REG2(command, address, value, index)                                         \
  GX_WRITE_SOME_REG4(command, address, value, index)
#define GX_WRITE_SOME_REG3(command, address, value, index)                                         \
  GX_WRITE_SOME_REG4(command, address, value, index)

#define GX_GET_MEM_REG(offset) (*(volatile u16*)((u8*)__memReg + (offset) * 2))
#define GX_GET_CP_REG(offset) (((volatile u16*)__cpReg)[offset])
#define GX_GET_PE_REG(offset) (((volatile u16*)__peReg)[offset])
#define GX_GET_PI_REG(offset) (((volatile u32*)__piReg)[offset])
#define GX_SET_MEM_REG(offset, value) (GX_GET_MEM_REG(offset) = (value))
#define GX_SET_CP_REG(offset, value) (GX_GET_CP_REG(offset) = (value))
#define GX_SET_PE_REG(offset, value) (GX_GET_PE_REG(offset) = (value))
#define GX_SET_PI_REG(offset, value) (GX_GET_PI_REG(offset) = (value))

void __GXFifoInit(void);
void __GXPEInit(void);
void __GXSetDirtyState(void);
void __GXSetSUTexRegs(void);
void __GXFlushTextureState(void);
void __GXUpdateBPMask(void);
void __GXSetGenMode(void);
void __GXSetVCD(void);
void __GXSetVAT(void);
void __GXCalculateVLim(void);
void __GXSendFlushPrim(void);

void __GXSetMatrixIndex(GXAttr matIdxAttr);
void __GXSetRange(f32 nearz, f32 sideX);
void __GXInitGX();

#if VERSION == VERSION_GM8E_02
void __GXAbort(void);

static inline u32 __GXReadMEMCounterU32(u32 addrHi, u32 addrLo) {
  u32 ctrH0, ctrH1, ctrL;
  ctrH0 = GX_GET_MEM_REG(addrHi);
  do {
    ctrH1 = ctrH0;
    ctrH0 = GX_GET_MEM_REG(addrHi);
    ctrL = GX_GET_MEM_REG(addrLo);
  } while (ctrH0 != ctrH1);
  return (ctrH0 << 16) | ctrL;
}
#endif

#define GX_REG_ASSERT(c) ASSERTMSG(c, "GX Internal: Register field out of range")

#define GX_FLAG_SET(regOrg, newFlag, regName)                                                      \
  do {                                                                                             \
    GX_REG_ASSERT(!((newFlag) & ~((1 << (regName##_SIZE)) - 1)));                                  \
    (regOrg) = (((u32)(regOrg) & ~(regName##_MASK)) |                                              \
                (((u32)(newFlag) << (regName##_SHIFT)) & (regName##_MASK)));                       \
  } while (0)

#define GX_GENMODE_ID 0
#define GX_GENMODE_REG_ID_SIZE 8
#define GX_GENMODE_REG_ID_SHIFT 24
#define GX_GENMODE_REG_ID_MASK 0xff000000
#define GX_GENMODE_GET_REG_ID(genMode)                                                             \
  ((((u32)(genMode)) & GX_GENMODE_REG_ID_MASK) >> GX_GENMODE_REG_ID_SHIFT)

#define GX_BPMASK_ID 15
#define GX_BPMASK_REG_ID_SIZE 8
#define GX_BPMASK_REG_ID_SHIFT 24
#define GX_BPMASK_REG_ID_MASK 0xff000000

#define GX_LPSIZE_ID 34
#define GX_LPSIZE_REG_ID_SIZE 8
#define GX_LPSIZE_REG_ID_SHIFT 24
#define GX_LPSIZE_REG_ID_MASK 0xff000000

#define TEV_COLOR_ENV_REG_ID_SIZE 8
#define TEV_COLOR_ENV_REG_ID_SHIFT 24
#define TEV_COLOR_ENV_REG_ID_MASK 0xff000000

#define TEV_ALPHA_ENV_REG_ID_SIZE 8
#define TEV_ALPHA_ENV_REG_ID_SHIFT 24
#define TEV_ALPHA_ENV_REG_ID_MASK 0xff000000

#define TEV_COLOR_ENV_0_ID 0x000000c0
#define TEV_ALPHA_ENV_0_ID 0x000000c1
#define TEV_COLOR_ENV_1_ID 0x000000c2
#define TEV_ALPHA_ENV_1_ID 0x000000c3
#define TEV_COLOR_ENV_2_ID 0x000000c4
#define TEV_ALPHA_ENV_2_ID 0x000000c5
#define TEV_COLOR_ENV_3_ID 0x000000c6
#define TEV_ALPHA_ENV_3_ID 0x000000c7
#define TEV_COLOR_ENV_4_ID 0x000000c8
#define TEV_ALPHA_ENV_4_ID 0x000000c9
#define TEV_COLOR_ENV_5_ID 0x000000ca
#define TEV_ALPHA_ENV_5_ID 0x000000cb
#define TEV_COLOR_ENV_6_ID 0x000000cc
#define TEV_ALPHA_ENV_6_ID 0x000000cd
#define TEV_COLOR_ENV_7_ID 0x000000ce
#define TEV_ALPHA_ENV_7_ID 0x000000cf
#define TEV_COLOR_ENV_8_ID 0x000000d0
#define TEV_ALPHA_ENV_8_ID 0x000000d1
#define TEV_COLOR_ENV_9_ID 0x000000d2
#define TEV_ALPHA_ENV_9_ID 0x000000d3
#define TEV_COLOR_ENV_A_ID 0x000000d4
#define TEV_ALPHA_ENV_A_ID 0x000000d5
#define TEV_COLOR_ENV_B_ID 0x000000d6
#define TEV_ALPHA_ENV_B_ID 0x000000d7
#define TEV_COLOR_ENV_C_ID 0x000000d8
#define TEV_ALPHA_ENV_C_ID 0x000000d9
#define TEV_COLOR_ENV_D_ID 0x000000da
#define TEV_ALPHA_ENV_D_ID 0x000000db
#define TEV_COLOR_ENV_E_ID 0x000000dc
#define TEV_ALPHA_ENV_E_ID 0x000000dd
#define TEV_COLOR_ENV_F_ID 0x000000de
#define TEV_ALPHA_ENV_F_ID 0x000000df

#define TEV_KSEL_REG_ID_SIZE 8
#define TEV_KSEL_REG_ID_SHIFT 24
#define TEV_KSEL_REG_ID_MASK 0xff000000

#define TEV_KSEL_0_ID 0x000000f6
#define TEV_KSEL_1_ID 0x000000f7
#define TEV_KSEL_2_ID 0x000000f8
#define TEV_KSEL_3_ID 0x000000f9
#define TEV_KSEL_4_ID 0x000000fa
#define TEV_KSEL_5_ID 0x000000fb
#define TEV_KSEL_6_ID 0x000000fc
#define TEV_KSEL_7_ID 0x000000fd

#define RAS_IREF_ID 39
#define RAS_IREF_REG_ID_SIZE 8
#define RAS_IREF_REG_ID_SHIFT 24
#define RAS_IREF_REG_ID_MASK 0xff000000

#define RAS_TREF0_ID 40
#define RAS_TREF_REG_ID_SIZE 8
#define RAS_TREF_REG_ID_SHIFT 24
#define RAS_TREF_REG_ID_MASK 0xff000000

#define SU_TS0_REG_ID_SIZE 8
#define SU_TS0_REG_ID_SHIFT 24
#define SU_TS0_REG_ID_MASK 0xff000000

#define SU_TS1_REG_ID_SIZE 8
#define SU_TS1_REG_ID_SHIFT 24
#define SU_TS1_REG_ID_MASK 0xff000000

#define SU_SCIS0_ID 0x00000020
#define SU_SCIS1_ID 0x00000021

#define SU_SCIS0_REG_ID_SIZE 8
#define SU_SCIS0_REG_ID_SHIFT 24
#define SU_SCIS0_REG_ID_MASK 0xff000000

#define SU_SCIS1_REG_ID_SIZE 8
#define SU_SCIS1_REG_ID_SHIFT 24
#define SU_SCIS1_REG_ID_MASK 0xff000000

#define SU_SSIZE0_ID 0x00000030
#define SU_TSIZE0_ID 0x00000031
#define SU_SSIZE1_ID 0x00000032
#define SU_TSIZE1_ID 0x00000033
#define SU_SSIZE2_ID 0x00000034
#define SU_TSIZE2_ID 0x00000035
#define SU_SSIZE3_ID 0x00000036
#define SU_TSIZE3_ID 0x00000037
#define SU_SSIZE4_ID 0x00000038
#define SU_TSIZE4_ID 0x00000039
#define SU_SSIZE5_ID 0x0000003a
#define SU_TSIZE5_ID 0x0000003b
#define SU_SSIZE6_ID 0x0000003c
#define SU_TSIZE6_ID 0x0000003d
#define SU_SSIZE7_ID 0x0000003e
#define SU_TSIZE7_ID 0x0000003f

#define GX_ZMODE_ID 64
#define GX_ZMODE_REG_ID_SIZE 8
#define GX_ZMODE_REG_ID_SHIFT 24
#define GX_ZMODE_REG_ID_MASK 0xff000000

#define GX_CMODE0_ID 65
#define GX_CMODE0_REG_ID_SIZE 8
#define GX_CMODE0_REG_ID_SHIFT 24
#define GX_CMODE0_REG_ID_MASK 0xff000000

#define GX_CMODE1_ID 66
#define GX_CMODE1_REG_ID_SIZE 8
#define GX_CMODE1_REG_ID_SHIFT 24
#define GX_CMODE1_REG_ID_MASK 0xff000000

#define PE_CONTROL_ID 67
#define PE_CONTROL_REG_ID_SIZE 8
#define PE_CONTROL_REG_ID_SHIFT 24
#define PE_CONTROL_REG_ID_MASK 0xff000000

#define PE_COPY_CMD_GAMMA_SIZE 2
#define PE_COPY_CMD_GAMMA_SHIFT 7
#define PE_COPY_CMD_GAMMA_MASK 0x00000180

#define GEN_MODE_REG_ID_SIZE 8
#define GEN_MODE_REG_ID_SHIFT 24
#define GEN_MODE_REG_ID_MASK 0xff000000

#define GX_OPCODE_INDEX_SIZE 3
#define GX_OPCODE_INDEX_SHIFT 0
#define GX_OPCODE_INDEX_MASK 0x00000007

#define GX_OPCODE_CMD_SHIFT 3

#define GX_OPCODE(index, cmd)                                                                      \
  ((((u32)(index)) << GX_OPCODE_INDEX_SHIFT) | (((u32)(cmd)) << GX_OPCODE_CMD_SHIFT))

#ifdef _DEBUG
#define GX_WRITE_RA_REG(reg)                                                                       \
  {                                                                                                \
    GX_WRITE_U8(GX_OPCODE(1, 12));                                                                 \
    GX_WRITE_U32((reg));                                                                           \
    __gxVerif->rasRegs[GX_GENMODE_GET_REG_ID(reg)] = reg;                                          \
  }
#else
#define GX_WRITE_RA_REG(reg)                                                                       \
  {                                                                                                \
    GX_WRITE_U8(GX_OPCODE(1, 12));                                                                 \
    GX_WRITE_U32((reg));                                                                           \
  }
#endif

#define GX_BITFIELD(field, pos, size, value)                                                       \
  (__rlwimi((field), (value), 31 - (pos) - (size) + 1, (pos), (pos) + (size) - 1))
#define GX_BITFIELD_SET(field, pos, size, value) ((field) = GX_BITFIELD(field, pos, size, value))

#define CP_STREAM_REG_INDEX_SIZE 4
#define CP_STREAM_REG_INDEX_SHIFT 0
#define CP_STREAM_REG_INDEX_MASK 0x0000000f

#define CP_STREAM_REG_ADDR_SIZE 4
#define CP_STREAM_REG_ADDR_SHIFT 4
#define CP_STREAM_REG_ADDR_MASK 0x000000f0

#define CP_STREAM_REG(index, addr)                                                                 \
  ((((unsigned long)(index)) << CP_STREAM_REG_INDEX_SHIFT) |                                       \
   (((unsigned long)(addr)) << CP_STREAM_REG_ADDR_SHIFT))

#ifdef _DEBUG
#define GX_WRITE_CP_STRM_REG(addr, vtxfmt, data)                                                   \
  {                                                                                                \
    s32 regAddr;                                                                                   \
    GX_WRITE_U8(GX_OPCODE(0, 1));                                                                  \
    GX_WRITE_U8(CP_STREAM_REG((vtxfmt), (addr)));                                                  \
    GX_WRITE_U32((data));                                                                          \
    regAddr = (vtxfmt) - GX_POS_MTX_ARRAY + GX_VA_POS;                                             \
    if ((addr) == 10) {                                                                            \
      if (regAddr >= 0 && regAddr < 4)                                                             \
        __GXData->indexBase[regAddr] = (data);                                                     \
    } else if ((addr) == 11) {                                                                     \
      if (regAddr >= 0 && regAddr < 4)                                                             \
        __GXData->indexStride[regAddr] = (data);                                                   \
    }                                                                                              \
  }
#else
#define GX_WRITE_CP_STRM_REG(addr, vtxfmt, data)                                                   \
  {                                                                                                \
    GX_WRITE_U8(GX_OPCODE(0, 1));                                                                  \
    GX_WRITE_U8(CP_STREAM_REG((vtxfmt), (addr)));                                                  \
    GX_WRITE_U32((data));                                                                          \
  }
#endif

#define PE_REFRESH_REG_ID_SIZE 8
#define PE_REFRESH_REG_ID_SHIFT 24
#define PE_REFRESH_REG_ID_MASK 0xff000000

#define PE_REFRESH_INTERVAL_SHIFT 0
#define PE_REFRESH_ENABLE_SHIFT 9
#define PE_REFRESH_TOTAL_SIZE 32
#define PE_REFRESH(interval, enable, rid)                                                          \
  ((((u32)(interval)) << PE_REFRESH_INTERVAL_SHIFT) |                                              \
   (((u32)(enable)) << PE_REFRESH_ENABLE_SHIFT) | (((u32)(rid)) << PE_REFRESH_REG_ID_SHIFT))

#define TX_REFRESH_REG_ID_SIZE 8
#define TX_REFRESH_REG_ID_SHIFT 24
#define TX_REFRESH_REG_ID_MASK 0xff000000

#define TX_REFRESH_INTERVAL_SHIFT 0
#define TX_REFRESH_ENABLE_SHIFT 10
#define TX_REFRESH_TOTAL_SIZE 32
#define TX_REFRESH(interval, enable, rid)                                                          \
  ((((u32)(interval)) << TX_REFRESH_INTERVAL_SHIFT) |                                              \
   (((u32)(enable)) << TX_REFRESH_ENABLE_SHIFT) | (((u32)(rid)) << TX_REFRESH_REG_ID_SHIFT))

#define GX_VAT_REG_A_UNK_SIZE 1
#define GX_VAT_REG_A_UNK_SHIFT 30
#define GX_VAT_REG_A_UNK_MASK 0x40000000

#define GX_VAT_REG_B_UNK_SIZE 1
#define GX_VAT_REG_B_UNK_SHIFT 31
#define GX_VAT_REG_B_UNK_MASK 0x80000000

#endif // _DOLPHIN_GXPRIV
