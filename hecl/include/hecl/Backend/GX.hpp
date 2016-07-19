#ifndef HECLBACKEND_GX_HPP
#define HECLBACKEND_GX_HPP

#include "Backend.hpp"
#include <athena/DNA.hpp>
#include <stdint.h>
#include <stdlib.h>
#include <algorithm>

namespace hecl
{
namespace Backend
{

struct GX : IBackend
{
    enum AttrType
    {
        NONE,
        DIRECT,
        INDEX8,
        INDEX16
    };

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

    enum TevBias
    {
        TB_ZERO          = 0,
        TB_ADDHALF       = 1,
        TB_SUBHALF       = 2,
    };

    enum TevScale
    {
        CS_SCALE_1       = 0,
        CS_SCALE_2       = 1,
        CS_SCALE_4       = 2,
        CS_DIVIDE_2      = 3
    };

    enum TexGenType
    {
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

    enum PTTexMtx
    {
        PTTEXMTX0  = 64,
        PTTEXMTX1  = 67,
        PTTEXMTX2  = 70,
        PTTEXMTX3  = 73,
        PTTEXMTX4  = 76,
        PTTEXMTX5  = 79,
        PTTEXMTX6  = 82,
        PTTEXMTX7  = 85,
        PTTEXMTX8  = 88,
        PTTEXMTX9  = 91,
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

    enum DiffuseFn
    {
        DF_NONE = 0,
        DF_SIGN,
        DF_CLAMP
    };

    enum AttnFn
    {
        AF_SPEC = 0,
        AF_SPOT = 1,
        AF_NONE
    };

    enum Primitive
    {
        POINTS        = 0xb8,
        LINES         = 0xa8,
        LINESTRIP     = 0xb0,
        TRIANGLES     = 0x90,
        TRIANGLESTRIP = 0x98,
        TRIANGLEFAN   = 0xa0,
        QUADS         = 0x80
    };

    struct TexCoordGen
    {
        TexGenSrc m_src = TG_TEX0;
        TexMtx m_mtx = IDENTITY;
        bool m_norm = false;
        PTTexMtx m_pmtx = PTIDENTITY;

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
        TevOp m_cop = TEV_ADD;
        TevOp m_aop = TEV_ADD;
        TevColorArg m_color[4] = {CC_ZERO, CC_ZERO, CC_ZERO, CC_ZERO};
        TevAlphaArg m_alpha[4] = {CA_ZERO, CA_ZERO, CA_ZERO, CA_ZERO};
        TevKColorSel m_kColor = TEV_KCSEL_1;
        TevKAlphaSel m_kAlpha = TEV_KASEL_1;
        TevRegID m_cRegOut = TEVPREV;
        TevRegID m_aRegOut = TEVPREV;
        int m_lazyCInIdx = -1;
        int m_lazyAInIdx = -1;
        int m_lazyCOutIdx = -1;
        int m_lazyAOutIdx = -1;
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

    int m_aRegMask = 0;
    int m_aRegLazy = 0;

    int pickCLazy(Diagnostics& diag, const SourceLocation& loc, int stageIdx) const
    {
        int regMask = m_cRegMask;
        for (int i=stageIdx+1 ; i<int(m_tevCount) ; ++i)
        {
            const TEVStage& stage = m_tevs[i];
            for (int c=0 ; c<4 ; ++c)
            {
                if (stage.m_color[c] == CC_C0)
                    regMask |= 1;
                if (stage.m_color[c] == CC_C1)
                    regMask |= 2;
                if (stage.m_color[c] == CC_C2)
                    regMask |= 4;
            }
        }

        /* Allocate from back for compatibility with Retro's
         * extended shader arithmetic use */
        for (int i=2 ; i>=0 ; --i)
            if (!(regMask & (1 << i)))
                return i;

        diag.reportBackendErr(loc, "TEV C Register overflow");
        return -1;
    }

    int pickALazy(Diagnostics& diag, const SourceLocation& loc, int stageIdx) const
    {
        int regMask = m_aRegMask;
        for (int i=stageIdx+1 ; i<int(m_tevCount) ; ++i)
        {
            const TEVStage& stage = m_tevs[i];
            for (int c=0 ; c<4 ; ++c)
            {
                if (stage.m_color[c] == CC_A0 ||
                    stage.m_alpha[c] == CA_A0)
                    regMask |= 1;
                if (stage.m_color[c] == CC_A1 ||
                    stage.m_alpha[c] == CA_A1)
                    regMask |= 2;
                if (stage.m_color[c] == CC_A2 ||
                    stage.m_alpha[c] == CA_A2)
                    regMask |= 4;
            }
        }

        /* Allocate from back for compatibility with Retro's
         * extended shader arithmetic use */
        for (int i=2 ; i>=0 ; --i)
            if (!(regMask & (1 << i)))
                return i;

        diag.reportBackendErr(loc, "TEV A Register overflow");
        return -1;
    }

    enum BlendFactor : uint16_t
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

    struct Color : athena::io::DNA<athena::BigEndian>
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
            color[3] = uint8_t(std::min(std::max(vec.vec[3] * 255.f, 0.f), 255.f));
            return *this;
        }
        Color& operator=(const atVec3f& vec)
        {
            color[0] = uint8_t(std::min(std::max(vec.vec[0] * 255.f, 0.f), 255.f));
            color[1] = uint8_t(std::min(std::max(vec.vec[1] * 255.f, 0.f), 255.f));
            color[2] = uint8_t(std::min(std::max(vec.vec[2] * 255.f, 0.f), 255.f));
            color[3] = 0xff;
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
        Color(const atVec3f& vec) {*this = vec;}
        Color(uint8_t val) {*this = val;}
        bool operator==(const Color& other) const {return num == other.num;}
        bool operator!=(const Color& other) const {return num != other.num;}
        uint8_t operator[](size_t idx) const {return color[idx];}
        uint8_t& operator[](size_t idx) {return color[idx];}

        void read(athena::io::IStreamReader& reader)
        {reader.readUBytesToBuf(&num, 4);}
        void write(athena::io::IStreamWriter& writer) const
        {writer.writeUBytes(reinterpret_cast<const atUint8*>(&num), 4);}
        size_t binarySize(size_t __isz) const
        {return __isz + 4;}
    };
    unsigned m_kcolorCount = 0;
    Color m_kcolors[4];

    int m_alphaTraceStage = -1;

    bool operator==(const GX& other) const
    {
        if (m_tcgCount != other.m_tcgCount)
            return false;
        if (m_tevCount != other.m_tevCount)
            return false;
        if (m_blendSrc != other.m_blendSrc)
            return false;
        if (m_blendDst != other.m_blendDst)
            return false;
        if (m_kcolorCount != other.m_kcolorCount)
            return false;
        for (unsigned i=0 ; i<m_tcgCount ; ++i)
        {
            const TexCoordGen& a = m_tcgs[i];
            const TexCoordGen& b = other.m_tcgs[i];
            if (a.m_src != b.m_src)
                return false;
            if (a.m_mtx != b.m_mtx)
                return false;
            if (a.m_norm != b.m_norm)
                return false;
            if (a.m_pmtx != b.m_pmtx)
                return false;
        }
        for (unsigned i=0 ; i<m_tevCount ; ++i)
        {
            const TEVStage& a = m_tevs[i];
            const TEVStage& b = other.m_tevs[i];
            for (unsigned j=0 ; j<4 ; ++j)
                if (a.m_color[j] != b.m_color[j])
                    return false;
            for (unsigned j=0 ; j<4 ; ++j)
                if (a.m_alpha[j] != b.m_alpha[j])
                    return false;
            if (a.m_cop != b.m_cop)
                return false;
            if (a.m_aop != b.m_aop)
                return false;
            if (a.m_kColor != b.m_kColor)
                return false;
            if (a.m_kAlpha != b.m_kAlpha)
                return false;
            if (a.m_cRegOut != b.m_cRegOut)
                return false;
            if (a.m_aRegOut != b.m_aRegOut)
                return false;
            if (a.m_texMapIdx != b.m_texMapIdx)
                return false;
            if (a.m_texGenIdx != b.m_texGenIdx)
                return false;
        }
        for (unsigned i=0 ; i<m_kcolorCount ; ++i)
        {
            const Color& a = m_kcolors[i];
            const Color& b = other.m_kcolors[i];
            if (a.num != b.num)
                return false;
        }
        return true;
    }
    bool operator!=(const GX& other) const
    {
        return !(*this == other);
    }

    void reset(const IR& ir, Diagnostics& diag);

private:
    struct TraceResult
    {
        enum class Type
        {
            Invalid,
            TEVStage,
            TEVColorArg,
            TEVAlphaArg,
            TEVKColorSel,
            TEVKAlphaSel
        } type;
        union
        {
            GX::TEVStage* tevStage;
            GX::TevColorArg tevColorArg;
            GX::TevAlphaArg tevAlphaArg;
            GX::TevKColorSel tevKColorSel;
            GX::TevKAlphaSel tevKAlphaSel;
        };
        TraceResult() : type(Type::Invalid) {}
        TraceResult(GX::TEVStage* stage) : type(Type::TEVStage), tevStage(stage) {}
        TraceResult(GX::TevColorArg arg) : type(Type::TEVColorArg), tevColorArg(arg) {}
        TraceResult(GX::TevAlphaArg arg) : type(Type::TEVAlphaArg), tevAlphaArg(arg) {}
        TraceResult(GX::TevKColorSel arg) : type(Type::TEVKColorSel), tevKColorSel(arg) {}
        TraceResult(GX::TevKAlphaSel arg) : type(Type::TEVKAlphaSel), tevKAlphaSel(arg) {}
    };

    unsigned addKColor(Diagnostics& diag, const SourceLocation& loc, const Color& color);
    unsigned addKAlpha(Diagnostics& diag, const SourceLocation& loc, float alpha);
    unsigned addTexCoordGen(Diagnostics& diag, const SourceLocation& loc,
                            TexGenSrc src, TexMtx mtx, bool norm, PTTexMtx pmtx);
    TEVStage& addTEVStage(Diagnostics& diag, const SourceLocation& loc);
    TraceResult RecursiveTraceColor(const IR& ir, Diagnostics& diag,
                                    const IR::Instruction& inst,
                                    bool swizzleAlpha=false);
    TraceResult RecursiveTraceAlpha(const IR& ir, Diagnostics& diag,
                                    const IR::Instruction& inst);
    unsigned RecursiveTraceTexGen(const IR& ir, Diagnostics& diag,
                                  const IR::Instruction& inst,
                                  TexMtx mtx, bool normalize, PTTexMtx pmtx);
};

}
}

#endif // HECLBACKEND_GX_HPP
