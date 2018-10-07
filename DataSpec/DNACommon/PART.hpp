#pragma once

#include "ParticleCommon.hpp"
#include "PAK.hpp"
#include "athena/FileWriter.hpp"

namespace DataSpec::DNAParticle
{

template <class IDType>
struct GPSM : BigDNA
{
    AT_DECL_EXPLICIT_DNA_YAML
    AT_SUBDECL_DNA
    VectorElementFactory x0_PSIV;
    ModVectorElementFactory x4_PSVM;
    VectorElementFactory x8_PSOV;
    IntElementFactory xc_PSLT;
    IntElementFactory x10_PSWT;
    RealElementFactory x14_PSTS;
    VectorElementFactory x18_POFS;
    IntElementFactory x1c_SEED;
    RealElementFactory x20_LENG;
    RealElementFactory x24_WIDT;
    IntElementFactory x28_MAXP;
    RealElementFactory x2c_GRTE;
    ColorElementFactory x30_COLR;
    IntElementFactory x34_LTME;
    VectorElementFactory x38_ILOC;
    VectorElementFactory x3c_IVEC;
    EmitterElementFactory x40_EMTR;
    union
    {
        struct
        {
            bool x44_28_SORT : 1; bool x44_30_MBLR : 1;  bool x44_24_LINE : 1; bool x44_29_LIT_ : 1;
            bool x44_26_AAPH : 1; bool x44_27_ZBUF : 1;  bool x44_25_FXLL : 1; bool x44_31_PMAB : 1;
            bool x45_29_VMD4 : 1; bool x45_28_VMD3 : 1;  bool x45_27_VMD2 : 1; bool x45_26_VMD1 : 1;
            bool x45_31_OPTS : 1; bool x45_24_PMUS : 1;  bool x45_25_PMOO : 1; bool x45_30_CIND : 1;
        };
        uint16_t dummy1 = 0;
    };
    IntElementFactory x48_MBSP;
    RealElementFactory x4c_SIZE;
    RealElementFactory x50_ROTA;
    UVElementFactory<IDType> x54_TEXR;
    UVElementFactory<IDType> x58_TIND;
    ChildResourceFactory<IDType> x5c_PMDL;
    VectorElementFactory x6c_PMOP;
    VectorElementFactory x70_PMRT;
    VectorElementFactory x74_PMSC;
    ColorElementFactory x78_PMCL;
    ModVectorElementFactory x7c_VEL1;
    ModVectorElementFactory x80_VEL2;
    ModVectorElementFactory x84_VEL3;
    ModVectorElementFactory x88_VEL4;
    ChildResourceFactory<IDType> x8c_ICTS;
    IntElementFactory x9c_NCSY;
    IntElementFactory xa0_CSSD;
    ChildResourceFactory<IDType> xa4_IDTS;
    IntElementFactory xb4_NDSY;
    ChildResourceFactory<IDType> xb8_IITS;
    IntElementFactory xc8_PISY;
    IntElementFactory xcc_SISY;
    SpawnSystemKeyframeData<IDType> xd0_KSSM;
    ChildResourceFactory<IDType> xd4_SSWH;
    IntElementFactory xe4_SSSD;
    VectorElementFactory xe8_SSPO;
    IntElementFactory xf8_SESD;
    VectorElementFactory xfc_SEPO;
    ChildResourceFactory<IDType> xec_PMLC;
    IntElementFactory x100_LTYP;
    ColorElementFactory x104_LCLR;
    RealElementFactory x108_LINT;
    VectorElementFactory x10c_LOFF;
    VectorElementFactory x110_LDIR;
    IntElementFactory x114_LFOT;
    RealElementFactory x118_LFOR;
    RealElementFactory x11c_LSLA;

    /* 0-00 additions */
    ChildResourceFactory<IDType> xd8_SELC;
    union
    {
        struct
        {
            bool x30_30_ORNT : 1;
            bool x30_31_RSOP : 1;
        };
        uint16_t dummy2 = 0;
    };
    RealElementFactory x10c_ADV1;
    RealElementFactory x110_ADV2;
    RealElementFactory x114_ADV3;
    RealElementFactory x118_ADV4;
    RealElementFactory x11c_ADV5;
    RealElementFactory x120_ADV6;
    RealElementFactory x124_ADV7;
    RealElementFactory x128_ADV8;

    GPSM()
    {
        x45_25_PMOO = true;
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>&) const;
};

template <class IDType>
bool ExtractGPSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteGPSM(const GPSM<IDType>& gpsm, const hecl::ProjectPath& outPath);

}

