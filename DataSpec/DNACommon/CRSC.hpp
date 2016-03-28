#ifndef CRSC_HPP
#define CRSC_HPP

#include "ParticleCommon.hpp"
#include "PAK.hpp"
#include "athena/FileWriter.hpp"
#include "optional.hpp"

namespace DataSpec
{
namespace DNAParticle
{
template <class IDType>
struct CRSM : BigYAML
{
    static const char* DNAType() { return "CRSM"; }
    const char* DNATypeV() const { return DNAType(); }

    std::unordered_map<FourCC, ChildResourceFactory<IDType>> x0_generators;
    std::unordered_map<FourCC, uint32_t> x10_sfx;
    std::unordered_map<FourCC, ChildResourceFactory<IDType>> x20_decals;
    float x30_RNGE;
    float x34_FOFF;

    void read(athena::io::YAMLDocReader& r);
    void write(athena::io::YAMLDocWriter& w) const;
    size_t binarySize(size_t __isz) const;
    void read(athena::io::IStreamReader& r);
    void write(athena::io::IStreamWriter& w) const;

    CRSM();
};
template <class IDType>
bool ExtractCRSM(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteCRSM(const CRSM<IDType>& crsm, const hecl::ProjectPath& outPath);
}
}
#endif // CRSC_HPP
