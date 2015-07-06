/* Auto generated atdna implementation */
#include <Athena/Global.hpp>
#include <Athena/IStreamReader.hpp>
#include <Athena/IStreamWriter.hpp>

#include "PAK.hpp"

void Retro::DNAMP3::PAK::Header::read(Athena::io::IStreamReader& __dna_reader)
{
    __dna_reader.setEndian(Athena::BigEndian);
    /* version */
    version = __dna_reader.readUint32();
    /* headSz */
    headSz = __dna_reader.readUint32();
    /* md5sum[0] */
    md5sum[0] = __dna_reader.readUByte();
    /* md5sum[1] */
    md5sum[1] = __dna_reader.readUByte();
    /* md5sum[2] */
    md5sum[2] = __dna_reader.readUByte();
    /* md5sum[3] */
    md5sum[3] = __dna_reader.readUByte();
    /* md5sum[4] */
    md5sum[4] = __dna_reader.readUByte();
    /* md5sum[5] */
    md5sum[5] = __dna_reader.readUByte();
    /* md5sum[6] */
    md5sum[6] = __dna_reader.readUByte();
    /* md5sum[7] */
    md5sum[7] = __dna_reader.readUByte();
    /* md5sum[8] */
    md5sum[8] = __dna_reader.readUByte();
    /* md5sum[9] */
    md5sum[9] = __dna_reader.readUByte();
    /* md5sum[10] */
    md5sum[10] = __dna_reader.readUByte();
    /* md5sum[11] */
    md5sum[11] = __dna_reader.readUByte();
    /* md5sum[12] */
    md5sum[12] = __dna_reader.readUByte();
    /* md5sum[13] */
    md5sum[13] = __dna_reader.readUByte();
    /* md5sum[14] */
    md5sum[14] = __dna_reader.readUByte();
    /* md5sum[15] */
    md5sum[15] = __dna_reader.readUByte();
    /* seek */
    __dna_reader.seek(40, Athena::Current);
}

void Retro::DNAMP3::PAK::Header::write(Athena::io::IStreamWriter& __dna_writer) const
{
    __dna_writer.setEndian(Athena::BigEndian);
    /* version */
    __dna_writer.writeUint32(version);
    /* headSz */
    __dna_writer.writeUint32(headSz);
    /* md5sum[0] */
    __dna_writer.writeUByte(md5sum[0]);
    /* md5sum[1] */
    __dna_writer.writeUByte(md5sum[1]);
    /* md5sum[2] */
    __dna_writer.writeUByte(md5sum[2]);
    /* md5sum[3] */
    __dna_writer.writeUByte(md5sum[3]);
    /* md5sum[4] */
    __dna_writer.writeUByte(md5sum[4]);
    /* md5sum[5] */
    __dna_writer.writeUByte(md5sum[5]);
    /* md5sum[6] */
    __dna_writer.writeUByte(md5sum[6]);
    /* md5sum[7] */
    __dna_writer.writeUByte(md5sum[7]);
    /* md5sum[8] */
    __dna_writer.writeUByte(md5sum[8]);
    /* md5sum[9] */
    __dna_writer.writeUByte(md5sum[9]);
    /* md5sum[10] */
    __dna_writer.writeUByte(md5sum[10]);
    /* md5sum[11] */
    __dna_writer.writeUByte(md5sum[11]);
    /* md5sum[12] */
    __dna_writer.writeUByte(md5sum[12]);
    /* md5sum[13] */
    __dna_writer.writeUByte(md5sum[13]);
    /* md5sum[14] */
    __dna_writer.writeUByte(md5sum[14]);
    /* md5sum[15] */
    __dna_writer.writeUByte(md5sum[15]);
    /* seek */
    __dna_writer.seek(40, Athena::Current);
}

void Retro::DNAMP3::PAK::NameEntry::read(Athena::io::IStreamReader& __dna_reader)
{
    /* name */
    name = __dna_reader.readString(-1);
    /* type */
    type.read(__dna_reader);
    /* id */
    id.read(__dna_reader);
}

void Retro::DNAMP3::PAK::NameEntry::write(Athena::io::IStreamWriter& __dna_writer) const
{
    /* name */
    __dna_writer.writeString(name, -1);
    /* type */
    type.write(__dna_writer);
    /* id */
    id.write(__dna_writer);
}

void Retro::DNAMP3::PAK::Entry::read(Athena::io::IStreamReader& __dna_reader)
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

void Retro::DNAMP3::PAK::Entry::write(Athena::io::IStreamWriter& __dna_writer) const
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

