#ifndef HECLBACKEND_PROGCOMMON_HPP
#define HECLBACKEND_PROGCOMMON_HPP

#include "Backend.hpp"
#include <Athena/DNA.hpp>
#include <boo/graphicsdev/IGraphicsDataFactory.hpp>
#include <stdint.h>
#include <stdlib.h>
#include <algorithm>

namespace HECL
{
namespace Backend
{

struct ProgrammableCommon : IBackend
{
    std::string m_colorExpr;
    std::string m_alphaExpr;
    boo::BlendFactor m_blendSrc;
    boo::BlendFactor m_blendDst;
    bool m_lighting = false;

    struct TexSampling
    {
        int mapIdx = -1;
        int tcgIdx = -1;
    };
    std::vector<TexSampling> m_texSamplings;

    enum TexGenSrc
    {
        TG_POS,
        TG_NRM,
        TG_UV
    };

    struct TexCoordGen
    {
        TexGenSrc m_src;
        int m_uvIdx = 0;
        int m_mtx = -1;
        std::string m_gameFunction;
        std::vector<atVec4f> m_gameArgs;
    };
    std::vector<TexCoordGen> m_tcgs;
    std::vector<size_t> m_texMtxRefs;

    void reset(const IR& ir, Diagnostics& diag, const char* backendName);

private:
    unsigned addTexCoordGen(TexGenSrc src, int uvIdx, int mtx);
    unsigned addTexSampling(unsigned mapIdx, unsigned tcgIdx);
    std::string RecursiveTraceColor(const IR& ir, Diagnostics& diag,
                                    const IR::Instruction& inst);
    std::string RecursiveTraceAlpha(const IR& ir, Diagnostics& diag,
                                    const IR::Instruction& inst);
    unsigned RecursiveTraceTexGen(const IR& ir, Diagnostics& diag,
                                  const IR::Instruction& inst,
                                  int mtx);

    std::string EmitSamplingUse(unsigned samplingIdx) const
    {
        return HECL::Format("sampling%u", samplingIdx);
    }

    std::string EmitColorRegUse(unsigned idx) const
    {
        return HECL::Format("colorReg%u", idx);
    }

    std::string EmitLighting() const
    {
        return std::string("lighting");
    }

    virtual std::string EmitVec3(const atVec4f& vec) const=0;

    std::string EmitVal(float val) const
    {
        return HECL::Format("%g", val);
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
