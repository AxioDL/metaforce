#ifndef HECLBACKEND_GLSL_HPP
#define HECLBACKEND_GLSL_HPP

#include "ProgrammableCommon.hpp"

namespace hecl::Backend
{

#define HECL_GLSL_VERT_UNIFORM_BLOCK_NAME "HECLVertUniform"
#define HECL_GLSL_TEXMTX_UNIFORM_BLOCK_NAME "HECLTexMtxUniform"

struct GLSL : ProgrammableCommon
{
    void reset(const IR& ir, Diagnostics& diag);
    std::string makeVert(unsigned col, unsigned uv, unsigned w,
                         unsigned skinSlots, size_t extTexCount,
                         const TextureInfo* extTexs, ReflectionType reflectionType) const;
    std::string makeFrag(bool alphaTest, ReflectionType reflectionType,
                         const Function& lighting=Function()) const;
    std::string makeFrag(bool alphaTest,
                         ReflectionType reflectionType,
                         const Function& lighting,
                         const Function& post,
                         size_t extTexCount, const TextureInfo* extTexs) const;

private:
    std::string GenerateVertInStruct(unsigned col, unsigned uv, unsigned w) const;
    std::string GenerateVertToFragStruct(size_t extTexCount, bool reflectionCoords) const;
    std::string GenerateVertUniformStruct(unsigned skinSlots, bool reflectionCoords) const;
    std::string GenerateAlphaTest() const;
    std::string GenerateReflectionExpr(ReflectionType type) const;

    std::string EmitVec3(const atVec4f& vec) const
    {
        return hecl::Format("vec3(%g,%g,%g)", vec.vec[0], vec.vec[1], vec.vec[2]);
    }

    std::string EmitVec3(const std::string& a, const std::string& b, const std::string& c) const
    {
        return hecl::Format("vec3(%s,%s,%s)", a.c_str(), b.c_str(), c.c_str());
    }

    std::string EmitTexGenSource2(TexGenSrc src, int uvIdx) const;
    std::string EmitTexGenSource4(TexGenSrc src, int uvIdx) const;
};

}

#endif // HECLBACKEND_GLSL_HPP
