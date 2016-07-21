#ifndef HECLBACKEND_METAL_HPP
#define HECLBACKEND_METAL_HPP

#if __APPLE__
#include <Availability.h>
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 101100
#define BOO_HAS_METAL 1
#else
#define BOO_HAS_METAL 0
#endif
#endif

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
                         unsigned skinSlots, unsigned texMtxs) const;
    std::string makeFrag(size_t blockCount, const char** blockNames,
                         const ShaderFunction& lighting=ShaderFunction()) const;
    std::string makeFrag(size_t blockCount, const char** blockNames,
                         const ShaderFunction& lighting,
                         const ShaderFunction& post) const;

private:
    std::string GenerateVertInStruct(unsigned col, unsigned uv, unsigned w) const;
    std::string GenerateVertToFragStruct() const;
    std::string GenerateVertUniformStruct(unsigned skinSlots) const;

    std::string EmitVec3(const atVec4f& vec) const
    {
        return hecl::Format("float3(%g,%g,%g)", vec.vec[0], vec.vec[1], vec.vec[2]);
    }

    std::string EmitColorRegUse(unsigned idx) const
    {
        return hecl::Format("lu.colorReg%u", idx);
    }

    std::string EmitTexGenSource2(TexGenSrc src, int uvIdx) const;
    std::string EmitTexGenSource4(TexGenSrc src, int uvIdx) const;
};

}
}

#endif // BOO_HAS_METAL
#endif // HECLBACKEND_METAL_HPP
