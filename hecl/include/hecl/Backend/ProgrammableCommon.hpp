#ifndef HECLBACKEND_PROGCOMMON_HPP
#define HECLBACKEND_PROGCOMMON_HPP

#include "Backend.hpp"
#include "hecl/Runtime.hpp"
#include <athena/DNA.hpp>
#include <stdint.h>
#include <stdlib.h>
#include <algorithm>

namespace hecl
{
namespace Backend
{

struct ProgrammableCommon : IBackend
{
    using ShaderFunction = Runtime::ShaderCacheExtensions::Function;

    std::string m_colorExpr;
    std::string m_alphaExpr;
    BlendFactor m_blendSrc;
    BlendFactor m_blendDst;
    bool m_lighting = false;

    struct TexSampling
    {
        int mapIdx = -1;
        int tcgIdx = -1;
    };
    std::vector<TexSampling> m_texSamplings;
    unsigned m_texMapEnd = 0;
    unsigned m_extMapStart = 8;

    struct TexCoordGen
    {
        TexGenSrc m_src;
        int m_uvIdx = 0;
        int m_mtx = -1;
        bool m_norm = false;
        std::string m_gameFunction;
        std::vector<atVec4f> m_gameArgs;
    };
    std::vector<TexCoordGen> m_tcgs;
    std::vector<size_t> m_texMtxRefs;

    void reset(const IR& ir, Diagnostics& diag, const char* backendName);

private:
    unsigned addTexCoordGen(TexGenSrc src, int uvIdx, int mtx, bool normalize);
    unsigned addTexSampling(unsigned mapIdx, unsigned tcgIdx);
    std::string RecursiveTraceColor(const IR& ir, Diagnostics& diag,
                                    const IR::Instruction& inst, bool toSwizzle);
    std::string RecursiveTraceAlpha(const IR& ir, Diagnostics& diag,
                                    const IR::Instruction& inst, bool toSwizzle);
    unsigned RecursiveTraceTexGen(const IR& ir, Diagnostics& diag,
                                  const IR::Instruction& inst,
                                  int mtx, bool normalize);

    std::string EmitSamplingUseRaw(unsigned samplingIdx) const
    {
        return hecl::Format("sampling%u", samplingIdx);
    }

    std::string EmitSamplingUseRGB(unsigned samplingIdx) const
    {
        return hecl::Format("sampling%u.rgb", samplingIdx);
    }

    std::string EmitSamplingUseAlpha(unsigned samplingIdx) const
    {
        return hecl::Format("sampling%u.a", samplingIdx);
    }

    std::string EmitColorRegUseRaw(unsigned idx) const
    {
        return hecl::Format("colorReg%u", idx);
    }

    std::string EmitColorRegUseRGB(unsigned idx) const
    {
        return hecl::Format("colorReg%u.rgb", idx);
    }

    std::string EmitColorRegUseAlpha(unsigned idx) const
    {
        return hecl::Format("colorReg%u.a", idx);
    }

    std::string EmitLightingRaw() const
    {
        return std::string("lighting");
    }

    std::string EmitLightingRGB() const
    {
        return std::string("lighting.rgb");
    }

    std::string EmitLightingAlpha() const
    {
        return std::string("lighting.a");
    }

    virtual std::string EmitVec3(const atVec4f& vec) const=0;

    std::string EmitVal(float val) const
    {
        return hecl::Format("%g", val);
    }

    std::string EmitAdd(const std::string& a, const std::string& b) const
    {
        return '(' + a + '+' + b + ')';
    }

    std::string EmitSub(const std::string& a, const std::string& b) const
    {
        return '(' + a + '-' + b + ')';
    }

    std::string EmitMult(const std::string& a, const std::string& b) const
    {
        return '(' + a + '*' + b + ')';
    }

    std::string EmitDiv(const std::string& a, const std::string& b) const
    {
        return '(' + a + '/' + b + ')';
    }

    std::string EmitSwizzle3(Diagnostics& diag, const SourceLocation& loc,
                             const std::string& a, const atInt8 swiz[4]) const;

    std::string EmitSwizzle1(Diagnostics& diag, const SourceLocation& loc,
                             const std::string& a, const atInt8 swiz[4]) const;
};

}
}

#endif // HECLBACKEND_PROGCOMMON_HPP
