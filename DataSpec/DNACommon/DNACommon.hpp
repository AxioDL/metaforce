#ifndef __DNA_COMMON_HPP__
#define __DNA_COMMON_HPP__

#include <Athena/DNA.hpp>
#include <CFourCC.hpp>
#include <CUniqueID.hpp>

namespace Retro
{

/* This comes up a great deal */
typedef Athena::io::DNA<Athena::BigEndian> BigDNA;

/* FourCC DNA */
class DNAFourCC : public BigDNA, public CFourCC
{
public:
    Delete expl;
    void read(Athena::io::IStreamReader& reader)
    {_read(reader);}
    void write(Athena::io::IStreamWriter& writer) const
    {_write(writer);}
};

/* PAK 32-bit Unique ID DNA */
class DNAUniqueID32 : public BigDNA, public CUniqueID
{
public:
    Delete expl;
    void read(Athena::io::IStreamReader& reader)
    {_read(reader, E_32Bits);}
    void write(Athena::io::IStreamWriter& writer) const
    {_write(writer);}
};

/* PAK 64-bit Unique ID DNA */
class DNAUniqueID64 : public BigDNA, public CUniqueID
{
public:
    Delete expl;
    void read(Athena::io::IStreamReader& reader)
    {_read(reader, E_64Bits);}
    void write(Athena::io::IStreamWriter& writer) const
    {_write(writer);}
};

/* PAK 128-bit Unique ID DNA */
class DNAUniqueID128 : public BigDNA, public CUniqueID
{
public:
    Delete expl;
    void read(Athena::io::IStreamReader& reader)
    {_read(reader, E_128Bits);}
    void write(Athena::io::IStreamWriter& writer) const
    {_write(writer);}
};

}

#endif // __DNA_COMMON_HPP__
