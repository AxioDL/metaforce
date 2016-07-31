#ifndef HECLBACKEND_GLSL_HPP
#define HECLBACKEND_GLSL_HPP

#include "ProgrammableCommon.hpp"

namespace hecl
{
namespace Backend
{

#define HECL_GLSL_VERT_UNIFORM_BLOCK_NAME "HECLVertUniform"
#define HECL_GLSL_TEXMTX_UNIFORM_BLOCK_NAME "HECLTexMtxUniform"

struct GLSL : ProgrammableCommon
{
    void reset(const IR& ir, Diagnostics& diag);
    std::string makeVert(const char* glslVer, unsigned col, unsigned uv, unsigned w,
                         unsigned skinSlots, unsigned texMtxs, size_t extTexCount,
                         const TextureInfo* extTexs) const;
    std::string makeFrag(const char* glslVer,
                         const ShaderFunction& lighting=ShaderFunction()) const;
    std::string makeFrag(const char* glslVer,
                         const ShaderFunction& lighting,
                         const ShaderFunction& post,
                         size_t extTexCount) const;

private:
    std::string GenerateVertInStruct(unsigned col, unsigned uv, unsigned w) const;
    std::string GenerateVertToFragStruct(size_t extTexCount) const;
    std::string GenerateVertUniformStruct(unsigned skinSlots, unsigned texMtxs) const;

    std::string EmitVec3(const atVec4f& vec) const
    {
        return hecl::Format("vec3(%g,%g,%g)", vec.vec[0], vec.vec[1], vec.vec[2]);
    }

    std::string EmitTexGenSource2(TexGenSrc src, int uvIdx) const;
    std::string EmitTexGenSource4(TexGenSrc src, int uvIdx) const;
};

}
}

#endif // HECLBACKEND_GLSL_HPP
