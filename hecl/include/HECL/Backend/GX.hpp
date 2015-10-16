#ifndef HECLBACKEND_GX_HPP
#define HECLBACKEND_GX_HPP

#include "Backend.hpp"
#include <Athena/Types.hpp>
#include <stdint.h>
#include <stdlib.h>
#include <algorithm>

namespace HECL
{
namespace Backend
{

struct GX : IBackend
{
    enum TevOp
    {
        TEV_ADD              = 0,
        TEV_SUB              = 1,
        TEV_COMP_R8_GT       = 8,
        TEV_COMP_R8_EQ       = 9,
        TEV_COMP_GR16_GT     = 10,
        TEV_COMP_GR16_EQ     = 11,
        TEV_COMP_BGR24_GT    = 12,
        TEV_COMP_BGR24_EQ    = 13,
        TEV_COMP_RGB8_GT     = 14,
        TEV_COMP_RGB8_EQ     = 15,
        TEV_COMP_A8_GT       = TEV_COMP_RGB8_GT,     // for alpha channel
        TEV_COMP_A8_EQ       = TEV_COMP_RGB8_EQ  // for alpha channel
    };

    enum TevRegID
    {
        TEVPREV       = 0,
        TEVREG0       = 1,
        TEVREG1       = 2,
        TEVREG2       = 3,
        TEVLAZY       = 5
    };

    enum TevColorArg
    {
        CC_CPREV         = 0,                /*!< Use the color value from previous TEV stage */
        CC_APREV         = 1,                /*!< Use the alpha value from previous TEV stage */
        CC_C0            = 2,                /*!< Use the color value from the color/output register 0 */
        CC_A0            = 3,                /*!< Use the alpha value from the color/output register 0 */
        CC_C1            = 4,                /*!< Use the color value from the color/output register 1 */
        CC_A1            = 5,                /*!< Use the alpha value from the color/output register 1 */
        CC_C2            = 6,                /*!< Use the color value from the color/output register 2 */
        CC_A2            = 7,                /*!< Use the alpha value from the color/output register 2 */
        CC_TEXC          = 8,                /*!< Use the color value from texture */
        CC_TEXA          = 9,                /*!< Use the alpha value from texture */
        CC_RASC          = 10,               /*!< Use the color value from rasterizer */
        CC_RASA          = 11,               /*!< Use the alpha value from rasterizer */
        CC_ONE           = 12,
        CC_HALF          = 13,
        CC_KONST         = 14,
        CC_ZERO          = 15,               /*!< Use to pass zero value */

        /* Non-GX */
        CC_LAZY                              /*!< Lazy register allocation */
    };

    enum TevAlphaArg
    {
        CA_APREV         = 0,                /*!< Use the alpha value from previous TEV stage */
        CA_A0            = 1,                /*!< Use the alpha value from the color/output register 0 */
        CA_A1            = 2,                /*!< Use the alpha value from the color/output register 1 */
        CA_A2            = 3,                /*!< Use the alpha value from the color/output register 2 */
        CA_TEXA          = 4,                /*!< Use the alpha value from texture */
        CA_RASA          = 5,                /*!< Use the alpha value from rasterizer */
        CA_KONST         = 6,
        CA_ZERO          = 7,                /*!< Use to pass zero value */

        /* Non-GX */
        CA_LAZY                              /*!< Lazy register allocation */
    };

    enum TevKColorSel
    {
        TEV_KCSEL_8_8  = 0x00,
        TEV_KCSEL_7_8  = 0x01,
        TEV_KCSEL_6_8  = 0x02,
        TEV_KCSEL_5_8  = 0x03,
        TEV_KCSEL_4_8  = 0x04,
        TEV_KCSEL_3_8  = 0x05,
        TEV_KCSEL_2_8  = 0x06,
        TEV_KCSEL_1_8  = 0x07,

        TEV_KCSEL_1    = TEV_KCSEL_8_8,
        TEV_KCSEL_3_4  = TEV_KCSEL_6_8,
        TEV_KCSEL_1_2  = TEV_KCSEL_4_8,
        TEV_KCSEL_1_4  = TEV_KCSEL_2_8,

        TEV_KCSEL_K0   = 0x0C,
        TEV_KCSEL_K1   = 0x0D,
        TEV_KCSEL_K2   = 0x0E,
        TEV_KCSEL_K3   = 0x0F,
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
        TEV_KCSEL_K3_A = 0x1F
    };

    enum TevKAlphaSel
    {
        TEV_KASEL_8_8  = 0x00,
        TEV_KASEL_7_8  = 0x01,
        TEV_KASEL_6_8  = 0x02,
        TEV_KASEL_5_8  = 0x03,
        TEV_KASEL_4_8  = 0x04,
        TEV_KASEL_3_8  = 0x05,
        TEV_KASEL_2_8  = 0x06,
        TEV_KASEL_1_8  = 0x07,

        TEV_KASEL_1    = TEV_KASEL_8_8,
        TEV_KASEL_3_4  = TEV_KASEL_6_8,
        TEV_KASEL_1_2  = TEV_KASEL_4_8,
        TEV_KASEL_1_4  = TEV_KASEL_2_8,

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
        TEV_KASEL_K3_A = 0x1F
    };

    enum TexGenSrc
    {
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
        TG_COLOR1
    };

    enum TexMtx
    {
        TEXMTX0  = 30,
        TEXMTX1  = 33,
        TEXMTX2  = 36,
        TEXMTX3  = 39,
        TEXMTX4  = 42,
        TEXMTX5  = 45,
        TEXMTX6  = 48,
        TEXMTX7  = 51,
        TEXMTX8  = 54,
        TEXMTX9  = 57,
        IDENTITY = 60
    };

    struct TexCoordGen
    {
        TexGenSrc m_src = TG_TEX0;
        TexMtx m_mtx = IDENTITY;

        /* Not actually part of GX, but a way to relate out-of-band
         * texmtx animation parameters */
        std::string m_gameFunction;
        std::vector<atVec4f> m_gameArgs;
    };
    unsigned m_tcgCount = 0;
    TexCoordGen m_tcgs[8];

    unsigned m_texMtxCount = 0;
    TexCoordGen* m_texMtxRefs[8];

    struct TEVStage
    {
        TevOp m_op = TEV_ADD;
        TevColorArg m_color[4] = {CC_ZERO, CC_ZERO, CC_ZERO, CC_ZERO};
        TevAlphaArg m_alpha[4] = {CA_ZERO, CA_ZERO, CA_ZERO, CA_ZERO};
        TevKColorSel m_kColor = TEV_KCSEL_1;
        TevKAlphaSel m_kAlpha = TEV_KASEL_1;
        TevRegID m_regOut = TEVPREV;
        int m_lazyCInIdx = -1;
        int m_lazyAInIdx = -1;
        int m_lazyOutIdx = -1;
        int m_texMapIdx = -1;
        int m_texGenIdx = -1;

        /* Convenience Links */
        TEVStage* m_prev = nullptr;
        TEVStage* m_next = nullptr;

        /* Remember this for debugging */
        SourceLocation m_loc;
    };
    unsigned m_tevCount = 0;
    TEVStage m_tevs[16];

    int getStageIdx(const TEVStage* stage) const
    {
        for (int i=0 ; i<int(m_tevCount) ; ++i)
            if (&m_tevs[i] == stage)
                return i;
        return -1;
    }

    int m_cRegMask = 0;
    int m_cRegLazy = 0;

    int pickCLazy(Diagnostics& diag, const SourceLocation& loc, int stageIdx) const
    {
        int regMask = m_cRegMask;
        for (int i=stageIdx+1 ; i<int(m_tevCount) ; ++i)
        {
            const TEVStage& stage = m_tevs[i];
            for (int c=0 ; c<4 ; ++c)
            {
                if (stage.m_color[c] == CC_C0 ||
                    stage.m_color[c] == CC_A0 ||
                    stage.m_alpha[c] == CA_A0)
                    regMask |= 1;
                if (stage.m_color[c] == CC_C1 ||
                    stage.m_color[c] == CC_A1 ||
                    stage.m_alpha[c] == CA_A1)
                    regMask |= 2;
                if (stage.m_color[c] == CC_C2 ||
                    stage.m_color[c] == CC_A2 ||
                    stage.m_alpha[c] == CA_A2)
                    regMask |= 4;
            }
        }

        for (int i=0 ; i<3 ; ++i)
            if (!(regMask & (1 << i)))
                return i;

        diag.reportBackendErr(loc, "TEV C Register overflow");
        return -1;
    }

    enum BlendFactor
    {
        BL_ZERO,
        BL_ONE,
        BL_SRCCLR,
        BL_INVSRCCLR,
        BL_SRCALPHA,
        BL_INVSRCALPHA,
        BL_DSTALPHA,
        BL_INVDSTALPHA
    };
    BlendFactor m_blendSrc;
    BlendFactor m_blendDst;

    struct Color
    {
        union
        {
            uint8_t color[4];
            uint32_t num = 0;
        };
        Color() = default;
        Color& operator=(const atVec4f& vec)
        {
            color[0] = uint8_t(std::min(std::max(vec.vec[0] * 255.f, 0.f), 255.f));
            color[1] = uint8_t(std::min(std::max(vec.vec[1] * 255.f, 0.f), 255.f));
            color[2] = uint8_t(std::min(std::max(vec.vec[2] * 255.f, 0.f), 255.f));
            color[3] = 0;
            return *this;
        }
        Color& operator=(uint8_t val)
        {
            color[0] = val;
            color[1] = val;
            color[2] = val;
            color[3] = val;
            return *this;
        }
        Color(const atVec4f& vec) {*this = vec;}
        Color(uint8_t val) {*this = val;}
        bool operator==(const Color& other) const {return num == other.num;}
        bool operator!=(const Color& other) const {return num != other.num;}
        uint8_t operator[](size_t idx) const {return color[idx];}
        uint8_t& operator[](size_t idx) {return color[idx];}
    };
    unsigned m_kcolorCount = 0;
    Color m_kcolors[4];

    int m_alphaTraceStage = -1;

    void reset(const IR& ir, Diagnostics& diag);

private:
    struct TraceResult
    {
        enum
        {
            TraceInvalid,
            TraceTEVStage,
            TraceTEVColorArg,
            TraceTEVAlphaArg,
            TraceTEVKColorSel,
            TraceTEVKAlphaSel
        } type;
        union
        {
            GX::TEVStage* tevStage;
            GX::TevColorArg tevColorArg;
            GX::TevAlphaArg tevAlphaArg;
            GX::TevKColorSel tevKColorSel;
            GX::TevKAlphaSel tevKAlphaSel;
        };
        TraceResult() : type(TraceInvalid) {}
        TraceResult(GX::TEVStage* stage) : type(TraceTEVStage), tevStage(stage) {}
        TraceResult(GX::TevColorArg arg) : type(TraceTEVColorArg), tevColorArg(arg) {}
        TraceResult(GX::TevAlphaArg arg) : type(TraceTEVAlphaArg), tevAlphaArg(arg) {}
        TraceResult(GX::TevKColorSel arg) : type(TraceTEVKColorSel), tevKColorSel(arg) {}
        TraceResult(GX::TevKAlphaSel arg) : type(TraceTEVKAlphaSel), tevKAlphaSel(arg) {}
    };

    unsigned addKColor(Diagnostics& diag, const SourceLocation& loc, const Color& color);
    unsigned addKAlpha(Diagnostics& diag, const SourceLocation& loc, float alpha);
    unsigned addTexCoordGen(Diagnostics& diag, const SourceLocation& loc,
                            TexGenSrc src, TexMtx mtx);
    TEVStage& addTEVStage(Diagnostics& diag, const SourceLocation& loc);
    void PreTraceColor(const IR& ir, Diagnostics& diag,
                       const IR::Instruction& inst);
    void PreTraceAlpha(const IR& ir, Diagnostics& diag,
                       const IR::Instruction& inst);
    TraceResult RecursiveTraceColor(const IR& ir, Diagnostics& diag,
                                    const IR::Instruction& inst,
                                    bool swizzleAlpha=false);
    TraceResult RecursiveTraceAlpha(const IR& ir, Diagnostics& diag,
                                    const IR::Instruction& inst);
    unsigned RecursiveTraceTexGen(const IR& ir, Diagnostics& diag,
                                  const IR::Instruction& inst,
                                  TexMtx mtx);
};

}
}

#endif // HECLBACKEND_GX_HPP
