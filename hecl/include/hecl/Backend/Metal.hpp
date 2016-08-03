#ifndef HECLBACKEND_METAL_HPP
#define HECLBACKEND_METAL_HPP

#if BOO_HAS_METAL

#include "ProgrammableCommon.hpp"

namespace hecl
{
namespace Backend
{

struct Metal : ProgrammableCommon
{
    void reset(const IR& ir, Diagnostics& diag);
    std::string makeVert(unsigned col, unsigned uv, unsigned w,
                         unsigned skinSlots, unsigned texMtxs, size_t extTexCount,
                         const TextureInfo* extTexs) const;
    std::string makeFrag(size_t blockCount, const char** blockNames, bool alphaTest,
                         const ShaderFunction& lighting=ShaderFunction()) const;
    std::string makeFrag(size_t blockCount, const char** blockNames, bool alphaTest,
                         const ShaderFunction& lighting,
                         const ShaderFunction& post, size_t extTexCount,
                         const TextureInfo* extTexs) const;

private:
    std::string GenerateVertInStruct(unsigned col, unsigned uv, unsigned w) const;
    std::string GenerateVertToFragStruct(size_t extTexCount) const;
    std::string GenerateVertUniformStruct(unsigned skinSlots) const;
    std::string GenerateFragOutStruct() const;
    std::string GenerateAlphaTest() const;

    std::string EmitVec3(const atVec4f& vec) const
    {
        return hecl::Format("float3(%g,%g,%g)", vec.vec[0], vec.vec[1], vec.vec[2]);
    }

    std::string EmitTexGenSource2(TexGenSrc src, int uvIdx) const;
    std::string EmitTexGenSource4(TexGenSrc src, int uvIdx) const;
};

}
}

#endif // BOO_HAS_METAL
#endif // HECLBACKEND_METAL_HPP
