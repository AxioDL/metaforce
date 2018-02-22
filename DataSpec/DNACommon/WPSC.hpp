#ifndef __COMMON_WPSC_HPP__
#define __COMMON_WPSC_HPP__

#include "ParticleCommon.hpp"
#include "PAK.hpp"
#include "athena/FileWriter.hpp"

namespace DataSpec::DNAParticle
{
template <class IDType>
struct WPSM : BigDNA
{
    AT_DECL_EXPLICIT_DNA_YAML
    AT_SUBDECL_DNA
    VectorElementFactory x0_IORN;
    VectorElementFactory x4_IVEC;
    VectorElementFactory x8_PSOV;
    ModVectorElementFactory xc_PSVM;
    BoolHelper x10_VMD2;
    IntElementFactory x14_PSLT;
    VectorElementFactory x18_PSCL;
    ColorElementFactory x1c_PCOL;
    VectorElementFactory x20_POFS;
    VectorElementFactory x24_OFST;
    BoolHelper x28_APSO;
    BoolHelper x29_HOMG;
    BoolHelper x2a_AP11;
    BoolHelper x2b_AP21;
    BoolHelper x2c_AS11;
    BoolHelper x2d_AS12;
    BoolHelper x2e_AS13;
    RealElementFactory x30_TRAT;
    ChildResourceFactory<IDType> x34_APSM;
    ChildResourceFactory<IDType> x44_APS2;
    ChildResourceFactory<IDType> x54_ASW1;
    ChildResourceFactory<IDType> x64_ASW2;
    ChildResourceFactory<IDType> x74_ASW3;
    ChildResourceFactory<IDType> x84_OHEF;
    ChildResourceFactory<IDType> x94_COLR;
    BoolHelper xa4_EWTR;
    BoolHelper xa5_LWTR;
    BoolHelper xa6_SWTR;
    uint32_t xa8_PJFX = ~0;
    RealElementFactory xac_RNGE;
    RealElementFactory xb0_FOFF;
    BoolHelper xunk_FC60;
    BoolHelper xunk_SPS1;
    BoolHelper xunk_SPS2;

    WPSM()
    {
        xa4_EWTR = true; xa5_LWTR = true; xa6_SWTR = true;
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>&) const;
};

template <class IDType>
bool ExtractWPSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteWPSM(const WPSM<IDType>& wpsm, const hecl::ProjectPath& outPath);

}

#endif // __COMMON_WPSC_HPP__
