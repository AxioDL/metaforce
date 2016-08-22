#include "athena/IStreamReader.hpp"
#include "athena/IStreamWriter.hpp"
#include "athena/FileWriter.hpp"
#include "DGRP.hpp"

namespace DataSpec
{
namespace DNADGRP
{
template <class IDType>
void DGRP<IDType>::read(athena::io::IStreamReader& __dna_reader)
{
    /* dependCount */
    dependCount = __dna_reader.readUint32Big();
    /* depends */
    __dna_reader.enumerate(depends, dependCount);
}

template <class IDType>
void DGRP<IDType>::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* dependCount */
    __dna_writer.writeUint32Big(dependCount);
    /* depends */
    __dna_writer.enumerate(depends);
}

template <class IDType>
void DGRP<IDType>::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* dependCount squelched */
    /* depends */
    dependCount = __dna_docin.enumerate("depends", depends);
}

template <class IDType>
void DGRP<IDType>::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* dependCount squelched */
    /* depends */
    __dna_docout.enumerate("depends", depends);
}

template <class IDType>
const char* DGRP<IDType>::DNAType()
{
    return "urde::DGRP";
}

template <class IDType>
size_t DGRP<IDType>::binarySize(size_t __isz) const
{
    __isz = __EnumerateSize(__isz, depends);
    return __isz + 4;
}

template <class IDType>
void DGRP<IDType>::ObjectTag::read(athena::io::IStreamReader& __dna_reader)
{
    /* type */
    type.read(__dna_reader);
    /* id */
    id.read(__dna_reader);
}

template <class IDType>
void DGRP<IDType>::ObjectTag::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* type */
    type.write(__dna_writer);
    /* id */
    id.write(__dna_writer);
}

template <class IDType>
void DGRP<IDType>::ObjectTag::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* type */
    __dna_docin.enumerate("type", type);
    /* id */
    __dna_docin.enumerate("id", id);
}

template <class IDType>
void DGRP<IDType>::ObjectTag::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* type */
    __dna_docout.enumerate("type", type);
    /* id */
    __dna_docout.enumerate("id", id);
}

template <class IDType>
const char* DGRP<IDType>::ObjectTag::DNAType()
{
    return "urde::DGRP::ObjectTag";
}

template <class IDType>
size_t DGRP<IDType>::ObjectTag::binarySize(size_t __isz) const
{
    __isz = type.binarySize(__isz);
    __isz = id.binarySize(__isz);
    return __isz;
}

template struct DGRP<UniqueID32>;
template struct DGRP<UniqueID64>;

template <class IDType>
bool ExtractDGRP(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter writer(outPath.getAbsolutePath());
    if (writer.isOpen())
    {
        DGRP<IDType> dgrp;
        dgrp.read(rs);
        dgrp.toYAMLStream(writer);
        return true;
    }
    return false;
}
template bool ExtractDGRP<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractDGRP<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteDGRP(const DGRP<IDType>& dgrp, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
    if (w.hasError())
        return false;
    dgrp.write(w);
    int64_t rem = w.position() % 32;
    if (rem)
        for (int64_t i=0 ; i<32-rem ; ++i)
            w.writeBytes((atInt8*)"\xff", 1);
    return true;
}
template bool WriteDGRP<UniqueID32>(const DGRP<UniqueID32>& dgrp, const hecl::ProjectPath& outPath);
template bool WriteDGRP<UniqueID64>(const DGRP<UniqueID64>& dgrp, const hecl::ProjectPath& outPath);
}
}
