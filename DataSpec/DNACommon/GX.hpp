#ifndef _DNACOMMON_GX_HPP_
#define _DNACOMMON_GX_HPP_

#include <Athena/DNA.hpp>

namespace GX
{

/* RGBA8 structure (GXColor) */
struct Color : Athena::io::DNA<Athena::BigEndian>
{
    Value<atUint8> r;
    Value<atUint8> g;
    Value<atUint8> b;
    Value<atUint8> a;
    Delete expl;
    void read(Athena::io::IStreamReader& reader)
    {reader.readUBytesToBuf(&r, 4);}
    void write(Athena::io::IStreamWriter& writer) const
    {writer.writeUBytes(&r, 4);}
};

/* GX enums */

enum AttrType
{
    GX_NONE,
    GX_DIRECT,
    GX_INDEX8,
    GX_INDEX16
};

enum TevColorArg {
    GX_CC_CPREV         = 0,                /*!< Use the color value from previous TEV stage */
    GX_CC_APREV         = 1,                /*!< Use the alpha value from previous TEV stage */
    GX_CC_C0            = 2,                /*!< Use the color value from the color/output register 0 */
    GX_CC_A0            = 3,                /*!< Use the alpha value from the color/output register 0 */
    GX_CC_C1            = 4,                /*!< Use the color value from the color/output register 1 */
    GX_CC_A1            = 5,                /*!< Use the alpha value from the color/output register 1 */
    GX_CC_C2            = 6,                /*!< Use the color value from the color/output register 2 */
    GX_CC_A2            = 7,                /*!< Use the alpha value from the color/output register 2 */
    GX_CC_TEXC          = 8,                /*!< Use the color value from texture */
    GX_CC_TEXA          = 9,                /*!< Use the alpha value from texture */
    GX_CC_RASC          = 10,               /*!< Use the color value from rasterizer */
    GX_CC_RASA          = 11,               /*!< Use the alpha value from rasterizer */
    GX_CC_ONE           = 12,
    GX_CC_HALF          = 13,
    GX_CC_KONST         = 14,
    GX_CC_ZERO          = 15                /*!< Use to pass zero value */
};

enum TevAlphaArg {
    GX_CA_APREV         = 0,                /*!< Use the alpha value from previous TEV stage */
    GX_CA_A0            = 1,                /*!< Use the alpha value from the color/output register 0 */
    GX_CA_A1            = 2,                /*!< Use the alpha value from the color/output register 1 */
    GX_CA_A2            = 3,                /*!< Use the alpha value from the color/output register 2 */
    GX_CA_TEXA          = 4,                /*!< Use the alpha value from texture */
    GX_CA_RASA          = 5,                /*!< Use the alpha value from rasterizer */
    GX_CA_KONST         = 6,
    GX_CA_ZERO          = 7                 /*!< Use to pass zero value */
};

enum TevOp {
    GX_TEV_ADD              = 0,
    GX_TEV_SUB              = 1,
    GX_TEV_COMP_R8_GT       = 8,
    GX_TEV_COMP_R8_EQ       = 9,
    GX_TEV_COMP_GR16_GT     = 10,
    GX_TEV_COMP_GR16_EQ     = 11,
    GX_TEV_COMP_BGR24_GT    = 12,
    GX_TEV_COMP_BGR24_EQ    = 13,
    GX_TEV_COMP_RGB8_GT     = 14,
    GX_TEV_COMP_RGB8_EQ     = 15,
    GX_TEV_COMP_A8_GT       = GX_TEV_COMP_RGB8_GT,     // for alpha channel
    GX_TEV_COMP_A8_EQ       = GX_TEV_COMP_RGB8_EQ  // for alpha channel
};

enum TevBias {
    GX_TB_ZERO          = 0,
    GX_TB_ADDHALF       = 1,
    GX_TB_SUBHALF       = 2,
};

enum TevScale {
    GX_CS_SCALE_1       = 0,
    GX_CS_SCALE_2       = 1,
    GX_CS_SCALE_4       = 2,
    GX_CS_DIVIDE_2      = 3
};

enum TevRegID {
    GX_TEVPREV       = 0,
    GX_TEVREG0       = 1,
    GX_TEVREG1       = 2,
    GX_TEVREG2       = 3
};

enum TexGenType
{
    GX_TG_MTX3x4 = 0,
    GX_TG_MTX2x4,
    GX_TG_BUMP0,
    GX_TG_BUMP1,
    GX_TG_BUMP2,
    GX_TG_BUMP3,
    GX_TG_BUMP4,
    GX_TG_BUMP5,
    GX_TG_BUMP6,
    GX_TG_BUMP7,
    GX_TG_SRTG
};

enum TexGenSrc
{
    GX_TG_POS = 0,
    GX_TG_NRM,
    GX_TG_BINRM,
    GX_TG_TANGENT,
    GX_TG_TEX0,
    GX_TG_TEX1,
    GX_TG_TEX2,
    GX_TG_TEX3,
    GX_TG_TEX4,
    GX_TG_TEX5,
    GX_TG_TEX6,
    GX_TG_TEX7,
    GX_TG_TEXCOORD0,
    GX_TG_TEXCOORD1,
    GX_TG_TEXCOORD2,
    GX_TG_TEXCOORD3,
    GX_TG_TEXCOORD4,
    GX_TG_TEXCOORD5,
    GX_TG_TEXCOORD6,
    GX_TG_COLOR0,
    GX_TG_COLOR1
};

enum TexMtx
{
    GX_TEXMTX0  = 30,
    GX_TEXMTX1  = 33,
    GX_TEXMTX2  = 36,
    GX_TEXMTX3  = 39,
    GX_TEXMTX4  = 42,
    GX_TEXMTX5  = 45,
    GX_TEXMTX6  = 48,
    GX_TEXMTX7  = 51,
    GX_TEXMTX8  = 54,
    GX_TEXMTX9  = 57,
    GX_IDENTITY = 60
};

enum PTTexMtx
{
    GX_PTTEXMTX0  = 64,
    GX_PTTEXMTX1  = 67,
    GX_PTTEXMTX2  = 70,
    GX_PTTEXMTX3  = 73,
    GX_PTTEXMTX4  = 76,
    GX_PTTEXMTX5  = 79,
    GX_PTTEXMTX6  = 82,
    GX_PTTEXMTX7  = 85,
    GX_PTTEXMTX8  = 88,
    GX_PTTEXMTX9  = 91,
    GX_PTTEXMTX10 = 94,
    GX_PTTEXMTX11 = 97,
    GX_PTTEXMTX12 = 100,
    GX_PTTEXMTX13 = 103,
    GX_PTTEXMTX14 = 106,
    GX_PTTEXMTX15 = 109,
    GX_PTTEXMTX16 = 112,
    GX_PTTEXMTX17 = 115,
    GX_PTTEXMTX18 = 118,
    GX_PTTEXMTX19 = 121,
    GX_PTIDENTITY = 125
};

}

#endif
