#ifndef __COMMON_DPSC_HPP__
#define __COMMON_DPSC_HPP__

#include "ParticleCommon.hpp"
#include "PAK.hpp"
#include "athena/FileWriter.hpp"

namespace DataSpec::DNAParticle
{

template <class IDType>
struct DPSM : BigDNA
{
    AT_DECL_EXPLICIT_DNA_YAML
    AT_SUBDECL_DNA

    struct SQuadDescr
    {
        IntElementFactory        x0_LFT;
        RealElementFactory       x4_SZE;
        RealElementFactory       x8_ROT;
        VectorElementFactory     xc_OFF;
        ColorElementFactory      x10_CLR;
        UVElementFactory<IDType> x14_TEX;
        BoolHelper               x18_ADD;
    };

    SQuadDescr x0_quad;
    SQuadDescr x1c_quad;
    ChildResourceFactory<IDType> x38_DMDL;
    IntElementFactory    x48_DLFT;
    VectorElementFactory x4c_DMOP;
    VectorElementFactory x50_DMRT;
    VectorElementFactory x54_DMSC;
    ColorElementFactory  x58_DMCL;
    union
    {
        struct { bool x5c_24_DMAB : 1; bool x5c_25_DMOO : 1;};
        uint8_t dummy;
    };
    template <class Reader>
    void readQuadDecalInfo(Reader& r, uint32_t clsId, SQuadDescr& quad);
    void writeQuadDecalInfo(athena::io::YAMLDocWriter& w, const SQuadDescr& quad, bool first) const;
    void getQuadDecalBinarySize(size_t& s, const SQuadDescr& desc) const;
    void writeQuadDecalInfo(athena::io::IStreamWriter& w, const SQuadDescr& quad, bool first) const;

    void gatherDependencies(std::vector<hecl::ProjectPath>&) const;
};

template <class IDType>
bool ExtractDPSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteDPSM(const DPSM<IDType>& dpsm, const hecl::ProjectPath& outPath);

}

#endif // __COMMON_DPSC_HPP__
