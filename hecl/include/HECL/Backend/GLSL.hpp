#ifndef HECLBACKEND_GLSL_HPP
#define HECLBACKEND_GLSL_HPP

#include "Backend.hpp"
#include <Athena/DNA.hpp>
#include <boo/graphicsdev/GL.hpp>
#include <stdint.h>
#include <stdlib.h>
#include <algorithm>

namespace HECL
{
namespace Backend
{

struct GLSL : IBackend
{
    boo::BlendFactor m_blendSrc;
    boo::BlendFactor m_blendDst;
    std::string m_vertSource;
    std::string m_fragSource;

    size_t m_texSamplingCount = 0;
    std::string m_texSamplings;

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

    void reset(const IR& ir, Diagnostics& diag);

private:
    unsigned addTexCoordGen(Diagnostics& diag, const SourceLocation& loc,
                            TexGenSrc src, int uvIdx, int mtx);
    std::string RecursiveTraceColor(const IR& ir, Diagnostics& diag,
                                    const IR::Instruction& inst,
                                    bool swizzleAlpha=false);
    std::string RecursiveTraceAlpha(const IR& ir, Diagnostics& diag,
                                    const IR::Instruction& inst);
    unsigned RecursiveTraceTexGen(const IR& ir, Diagnostics& diag,
                                  const IR::Instruction& inst,
                                  int mtx);
};

}
}

#endif // HECLBACKEND_GLSL_HPP
