#ifndef HECLBACKEND_HLSL_HPP
#define HECLBACKEND_HLSL_HPP

#include "ProgrammableCommon.hpp"

namespace hecl
{
namespace Backend
{

struct HLSL : ProgrammableCommon
{
    void reset(const IR& ir, Diagnostics& diag);
    std::string makeVert(unsigned col, unsigned uv, unsigned w,
                         unsigned skinSlots, unsigned texMtxs, size_t extTexCount,
                         const TextureInfo* extTexs, ReflectionType reflectionType) const;
    std::string makeFrag(bool alphaTest, ReflectionType reflectionType,
                         const ShaderFunction& lighting=ShaderFunction()) const;
    std::string makeFrag(bool alphaTest, const ShaderFunction& lighting,
                         ReflectionType reflectionType,
                         const ShaderFunction& post, size_t extTexCount,
                         const TextureInfo* extTexs) const;

private:
    std::string GenerateVertInStruct(unsigned col, unsigned uv, unsigned w) const;
    std::string GenerateVertToFragStruct(size_t extTexCount, ReflectionType reflectionType) const;
    std::string GenerateVertUniformStruct(unsigned skinSlots, unsigned texMtxs, ReflectionType reflectionType) const;
    std::string GenerateAlphaTest() const;
    std::string GenerateReflectionExpr(ReflectionType type) const;

    std::string EmitVec3(const atVec4f& vec) const
    {
        return hecl::Format("float3(%g,%g,%g)", vec.vec[0], vec.vec[1], vec.vec[2]);
    }

    std::string EmitTexGenSource2(TexGenSrc src, int uvIdx) const;
    std::string EmitTexGenSource4(TexGenSrc src, int uvIdx) const;
};

}
}

#endif // HECLBACKEND_HLSL_HPP
