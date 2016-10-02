#ifndef __COMMON_SWHC_HPP__
#define __COMMON_SWHC_HPP__

#include "ParticleCommon.hpp"
#include "PAK.hpp"
#include "athena/FileWriter.hpp"

namespace DataSpec
{
namespace DNAParticle
{

template <class IDType>
struct SWSH : public BigYAML
{
    static const char* DNAType() { return "SWSH"; }
    const char* DNATypeV() const { return DNAType(); }

    IntElementFactory x0_PSLT;
    RealElementFactory x4_TIME;
    RealElementFactory x8_LRAD;
    RealElementFactory xc_RRAD;
    IntElementFactory x10_LENG;
    ColorElementFactory x14_COLR;
    IntElementFactory x18_SIDE;
    RealElementFactory x1c_IROT;
    RealElementFactory x20_ROTM;
    VectorElementFactory x24_POFS;
    VectorElementFactory x28_IVEL;
    VectorElementFactory x2c_NPOS;
    ModVectorElementFactory x30_VELM;
    ModVectorElementFactory x34_VLM2;
    IntElementFactory x38_SPLN;
    UVElementFactory<IDType> x3c_TEXR;
    IntElementFactory x40_TSPN;
    union
    {
        struct
        {
            bool x44_24_LLRD : 1; bool x44_25_CROS : 1; bool x44_26_VLS1 : 1; bool x44_27_VLS2 : 1;
            bool x44_28_SROT : 1; bool x44_29_WIRE : 1; bool x44_30_TEXW : 1; bool x44_31_AALP : 1;
            bool x45_24_ZBUF : 1; bool x45_25_ORNT : 1; bool x45_26_CRND : 1;
        };
        uint16_t dummy = 0;
    };


    void read(athena::io::YAMLDocReader& r);
    void write(athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(athena::io::IStreamReader& r);
    void write(athena::io::IStreamWriter& w) const;

    SWSH()
    {
        x44_25_CROS = true;
    }

    void gatherDependencies(std::vector<hecl::ProjectPath>&) const;
};

template <class IDType>
bool ExtractSWSH(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteSWSH(const SWSH<IDType>& gpsm, const hecl::ProjectPath& outPath);
}
}
#endif // __COMMON_SWHC_HPP__
