#pragma once

#ifndef GX_IS_WII
#define GX_IS_WII 0
#endif

#include "../GCNTypes.hpp"

#include <bit>
#include <bitset>
#include <memory>

#include <zeus/CColor.hpp>
#include <zeus/CMatrix4f.hpp>
#include <zeus/CTransform.hpp>

namespace GX {
#if GX_IS_WII
constexpr float LARGE_NUMBER = -1.0e+18f;
#else
constexpr float LARGE_NUMBER = -1048576.0f;
#endif

enum Attr {
  VA_PNMTXIDX = 0x0,
  VA_TEX0MTXIDX = 0x1,
  VA_TEX1MTXIDX = 0x2,
  VA_TEX2MTXIDX = 0x3,
  VA_TEX3MTXIDX = 0x4,
  VA_TEX4MTXIDX = 0x5,
  VA_TEX5MTXIDX = 0x6,
  VA_TEX6MTXIDX = 0x7,
  VA_TEX7MTXIDX = 0x8,
  VA_POS = 0x9,
  VA_NRM = 0xa,
  VA_CLR0 = 0xb,
  VA_CLR1 = 0xc,
  VA_TEX0 = 0xd,
  VA_TEX1 = 0xe,
  VA_TEX2 = 0xf,
  VA_TEX3 = 0x10,
  VA_TEX4 = 0x11,
  VA_TEX5 = 0x12,
  VA_TEX6 = 0x13,
  VA_TEX7 = 0x14,
  POS_MTX_ARRAY = 0x15,
  NRM_MTX_ARRAY = 0x16,
  TEX_MTX_ARRAY = 0x17,
  LIGHT_ARRAY = 0x18,
  VA_NBT = 0x19,
  VA_MAX_ATTR = 0x1a,
  VA_NULL = 0xff,
};

enum AttrType {
  NONE,
  DIRECT,
  INDEX8,
  INDEX16,
};

struct VtxDescList {
  Attr attr = GX::VA_NULL;
  AttrType type = GX::NONE;
  operator bool() const { return attr != GX::VA_NULL; }
};

enum VtxFmt {
  VTXFMT0 = 0,
  VTXFMT1,
  VTXFMT2,
  VTXFMT3,
  VTXFMT4,
  VTXFMT5,
  VTXFMT6,
  VTXFMT7,
  MAX_VTXFMT,
};

enum TevColorArg {
  CC_CPREV = 0,
  CC_APREV = 1,
  CC_C0 = 2,
  CC_A0 = 3,
  CC_C1 = 4,
  CC_A1 = 5,
  CC_C2 = 6,
  CC_A2 = 7,
  CC_TEXC = 8,
  CC_TEXA = 9,
  CC_RASC = 10,
  CC_RASA = 11,
  CC_ONE = 12,
  CC_HALF = 13,
  CC_KONST = 14,
  CC_ZERO = 15,
};

enum TevAlphaArg {
  CA_APREV = 0,
  CA_A0 = 1,
  CA_A1 = 2,
  CA_A2 = 3,
  CA_TEXA = 4,
  CA_RASA = 5,
  CA_KONST = 6,
  CA_ZERO = 7,
};

enum TevKColorID {
  KCOLOR0 = 0,
  KCOLOR1,
  KCOLOR2,
  KCOLOR3,
  MAX_KCOLOR,
};

enum TevKColorSel {
  TEV_KCSEL_8_8 = 0x00,
  TEV_KCSEL_7_8 = 0x01,
  TEV_KCSEL_6_8 = 0x02,
  TEV_KCSEL_5_8 = 0x03,
  TEV_KCSEL_4_8 = 0x04,
  TEV_KCSEL_3_8 = 0x05,
  TEV_KCSEL_2_8 = 0x06,
  TEV_KCSEL_1_8 = 0x07,

  TEV_KCSEL_1 = TEV_KCSEL_8_8,
  TEV_KCSEL_3_4 = TEV_KCSEL_6_8,
  TEV_KCSEL_1_2 = TEV_KCSEL_4_8,
  TEV_KCSEL_1_4 = TEV_KCSEL_2_8,

  TEV_KCSEL_K0 = 0x0C,
  TEV_KCSEL_K1 = 0x0D,
  TEV_KCSEL_K2 = 0x0E,
  TEV_KCSEL_K3 = 0x0F,
  TEV_KCSEL_K0_R = 0x10,
  TEV_KCSEL_K1_R = 0x11,
  TEV_KCSEL_K2_R = 0x12,
  TEV_KCSEL_K3_R = 0x13,
  TEV_KCSEL_K0_G = 0x14,
  TEV_KCSEL_K1_G = 0x15,
  TEV_KCSEL_K2_G = 0x16,
  TEV_KCSEL_K3_G = 0x17,
  TEV_KCSEL_K0_B = 0x18,
  TEV_KCSEL_K1_B = 0x19,
  TEV_KCSEL_K2_B = 0x1A,
  TEV_KCSEL_K3_B = 0x1B,
  TEV_KCSEL_K0_A = 0x1C,
  TEV_KCSEL_K1_A = 0x1D,
  TEV_KCSEL_K2_A = 0x1E,
  TEV_KCSEL_K3_A = 0x1F,
  INVALID_KCSEL = 0xFF
};

enum TevKAlphaSel {
  TEV_KASEL_8_8 = 0x00,
  TEV_KASEL_7_8 = 0x01,
  TEV_KASEL_6_8 = 0x02,
  TEV_KASEL_5_8 = 0x03,
  TEV_KASEL_4_8 = 0x04,
  TEV_KASEL_3_8 = 0x05,
  TEV_KASEL_2_8 = 0x06,
  TEV_KASEL_1_8 = 0x07,

  TEV_KASEL_1 = TEV_KASEL_8_8,
  TEV_KASEL_3_4 = TEV_KASEL_6_8,
  TEV_KASEL_1_2 = TEV_KASEL_4_8,
  TEV_KASEL_1_4 = TEV_KASEL_2_8,

  TEV_KASEL_K0_R = 0x10,
  TEV_KASEL_K1_R = 0x11,
  TEV_KASEL_K2_R = 0x12,
  TEV_KASEL_K3_R = 0x13,
  TEV_KASEL_K0_G = 0x14,
  TEV_KASEL_K1_G = 0x15,
  TEV_KASEL_K2_G = 0x16,
  TEV_KASEL_K3_G = 0x17,
  TEV_KASEL_K0_B = 0x18,
  TEV_KASEL_K1_B = 0x19,
  TEV_KASEL_K2_B = 0x1A,
  TEV_KASEL_K3_B = 0x1B,
  TEV_KASEL_K0_A = 0x1C,
  TEV_KASEL_K1_A = 0x1D,
  TEV_KASEL_K2_A = 0x1E,
  TEV_KASEL_K3_A = 0x1F,
  INVALID_KASEL = 0xFF
};

enum TevOp {
  TEV_ADD = 0,
  TEV_SUB = 1,
  TEV_COMP_R8_GT = 8,
  TEV_COMP_R8_EQ = 9,
  TEV_COMP_GR16_GT = 10,
  TEV_COMP_GR16_EQ = 11,
  TEV_COMP_BGR24_GT = 12,
  TEV_COMP_BGR24_EQ = 13,
  TEV_COMP_RGB8_GT = 14,
  TEV_COMP_RGB8_EQ = 15,
  TEV_COMP_A8_GT = TEV_COMP_RGB8_GT,
  TEV_COMP_A8_EQ = TEV_COMP_RGB8_EQ
};

enum TevBias {
  TB_ZERO = 0,
  TB_ADDHALF = 1,
  TB_SUBHALF = 2,
};

enum TevScale {
  CS_SCALE_1 = 0,
  CS_SCALE_2 = 1,
  CS_SCALE_4 = 2,
  CS_DIVIDE_2 = 3,
};

enum TexOffset {
  TO_ZERO,
  TO_SIXTEENTH,
  TO_EIGHTH,
  TO_FOURTH,
  TO_HALF,
  TO_ONE,
  MAX_TEXOFFSET,
};

enum TexGenType {
  TG_MTX3x4 = 0,
  TG_MTX2x4,
  TG_BUMP0,
  TG_BUMP1,
  TG_BUMP2,
  TG_BUMP3,
  TG_BUMP4,
  TG_BUMP5,
  TG_BUMP6,
  TG_BUMP7,
  TG_SRTG
};

enum TexGenSrc {
  TG_POS = 0,
  TG_NRM,
  TG_BINRM,
  TG_TANGENT,
  TG_TEX0,
  TG_TEX1,
  TG_TEX2,
  TG_TEX3,
  TG_TEX4,
  TG_TEX5,
  TG_TEX6,
  TG_TEX7,
  TG_TEXCOORD0,
  TG_TEXCOORD1,
  TG_TEXCOORD2,
  TG_TEXCOORD3,
  TG_TEXCOORD4,
  TG_TEXCOORD5,
  TG_TEXCOORD6,
  TG_COLOR0,
  TG_COLOR1,
  MAX_TEXGENSRC = 0xFF,
};

enum TexMtx {
  TEXMTX0 = 30,
  TEXMTX1 = 33,
  TEXMTX2 = 36,
  TEXMTX3 = 39,
  TEXMTX4 = 42,
  TEXMTX5 = 45,
  TEXMTX6 = 48,
  TEXMTX7 = 51,
  TEXMTX8 = 54,
  TEXMTX9 = 57,
  IDENTITY = 60
};

enum PTTexMtx {
  PTTEXMTX0 = 64,
  PTTEXMTX1 = 67,
  PTTEXMTX2 = 70,
  PTTEXMTX3 = 73,
  PTTEXMTX4 = 76,
  PTTEXMTX5 = 79,
  PTTEXMTX6 = 82,
  PTTEXMTX7 = 85,
  PTTEXMTX8 = 88,
  PTTEXMTX9 = 91,
  PTTEXMTX10 = 94,
  PTTEXMTX11 = 97,
  PTTEXMTX12 = 100,
  PTTEXMTX13 = 103,
  PTTEXMTX14 = 106,
  PTTEXMTX15 = 109,
  PTTEXMTX16 = 112,
  PTTEXMTX17 = 115,
  PTTEXMTX18 = 118,
  PTTEXMTX19 = 121,
  PTIDENTITY = 125
};

enum TexCoordID {
  TEXCOORD0 = 0x0,
  TEXCOORD1,
  TEXCOORD2,
  TEXCOORD3,
  TEXCOORD4,
  TEXCOORD5,
  TEXCOORD6,
  TEXCOORD7,
  MAX_TEXCOORD = 8,
  TEXCOORD_NULL = 0xff
};

enum TevSwapSel {
  TEV_SWAP0 = 0x0,
  TEV_SWAP1 = 0x1,
  TEV_SWAP2 = 0x2,
  TEV_SWAP3 = 0x3,
  MAX_TEVSWAP = 0x4,
};

enum TevColorChan {
  CH_RED = 0x0,
  CH_GREEN = 0x1,
  CH_BLUE = 0x2,
  CH_ALPHA = 0x3,
};

enum TevRegID {
  TEVPREV = 0,
  TEVREG0 = 1,
  TEVREG1 = 2,
  TEVREG2 = 3,
  TEVLAZY = 5,
};

enum DiffuseFn {
  DF_NONE = 0,
  DF_SIGN = 1,
  DF_CLAMP = 2,
};

enum AttnFn {
  AF_SPEC = 0,
  AF_SPOT = 1,
  AF_NONE = 2,
};

enum Primitive {
  POINTS = 0xb8,
  LINES = 0xa8,
  LINESTRIP = 0xb0,
  TRIANGLES = 0x90,
  TRIANGLESTRIP = 0x98,
  TRIANGLEFAN = 0xa0,
  QUADS = 0x80
};

enum ChannelID {
  COLOR0,
  COLOR1,
  ALPHA0,
  ALPHA1,
  COLOR0A0,
  COLOR1A1,
  COLOR_ZERO,
  ALPHA_BUMP,
  ALPHA_BUMPN,
  COLOR_NULL = 0xff
};

enum BlendMode {
  BM_NONE,
  BM_BLEND,
  BM_LOGIC,
  BM_SUBTRACT,
  MAX_BLENDMODE,
};

enum LogicOp {
  LO_CLEAR,
  LO_AND,
  LO_REVAND,
  LO_COPY,
  LO_INVAND,
  LO_NOOP,
  LO_XOR,
  LO_OR,
  LO_NOR,
  LO_EQUIV,
  LO_INV,
  LO_REVOR,
  LO_INVCOPY,
  LO_INVOR,
  LO_NAND,
  LO_SET
};

enum AlphaOp {
  AOP_AND,
  AOP_OR,
  AOP_XOR,
  AOP_XNOR,
  MAX_ALPHAOP,
};

enum ZTexOp {
  ZT_DISABLE,
  ZT_ADD,
  ZT_REPLACE,
  MAX_ZTEXOP,
};

enum Compare {
  NEVER,
  LESS,
  EQUAL,
  LEQUAL,
  GREATER,
  NEQUAL,
  GEQUAL,
  ALWAYS,
};

enum BlendFactor {
  BL_ZERO,
  BL_ONE,
  BL_SRCCLR,
  BL_INVSRCCLR,
  BL_SRCALPHA,
  BL_INVSRCALPHA,
  BL_DSTALPHA,
  BL_INVDSTALPHA,
  BL_DSTCLR,
  BL_INVDSTCLR,
};

enum CullMode {
  CULL_NONE,
  CULL_FRONT,
  CULL_BACK,
  CULL_ALL,
};

enum TextureFormat {
  TF_I4 = 0x0,
  TF_I8 = 0x1,
  TF_IA4 = 0x2,
  TF_IA8 = 0x3,
  TF_RGB565 = 0x4,
  TF_RGB5A3 = 0x5,
  TF_RGBA8 = 0x6,
  TF_C4 = 0x8,
  TF_C8 = 0x9,
  TF_C14X2 = 0xa,
  TF_CMPR = 0xE,
  TF_Z8 = 0x11,
  TF_Z16 = 0x13,
  TF_Z24X8 = 0x16,
  CTF_R4 = 0x20,
  CTF_RA4 = 0x22,
  CTF_RA8 = 0x23,
  CTF_YUVA8 = 0x26,
  CTF_A8 = 0x27,
  CTF_R8 = 0x28,
  CTF_G8 = 0x29,
  CTF_B8 = 0x2a,
  CTF_RG8 = 0x2b,
  CTF_GB8 = 0x2c,
  CTF_Z4 = 0x30,
  CTF_Z8M = 0x39,
  CTF_Z8L = 0x3a,
  CTF_Z16L = 0x3c,
};

enum TexMapID {
  TEXMAP0,
  TEXMAP1,
  TEXMAP2,
  TEXMAP3,
  TEXMAP4,
  TEXMAP5,
  TEXMAP6,
  TEXMAP7,
  MAX_TEXMAP,
  TEXMAP_NULL = 0xFF,
  TEX_DISABLE = 0x100,
};

enum TevStageID {
  TEVSTAGE0,
  TEVSTAGE1,
  TEVSTAGE2,
  TEVSTAGE3,
  TEVSTAGE4,
  TEVSTAGE5,
  TEVSTAGE6,
  TEVSTAGE7,
  TEVSTAGE8,
  TEVSTAGE9,
  TEVSTAGE10,
  TEVSTAGE11,
  TEVSTAGE12,
  TEVSTAGE13,
  TEVSTAGE14,
  TEVSTAGE15,
  MAX_TEVSTAGE,
  NULL_STAGE = -1,
};

enum IndTexFormat {
  ITF_8,
  ITF_5,
  ITF_4,
  ITF_3,
  MAX_ITFORMAT,
};

enum IndTexBiasSel {
  ITB_NONE,
  ITB_S,
  ITB_T,
  ITB_ST,
  ITB_U,
  ITB_SU,
  ITB_TU,
  ITB_STU,
  MAX_ITBIAS,
};

enum IndTexWrap {
  ITW_OFF,
  ITW_256,
  ITW_128,
  ITW_64,
  ITW_32,
  ITW_16,
  ITW_0,
  MAX_ITWRAP,
};

enum IndTexAlphaSel {
  ITBA_OFF,
  ITBA_S,
  ITBA_T,
  ITBA_U,
  MAX_ITBALPHA,
};

enum IndTexStageID {
  INDTEXSTAGE0,
  INDTEXSTAGE1,
  INDTEXSTAGE2,
  INDTEXSTAGE3,
  MAX_INDTEXSTAGE,
};

enum IndTexScale {
  ITS_1,
  ITS_2,
  ITS_4,
  ITS_8,
  ITS_16,
  ITS_32,
  ITS_64,
  ITS_128,
  ITS_256,
  MAX_ITSCALE,
};

enum IndTexMtxID {
  ITM_OFF,
  ITM_0,
  ITM_1,
  ITM_2,
  ITM_S0 = 5,
  ITM_S1,
  ITM_S2,
  ITM_T0 = 9,
  ITM_T1,
  ITM_T2,
};

enum TexMtxType {
  MTX3x4 = 0,
  MTX2x4,
};

struct Color {
  union {
    uint8_t color[4];
    uint32_t num = 0;
  };
  Color() = default;
  Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;
  }

  Color(float r, float g, float b, float a = 1.f) {
    color[0] = r * 255;
    color[1] = g * 255;
    color[2] = b * 255;
    color[3] = a * 255;
  }

  Color& operator=(uint8_t val) {
    color[0] = val;
    color[1] = val;
    color[2] = val;
    color[3] = val;
    return *this;
  }

  explicit Color(uint32_t val) { *this = val; }
  bool operator==(const Color& other) const { return num == other.num; }
  bool operator!=(const Color& other) const { return num != other.num; }
  uint8_t operator[](size_t idx) const { return color[idx]; }
  uint8_t& operator[](size_t idx) { return color[idx]; }
};

enum ColorSrc {
  SRC_REG = 0,
  SRC_VTX,
};

enum LightID : u32 {
  LIGHT0 = 0x001,
  LIGHT1 = 0x002,
  LIGHT2 = 0x004,
  LIGHT3 = 0x008,
  LIGHT4 = 0x010,
  LIGHT5 = 0x020,
  LIGHT6 = 0x040,
  LIGHT7 = 0x080,
  MAX_LIGHT = 0x100,
  LIGHT_NULL = 0x000,
};
constexpr u8 MaxLights = 8;
using LightMask = std::bitset<MaxLights>;

enum FogType {
  FOG_NONE = 0x00,
  FOG_PERSP_LIN = 0x02,
  FOG_PERSP_EXP = 0x04,
  FOG_PERSP_EXP2 = 0x05,
  FOG_PERSP_REVEXP = 0x06,
  FOG_PERSP_REVEXP2 = 0x07,
  FOG_ORTHO_LIN = 0x0A,
  FOG_ORTHO_EXP = 0x0C,
  FOG_ORTHO_EXP2 = 0x0D,
  FOG_ORTHO_REVEXP = 0x0E,
  FOG_ORTHO_REVEXP2 = 0x0F,
};

enum PosNrmMtx {
  PNMTX0,
  PNMTX1,
  PNMTX2,
  PNMTX3,
  PNMTX4,
  PNMTX5,
  PNMTX6,
  PNMTX7,
  PNMTX8,
  PNMTX9,
};

enum ProjectionType {
  PERSPECTIVE,
  ORTHOGRAPHIC,
};

enum CompCnt {
  CLR_RGB = 0,
  POS_XY = 0,
  TEX_S = 0,
  NRM_XYZ = 0,
  CLR_RGBA = 1,
  POS_XYZ = 1,
  NRM_NBT = 1,
  TEX_ST = 1,
  NRM_NBT3 = 2,
};

enum CompType {
  RGB565 = 0,
  U8 = 0,
  RGB8 = 1,
  S8 = 1,
  U16 = 2,
  RGBX8 = 2,
  S16 = 3,
  RGBA4 = 3,
  F32 = 4,
  RGBA6 = 4,
  RGBA8 = 5,
};

struct LightObj {
  u32 __padding[3];
  GX::Color color;
  float a0 = 1.f;
  float a1 = 0.f;
  float a2 = 0.f;
  float k0 = 1.f;
  float k1 = 0.f;
  float k2 = 0.f;
  float px = 0.f;
  float py = 0.f;
  float pz = 0.f;
  float nx = 0.f;
  float ny = 0.f;
  float nz = 0.f;
};

enum SpotFn {
  SP_OFF,
  SP_FLAT,
  SP_COS,
  SP_COS2,
  SP_SHARP,
  SP_RING1,
  SP_RING2,
};

enum DistAttnFn {
  DA_OFF,
  DA_GENTLE,
  DA_MEDIUM,
  DA_STEEP,
};
} // namespace GX

using GXColor = zeus::CColor;
using GXBool = bool;

struct GXColorS10 {
  s16 r;
  s16 g;
  s16 b;
  s16 a;

  constexpr GXColorS10(u64 c) noexcept
  : r(static_cast<s16>((c >> 48) & 0xFFFF))
  , g(static_cast<s16>((c >> 32) & 0xFFFF))
  , b(static_cast<s16>((c >> 16) & 0xFFFF))
  , a(static_cast<s16>(c & 0xFFFF)) {}
};

enum GXTlutFmt {
  GX_TL_IA8 = 0x0,
  GX_TL_RGB565 = 0x1,
  GX_TL_RGB5A3 = 0x2,
  GX_MAX_TLUTFMT = 0x3,
};

namespace aurora::gfx {
struct TextureRef;
} // namespace aurora::gfx
struct GXTlutObj {
  std::shared_ptr<aurora::gfx::TextureRef> ref;
};

enum GXTlut {
  GX_TLUT0 = 0,
  GX_TLUT1 = 1,
  GX_TLUT2 = 2,
  GX_TLUT3 = 3,
  GX_TLUT4 = 4,
  GX_TLUT5 = 5,
  GX_TLUT6 = 6,
  GX_TLUT7 = 7,
  GX_TLUT8 = 8,
  GX_TLUT9 = 9,
  GX_TLUT10 = 10,
  GX_TLUT11 = 11,
  GX_TLUT12 = 12,
  GX_TLUT13 = 13,
  GX_TLUT14 = 14,
  GX_TLUT15 = 15,
  GX_BIGTLUT0 = 16,
  GX_BIGTLUT1 = 17,
  GX_BIGTLUT2 = 18,
  GX_BIGTLUT3 = 19,
};

enum GXTexWrapMode {
  GX_CLAMP = 0,
  GX_REPEAT = 1,
  GX_MIRROR = 2,
  GX_MAX_TEXWRAPMODE = 3,
};

enum GXTexFilter {
  GX_NEAR,
  GX_LINEAR,
  GX_NEAR_MIP_NEAR,
  GX_LIN_MIP_NEAR,
  GX_NEAR_MIP_LIN,
  GX_LIN_MIP_LIN,
};

enum GXAnisotropy {
  GX_ANISO_1,
  GX_ANISO_2,
  GX_ANISO_4,
  GX_MAX_ANISOTROPY,
};

enum GXCITexFmt {
  GX_TF_C4 = GX::TF_C4,
  GX_TF_C8 = GX::TF_C8,
  GX_TF_C14X2 = GX::TF_C14X2,
};

struct GXTexObj {
  std::shared_ptr<aurora::gfx::TextureRef> ref;
  const void* data;
  u32 dataSize;
  u16 width;
  u16 height;
  GX::TextureFormat fmt;
  GXTexWrapMode wrapS;
  GXTexWrapMode wrapT;
  GXBool hasMips;
  GXTexFilter minFilter;
  GXTexFilter magFilter;
  float minLod;
  float maxLod;
  float lodBias;
  GXBool biasClamp;
  GXBool doEdgeLod;
  GXAnisotropy maxAniso;
  GXTlut tlut;
  bool dataInvalidated;
};

void GXSetNumChans(u8 num) noexcept;
void GXSetNumIndStages(u8 num) noexcept;
void GXSetNumTevStages(u8 num) noexcept;
void GXSetNumTexGens(u8 num) noexcept;
void GXSetTevAlphaIn(GX::TevStageID stageId, GX::TevAlphaArg a, GX::TevAlphaArg b, GX::TevAlphaArg c,
                     GX::TevAlphaArg d) noexcept;
void GXSetTevAlphaOp(GX::TevStageID stageId, GX::TevOp op, GX::TevBias bias, GX::TevScale scale, GXBool clamp,
                     GX::TevRegID outReg) noexcept;
void GXSetTevColorIn(GX::TevStageID stageId, GX::TevColorArg a, GX::TevColorArg b, GX::TevColorArg c,
                     GX::TevColorArg d) noexcept;
void GXSetTevColorOp(GX::TevStageID stageId, GX::TevOp op, GX::TevBias bias, GX::TevScale scale, GXBool clamp,
                     GX::TevRegID outReg) noexcept;
void GXSetCullMode(GX::CullMode mode) noexcept;
void GXSetBlendMode(GX::BlendMode mode, GX::BlendFactor src, GX::BlendFactor dst, GX::LogicOp op) noexcept;
void GXSetZMode(GXBool compare_enable, GX::Compare func, GXBool update_enable) noexcept;
void GXSetTevColor(GX::TevRegID id, const GXColor& color) noexcept;
void GXSetTevColorS10(GX::TevRegID id, const GXColorS10& color) noexcept;
void GXSetTevKColor(GX::TevKColorID id, const GXColor& color) noexcept;
void GXSetColorUpdate(GXBool enabled) noexcept;
void GXSetAlphaUpdate(GXBool enabled) noexcept;
void GXSetDstAlpha(GXBool enabled, u8 value) noexcept;
void GXSetCopyClear(const GXColor& color, float depth) noexcept;
void GXSetTevOrder(GX::TevStageID id, GX::TexCoordID tcid, GX::TexMapID tmid, GX::ChannelID cid) noexcept;
void GXSetTevKColorSel(GX::TevStageID id, GX::TevKColorSel sel) noexcept;
void GXSetTevKAlphaSel(GX::TevStageID id, GX::TevKAlphaSel sel) noexcept;
void GXSetChanAmbColor(GX::ChannelID id, const GXColor& color) noexcept;
void GXSetChanMatColor(GX::ChannelID id, const GXColor& color) noexcept;
void GXSetChanCtrl(GX::ChannelID id, GXBool lightingEnabled, GX::ColorSrc ambSrc, GX::ColorSrc matSrc,
                   GX::LightMask lightState, GX::DiffuseFn diffFn, GX::AttnFn attnFn) noexcept;
void GXSetAlphaCompare(GX::Compare comp0, u8 ref0, GX::AlphaOp op, GX::Compare comp1, u8 ref1) noexcept;
void GXSetVtxDesc(GX::Attr attr, GX::AttrType type) noexcept;
void GXSetVtxDescv(GX::VtxDescList* list) noexcept;
void GXClearVtxDesc() noexcept;
void GXSetArray(GX::Attr attr, const void* data, u8 stride) noexcept;
void GXSetFog(GX::FogType type, float startZ, float endZ, float nearZ, float farZ, const GXColor& color) noexcept;
void GXSetFogColor(const GXColor& color) noexcept;
void GXCallDisplayList(const void* data, u32 nbytes) noexcept;
void GXSetTexCoordGen2(GX::TexCoordID dst, GX::TexGenType type, GX::TexGenSrc src, GX::TexMtx mtx, GXBool normalize,
                       GX::PTTexMtx postMtx) noexcept;
void GXLoadTexMtxImm(const void* data, u32 id /* GX::TexMtx or GX::PTTexMtx */, GX::TexMtxType type) noexcept;
void GXLoadPosMtxImm(const zeus::CTransform& xf, GX::PosNrmMtx id) noexcept;
void GXLoadNrmMtxImm(const zeus::CTransform& xf, GX::PosNrmMtx id) noexcept;
void GXSetProjection(const zeus::CMatrix4f& mtx, GX::ProjectionType type) noexcept;
void GXSetViewport(float left, float top, float width, float height, float nearZ, float farZ) noexcept;
void GXSetScissor(u32 left, u32 top, u32 width, u32 height) noexcept;
// Unneeded, all attributes are expected to be full floats
static inline void GXSetVtxAttrFmt(GX::VtxFmt vtxfmt, GX::Attr attr, GX::CompCnt cnt, GX::CompType type,
                                   u8 frac) noexcept {}
// Streaming
void GXBegin(GX::Primitive primitive, GX::VtxFmt vtxFmt, u16 nVerts) noexcept;
void GXMatrixIndex1u8(u8 idx) noexcept;
void GXPosition3f32(const zeus::CVector3f& pos) noexcept;
static inline void GXPosition3f32(float x, float y, float z) noexcept { GXPosition3f32({x, y, z}); }
void GXNormal3f32(const zeus::CVector3f& nrm) noexcept;
static inline void GXNormal3f32(float x, float y, float z) noexcept { GXNormal3f32({x, y, z}); }
void GXColor4f32(const zeus::CColor& color) noexcept;
static inline void GXColor4f32(float r, float g, float b, float a) noexcept { GXColor4f32({r, g, b, a}); }
void GXTexCoord2f32(const zeus::CVector2f& uv) noexcept;
static inline void GXTexCoord2f32(float u, float v) noexcept { GXTexCoord2f32({u, v}); }
void GXEnd() noexcept;
// End streaming
void GXSetTevSwapModeTable(GX::TevSwapSel id, GX::TevColorChan red, GX::TevColorChan green, GX::TevColorChan blue,
                           GX::TevColorChan alpha) noexcept;
void GXSetTevSwapMode(GX::TevStageID stage, GX::TevSwapSel rasSel, GX::TevSwapSel texSel) noexcept;
void GXSetLineWidth(u8 width, GX::TexOffset texOffset) noexcept;
void GXInitTlutObj(GXTlutObj* obj, const void* data, GXTlutFmt format, u16 entries) noexcept;
void GXLoadTlut(const GXTlutObj* obj, GXTlut idx) noexcept;
void GXInitTexObj(GXTexObj* obj, const void* data, u16 width, u16 height, GX::TextureFormat format, GXTexWrapMode wrapS,
                  GXTexWrapMode wrapT, GXBool mipmap) noexcept;
// Addition for binding render textures
void GXInitTexObjResolved(GXTexObj* obj, u32 bindIdx, GX::TextureFormat format, GXTexWrapMode wrapS,
                          GXTexWrapMode wrapT, GXTlut tlut);
void GXInitTexObjLOD(GXTexObj* obj, GXTexFilter minFilt, GXTexFilter magFilt, float minLod, float maxLod, float lodBias,
                     GXBool biasClamp, GXBool doEdgeLod, GXAnisotropy maxAniso) noexcept;
void GXInitTexObjCI(GXTexObj* obj, void* data, u16 width, u16 height, GXCITexFmt format, GXTexWrapMode wrapS,
                    GXTexWrapMode wrapT, GXBool mipmap, u32 tlut) noexcept;
void GXInitTexObjData(GXTexObj* obj, void* data) noexcept;
void GXInitTexObjWrapMode(GXTexObj* obj, GXTexWrapMode wrapS, GXTexWrapMode wrapT) noexcept;
void GXInitTexObjTlut(GXTexObj* obj, u32 tlut) noexcept;
void GXLoadTexObj(GXTexObj* obj, GX::TexMapID id) noexcept;
void GXSetTexCopySrc(u16 x, u16 y, u16 w, u16 h) noexcept;
void GXSetTexCopyDst(u16 wd, u16 ht, GX::TextureFormat fmt, GXBool mipmap) noexcept;
u32 GXGetTexBufferSize(u16 width, u16 height, u32 fmt, GXBool mips, u8 maxLod) noexcept;
void GXCopyTex(void* dest, GXBool clear) noexcept;
static inline void GXPixModeSync() noexcept {} // no-op
void GXInvalidateTexAll() noexcept;
void GXSetIndTexMtx(GX::IndTexMtxID id, const void* mtx /* Mat4x2<float> */, s8 scaleExp) noexcept;
void GXSetTevIndirect(GX::TevStageID tevStage, GX::IndTexStageID indStage, GX::IndTexFormat fmt,
                      GX::IndTexBiasSel biasSel, GX::IndTexMtxID matrixSel, GX::IndTexWrap wrapS, GX::IndTexWrap wrapT,
                      GXBool addPrev, GXBool indLod, GX::IndTexAlphaSel alphaSel) noexcept;
static inline void GXSetTevDirect(GX::TevStageID stageId) noexcept {
  GXSetTevIndirect(stageId, GX::INDTEXSTAGE0, GX::ITF_8, GX::ITB_NONE, GX::ITM_OFF, GX::ITW_OFF, GX::ITW_OFF, false,
                   false, GX::ITBA_OFF);
}
static inline void GXSetTevIndWarp(GX::TevStageID tevStage, GX::IndTexStageID indStage, GXBool signedOffsets,
                                   GXBool replaceMode, GX::IndTexMtxID matrixSel) noexcept {
  const auto wrap = replaceMode ? GX::ITW_0 : GX::ITW_OFF;
  const auto biasSel = signedOffsets ? GX::ITB_STU : GX::ITB_NONE;
  GXSetTevIndirect(tevStage, indStage, GX::ITF_8, biasSel, matrixSel, wrap, wrap, false, false, GX::ITBA_OFF);
}
void GXSetIndTexOrder(GX::IndTexStageID indStage, GX::TexCoordID texCoord, GX::TexMapID texMap) noexcept;
void GXSetIndTexCoordScale(GX::IndTexStageID indStage, GX::IndTexScale scaleS, GX::IndTexScale scaleT) noexcept;

// Lighting
void GXInitLightAttn(GX::LightObj* light, float a0, float a1, float a2, float k0, float k1, float k2) noexcept;
void GXInitLightAttnA(GX::LightObj* light, float a0, float a1, float a2) noexcept;
void GXInitLightAttnK(GX::LightObj* light, float k0, float k1, float k2) noexcept;
void GXInitLightSpot(GX::LightObj* light, float cutoff, GX::SpotFn spotFn) noexcept;
void GXInitLightDistAttn(GX::LightObj* light, float refDistance, float refBrightness, GX::DistAttnFn distFunc) noexcept;
static inline void GXInitLightShininess(GX::LightObj* light, float shininess) noexcept {
  GXInitLightAttn(light, 0.f, 0.f, 1.f, (shininess) / 2.f, 0.f, 1.f - (shininess) / 2.f);
}
void GXInitLightPos(GX::LightObj* light, float x, float y, float z) noexcept;
static inline void GXInitLightPosv(GX::LightObj* light, float vec[3]) noexcept {
  GXInitLightPos(light, vec[0], vec[1], vec[2]);
}
void GXInitLightDir(GX::LightObj* light, float nx, float ny, float nz) noexcept;
static inline void GXInitLightDirv(GX::LightObj* light, float vec[3]) noexcept {
  GXInitLightDir(light, vec[0], vec[1], vec[2]);
}
void GXInitSpecularDir(GX::LightObj* light, float nx, float ny, float nz) noexcept;
void GXInitSpecularDirHA(GX::LightObj* light, float nx, float ny, float nz, float hx, float hy, float hz) noexcept;
static inline void GXInitLightSpecularDirHAv(GX::LightObj* light, float vecn[3], float vech[3]) noexcept {
  GXInitSpecularDirHA(light, vecn[0], vecn[1], vecn[2], vech[0], vech[1], vech[2]);
}
void GXInitLightColor(GX::LightObj* light, GX::Color col) noexcept;
void GXLoadLightObjImm(const GX::LightObj* light, GX::LightID id) noexcept;
void GXLoadLightObjIndx(u32 index, GX::LightID) noexcept;

void GXGetLightAttnA(const GX::LightObj* light, float* a0, float* a1, float* a2) noexcept;
void GXGetLightAttnK(const GX::LightObj* light, float* k0, float* k1, float* k2) noexcept;
void GXGetLightPos(const GX::LightObj* light, float* x, float* y, float* z) noexcept;
static inline void GXGetLightPosv(const GX::LightObj* light, float* vec[3]) noexcept {
  GXGetLightPos(light, vec[0], vec[1], vec[2]);
}
void GXGetLightDir(const GX::LightObj* light, float* nx, float* ny, float* nz) noexcept;
static inline void GXGetLightDirv(const GX::LightObj* light, float* vec[3]) noexcept {
  GXGetLightDir(light, vec[0], vec[1], vec[2]);
}

void GXGetLightColor(const GX::LightObj* light, GX::Color* col) noexcept;
