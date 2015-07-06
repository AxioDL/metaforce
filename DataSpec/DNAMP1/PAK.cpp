/* Auto generated atdna implementation */
#include <Athena/Global.hpp>
#include <Athena/IStreamReader.hpp>
#include <Athena/IStreamWriter.hpp>

#include "PAK.hpp"

void Retro::DNAMP1::PAK::NameEntry::read(Athena::io::IStreamReader& __dna_reader)
{
    /* type */
    type.read(__dna_reader);
    /* id */
    id.read(__dna_reader);
    __dna_reader.setEndian(Athena::BigEndian);
    /* nameLen */
    nameLen = __dna_reader.readUint32();
    /* name */
    name = __dna_reader.readString(nameLen);
}

void Retro::DNAMP1::PAK::NameEntry::write(Athena::io::IStreamWriter& __dna_writer) const
{
    /* type */
    type.write(__dna_writer);
    /* id */
    id.write(__dna_writer);
    __dna_writer.setEndian(Athena::BigEndian);
    /* nameLen */
    __dna_writer.writeUint32(nameLen);
    /* name */
    __dna_writer.writeString(name, nameLen);
}

void Retro::DNAMP1::PAK::Entry::read(Athena::io::IStreamReader& __dna_reader)
{
    __dna_reader.setEndian(Athena::BigEndian);
    /* compressed */
    compressed = __dna_reader.readUint32();
    /* type */
    type.read(__dna_reader);
    /* id */
    id.read(__dna_reader);
    __dna_reader.setEndian(Athena::BigEndian);
    /* size */
    size = __dna_reader.readUint32();
    /* offset */
    offset = __dna_reader.readUint32();
}

void Retro::DNAMP1::PAK::Entry::write(Athena::io::IStreamWriter& __dna_writer) const
{
    __dna_writer.setEndian(Athena::BigEndian);
    /* compressed */
    __dna_writer.writeUint32(compressed);
    /* type */
    type.write(__dna_writer);
    /* id */
    id.write(__dna_writer);
    __dna_writer.setEndian(Athena::BigEndian);
    /* size */
    __dna_writer.writeUint32(size);
    /* offset */
    __dna_writer.writeUint32(offset);
}

