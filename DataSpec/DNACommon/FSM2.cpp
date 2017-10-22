#include <athena/Global.hpp>
#include <athena/IStreamReader.hpp>
#include <athena/IStreamWriter.hpp>

#include "FSM2.hpp"

namespace DataSpec
{
namespace DNAFSM2
{
logvisor::Module LogDNAFSM2("urde::DNAFSM2");

template <class IDType>
void FSM2<IDType>::Header::read(athena::io::IStreamReader& __dna_reader)
{
    /* magic */
    magic.read(__dna_reader);
    /* version */
    version = __dna_reader.readUint32Big();
}

template <class IDType>
void FSM2<IDType>::Header::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* magic */
    magic.write(__dna_writer);
    /* version */
    __dna_writer.writeUint32Big(version);
}

template <class IDType>
void FSM2<IDType>::Header::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* magic */
    __dna_docin.enumerate("magic", magic);
    /* version */
    version = __dna_docin.readUint32("version");
}

template <class IDType>
void FSM2<IDType>::Header::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* magic */
    __dna_docout.enumerate("magic", magic);
    /* version */
    __dna_docout.writeUint32("version", version);
}

template <class IDType>
const char* FSM2<IDType>::Header::DNAType()
{
    return "FSM2::Header";
}

template <class IDType>
size_t FSM2<IDType>::Header::binarySize(size_t __isz) const
{
    __isz = magic.binarySize(__isz);
    return __isz + 4;
}

template <class IDType>
void FSM2<IDType>::CommonStruct::read(athena::io::IStreamReader& __dna_reader)
{
    /* name */
    name = __dna_reader.readString(-1);
    /* unknown */
    unknown = __dna_reader.readUint32Big();
}

template <class IDType>
void FSM2<IDType>::CommonStruct::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* name */
    __dna_writer.writeString(name, -1);
    /* unknown */
    __dna_writer.writeUint32Big(unknown);
}

template <class IDType>
void FSM2<IDType>::CommonStruct::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* name */
    name = __dna_docin.readString("name");
    /* unknown */
    unknown = __dna_docin.readUint32("unknown");
}

template <class IDType>
void FSM2<IDType>::CommonStruct::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* name */
    __dna_docout.writeString("name", name);
    /* unknown */
    __dna_docout.writeUint32("unknown", unknown);
}

template <class IDType>
const char* FSM2<IDType>::CommonStruct::DNAType()
{
    return "FSM2::CommonStruct";
}

template <class IDType>
size_t FSM2<IDType>::CommonStruct::binarySize(size_t __isz) const
{
    __isz += name.size() + 1;
    __isz += 4;
    return __isz;
}

template <class IDType>
void FSM2<IDType>::FSMV1::read(athena::io::IStreamReader& __dna_reader)
{
    /* stateCount */
    stateCount = __dna_reader.readUint32Big();
    /* unknown1Count */
    unknown1Count = __dna_reader.readUint32Big();
    /* unknown2Count */
    unknown2Count = __dna_reader.readUint32Big();
    /* unknown3Count */
    unknown3Count = __dna_reader.readUint32Big();
    /* states */
    __dna_reader.enumerate(states, stateCount);
    /* unknown1 */
    __dna_reader.enumerate(unknown1, unknown1Count);
    /* unknown2 */
    __dna_reader.enumerate(unknown2, unknown2Count);
    /* unknown3 */
    __dna_reader.enumerate(unknown3, unknown3Count);
}

template <class IDType>
void FSM2<IDType>::FSMV1::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* stateCount */
    __dna_writer.writeUint32Big(stateCount);
    /* unknown1Count */
    __dna_writer.writeUint32Big(unknown1Count);
    /* unknown2Count */
    __dna_writer.writeUint32Big(unknown2Count);
    /* unknown3Count */
    __dna_writer.writeUint32Big(unknown3Count);
    /* states */
    __dna_writer.enumerate(states);
    /* unknown1 */
    __dna_writer.enumerate(unknown1);
    /* unknown2 */
    __dna_writer.enumerate(unknown2);
    /* unknown3 */
    __dna_writer.enumerate(unknown3);
}

template <class IDType>
void FSM2<IDType>::FSMV1::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* states */
    stateCount = __dna_docin.enumerate("states", states);
    /* unknown1 */
    unknown1Count = __dna_docin.enumerate("unknown1", unknown1);
    /* unknown2 */
    unknown2Count = __dna_docin.enumerate("unknown2", unknown2);
    /* unknown3 */
    unknown3Count = __dna_docin.enumerate("unknown3", unknown3);
}

template <class IDType>
void FSM2<IDType>::FSMV1::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* states */
    __dna_docout.enumerate("states", states);
    /* unknown1 */
    __dna_docout.enumerate("unknown1", unknown1);
    /* unknown2 */
    __dna_docout.enumerate("unknown2", unknown2);
    /* unknown3 */
    __dna_docout.enumerate("unknown3", unknown3);
}

template <class IDType>
const char* FSM2<IDType>::FSMV1::DNAType()
{
    return "FSM2::FSMV1";
}

template <class IDType>
size_t FSM2<IDType>::FSMV1::binarySize(size_t __isz) const
{
    __isz = __EnumerateSize(__isz, states);
    __isz = __EnumerateSize(__isz, unknown1);
    __isz = __EnumerateSize(__isz, unknown2);
    __isz = __EnumerateSize(__isz, unknown3);
    return __isz + 16;
}

template <class IDType>
void FSM2<IDType>::FSMV1::State::read(athena::io::IStreamReader& __dna_reader)
{
    /* name */
    name = __dna_reader.readString(-1);
    /* unknownCount */
    unknownCount = __dna_reader.readUint32Big();
    /* unknown */
    __dna_reader.enumerate(unknown, unknownCount);
}

template <class IDType>
void FSM2<IDType>::FSMV1::State::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* name */
    __dna_writer.writeString(name, -1);
    /* unknownCount */
    __dna_writer.writeUint32Big(unknownCount);
    /* unknown */
    __dna_writer.enumerate(unknown);
}

template <class IDType>
void FSM2<IDType>::FSMV1::State::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* name */
    name = __dna_docin.readString("name");
    /* unknownCount squelched */
    /* unknown */
    unknownCount = __dna_docin.enumerate("unknown", unknown);
}

template <class IDType>
void FSM2<IDType>::FSMV1::State::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* name */
    __dna_docout.writeString("name", name);
    /* unknownCount squelched */
    /* unknown */
    __dna_docout.enumerate("unknown", unknown);
}

template <class IDType>
const char* FSM2<IDType>::FSMV1::State::DNAType()
{
    return "FSM2::FSMV1::State";
}

template <class IDType>
size_t FSM2<IDType>::FSMV1::State::binarySize(size_t __isz) const
{
    __isz += name.size() + 1;
    __isz = __EnumerateSize(__isz, unknown);
    return __isz + 4;
}

template <class IDType>
void FSM2<IDType>::FSMV1::Unknown1::read(athena::io::IStreamReader& __dna_reader)
{
    /* name */
    name = __dna_reader.readString(-1);
    /* unknown1 */
    unknown1 = __dna_reader.readFloatBig();
    /* unknown2Count */
    unknown2Count = __dna_reader.readUint32Big();
    /* unknown2 */
    __dna_reader.enumerate(unknown2, unknown2Count);
    /* unknown3 */
    unknown3 = __dna_reader.readUByte();
}

template <class IDType>
void FSM2<IDType>::FSMV1::Unknown1::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* name */
    __dna_writer.writeString(name, -1);
    /* unknown1 */
    __dna_writer.writeFloatBig(unknown1);
    /* unknown2Count */
    __dna_writer.writeUint32Big(unknown2Count);
    /* unknown2 */
    __dna_writer.enumerate(unknown2);
    /* unknown3 */
    __dna_writer.writeUByte(unknown3);
}

template <class IDType>
void FSM2<IDType>::FSMV1::Unknown1::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* name */
    name = __dna_docin.readString("name");
    /* unknown1 */
    unknown1 = __dna_docin.readFloat("unknown1");
    /* unknown2Count squelched */
    /* unknown2 */
    unknown2Count = __dna_docin.enumerate("unknown2", unknown2);
    /* unknown3 */
    unknown3 = __dna_docin.readUByte("unknown3");
}

template <class IDType>
void FSM2<IDType>::FSMV1::Unknown1::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* name */
    __dna_docout.writeString("name", name);
    /* unknown1 */
    __dna_docout.writeFloat("unknown1", unknown1);
    /* unknown2Count squelched */
    /* unknown2 */
    __dna_docout.enumerate("unknown2", unknown2);
    /* unknown3 */
    __dna_docout.writeUByte("unknown3", unknown3);
}

template <class IDType>
const char* FSM2<IDType>::FSMV1::Unknown1::DNAType()
{
    return "FSM2::FSMV1::Unknown1";
}

template <class IDType>
size_t FSM2<IDType>::FSMV1::Unknown1::binarySize(size_t __isz) const
{
    __isz += name.size() + 1;
    __isz = __EnumerateSize(__isz, unknown2);
    return __isz + 9;
}

template <class IDType>
void FSM2<IDType>::FSMV1::Unknown2::read(athena::io::IStreamReader& __dna_reader)
{
    /* name */
    name = __dna_reader.readString(-1);
    /* unknownCount */
    unknownCount = __dna_reader.readUint32Big();
    /* unknown */
    __dna_reader.enumerate(unknown, unknownCount);
}

template <class IDType>
void FSM2<IDType>::FSMV1::Unknown2::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* name */
    __dna_writer.writeString(name, -1);
    /* unknownCount */
    __dna_writer.writeUint32Big(unknownCount);
    /* unknown */
    __dna_writer.enumerate(unknown);
}

template <class IDType>
void FSM2<IDType>::FSMV1::Unknown2::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* name */
    name = __dna_docin.readString("name");
    /* unknownCount squelched */
    /* unknown */
    unknownCount = __dna_docin.enumerate("unknown", unknown);
}

template <class IDType>
void FSM2<IDType>::FSMV1::Unknown2::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* name */
    __dna_docout.writeString("name", name);
    /* unknownCount squelched */
    /* unknown */
    __dna_docout.enumerate("unknown", unknown);
}

template <class IDType>
const char* FSM2<IDType>::FSMV1::Unknown2::DNAType()
{
    return "FSM2::FSMV1::Unknown2";
}

template <class IDType>
size_t FSM2<IDType>::FSMV1::Unknown2::binarySize(size_t __isz) const
{
    __isz += name.size() + 1;
    __isz = __EnumerateSize(__isz, unknown);
    return __isz + 4;
}

template <class IDType>
void FSM2<IDType>::FSMV1::Unknown3::read(athena::io::IStreamReader& __dna_reader)
{
    /* name */
    name = __dna_reader.readString(-1);
    /* unknownCount */
    unknownCount = __dna_reader.readUint32Big();
    /* unknown */
    __dna_reader.enumerate(unknown, unknownCount);
    /* fsmId */
    fsmId.read(__dna_reader);
}

template <class IDType>
void FSM2<IDType>::FSMV1::Unknown3::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* name */
    __dna_writer.writeString(name, -1);
    /* unknownCount */
    __dna_writer.writeUint32Big(unknownCount);
    /* unknown */
    __dna_writer.enumerate(unknown);
    /* fsmId */
    fsmId.write(__dna_writer);
}

template <class IDType>
void FSM2<IDType>::FSMV1::Unknown3::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* name */
    name = __dna_docin.readString("name");
    /* unknownCount squelched */
    /* unknown */
    unknownCount = __dna_docin.enumerate("unknown", unknown);
    /* fsmId */
    __dna_docin.enumerate("fsmId", fsmId);
}

template <class IDType>
void FSM2<IDType>::FSMV1::Unknown3::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* name */
    __dna_docout.writeString("name", name);
    /* unknownCount squelched */
    /* unknown */
    __dna_docout.enumerate("unknown", unknown);
    /* fsmId */
    __dna_docout.enumerate("fsmId", fsmId);

}

template <class IDType>
const char* FSM2<IDType>::FSMV1::Unknown3::DNAType()
{
    return "FSM2::FSMV1::Unknown3";
}

template <class IDType>
size_t FSM2<IDType>::FSMV1::Unknown3::binarySize(size_t __isz) const
{
    __isz += name.size() + 1;
    __isz = __EnumerateSize(__isz, unknown);
    __isz = fsmId.binarySize(__isz);
    return __isz + 4;
}

template <class IDType>
void FSM2<IDType>::FSMV2::read(athena::io::IStreamReader& __dna_reader)
{
    /* stateCount */
    stateCount = __dna_reader.readUint32Big();
    /* unknown1Count */
    unknown1Count = __dna_reader.readUint32Big();
    /* unknown2Count */
    unknown2Count = __dna_reader.readUint32Big();
    /* unknown3Count */
    unknown3Count = __dna_reader.readUint32Big();
    /* states */
    __dna_reader.enumerate(states, stateCount);
    /* unknown1 */
    __dna_reader.enumerate(unknown1, unknown1Count);
    /* unknown2 */
    __dna_reader.enumerate(unknown2, unknown2Count);
    /* unknown3 */
    __dna_reader.enumerate(unknown3, unknown3Count);
}

template <class IDType>
void FSM2<IDType>::FSMV2::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* stateCount */
    __dna_writer.writeUint32Big(stateCount);
    /* unknown1Count */
    __dna_writer.writeUint32Big(unknown1Count);
    /* unknown2Count */
    __dna_writer.writeUint32Big(unknown2Count);
    /* unknown3Count */
    __dna_writer.writeUint32Big(unknown3Count);
    /* states */
    __dna_writer.enumerate(states);
    /* unknown1 */
    __dna_writer.enumerate(unknown1);
    /* unknown2 */
    __dna_writer.enumerate(unknown2);
    /* unknown3 */
    __dna_writer.enumerate(unknown3);
}

template <class IDType>
void FSM2<IDType>::FSMV2::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* states */
    stateCount = __dna_docin.enumerate("states", states);
    /* unknown1 */
    unknown1Count = __dna_docin.enumerate("unknown1", unknown1);
    /* unknown2 */
    unknown2Count = __dna_docin.enumerate("unknown2", unknown2);
    /* unknown3 */
    unknown3Count = __dna_docin.enumerate("unknown3", unknown3);
}

template <class IDType>
void FSM2<IDType>::FSMV2::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* states */
    __dna_docout.enumerate("states", states);
    /* unknown1 */
    __dna_docout.enumerate("unknown1", unknown1);
    /* unknown2 */
    __dna_docout.enumerate("unknown2", unknown2);
    /* unknown3 */
    __dna_docout.enumerate("unknown3", unknown3);
}

template <class IDType>
const char* FSM2<IDType>::FSMV2::DNAType()
{
    return "FSM2::FSMV2";
}

template <class IDType>
size_t FSM2<IDType>::FSMV2::binarySize(size_t __isz) const
{
    __isz = __EnumerateSize(__isz, states);
    __isz = __EnumerateSize(__isz, unknown1);
    __isz = __EnumerateSize(__isz, unknown2);
    __isz = __EnumerateSize(__isz, unknown3);
    return __isz + 16;
}

template <class IDType>
void FSM2<IDType>::FSMV2::State::read(athena::io::IStreamReader& __dna_reader)
{
    /* name */
    name = __dna_reader.readString(-1);
    /* unknown1 */
    unknown1 = __dna_reader.readUint32Big();
    /* unknown2 */
    unknown2 = __dna_reader.readUint32Big();
    /* unknown3 */
    unknown3 = __dna_reader.readUint32Big();
    /* unknown4 */
    unknown4 = __dna_reader.readUint32Big();
    /* unknown5Count */
    unknown5Count = __dna_reader.readUint32Big();
    /* unknown5 */
    __dna_reader.enumerate(unknown5, unknown5Count);
}

template <class IDType>
void FSM2<IDType>::FSMV2::State::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* name */
    __dna_writer.writeString(name, -1);
    /* unknown1 */
    __dna_writer.writeUint32Big(unknown1);
    /* unknown2 */
    __dna_writer.writeUint32Big(unknown2);
    /* unknown3 */
    __dna_writer.writeUint32Big(unknown3);
    /* unknown4 */
    __dna_writer.writeUint32Big(unknown4);
    /* unknown5Count */
    __dna_writer.writeUint32Big(unknown5Count);
    /* unknown5 */
    __dna_writer.enumerate(unknown5);
}

template <class IDType>
void FSM2<IDType>::FSMV2::State::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* name */
    name = __dna_docin.readString("name");
    /* unknown1 */
    unknown1 = __dna_docin.readUint32("unknown1");
    /* unknown2 */
    unknown2 = __dna_docin.readUint32("unknown2");
    /* unknown3 */
    unknown3 = __dna_docin.readUint32("unknown3");
    /* unknown4 */
    unknown4 = __dna_docin.readUint32("unknown4");
    /* unknown5Count squelched */
    /* unknown5 */
    unknown5Count = __dna_docin.enumerate("unknown5", unknown5);
}

template <class IDType>
void FSM2<IDType>::FSMV2::State::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* name */
    __dna_docout.writeString("name", name);
    /* unknown1 */
    __dna_docout.writeUint32("unknown1", unknown1);
    /* unknown2 */
    __dna_docout.writeUint32("unknown2", unknown2);
    /* unknown3 */
    __dna_docout.writeUint32("unknown3", unknown3);
    /* unknown4 */
    __dna_docout.writeUint32("unknown4", unknown4);
    /* unknown5Count squelched */
    /* unknown5 */
    __dna_docout.enumerate("unknown5", unknown5);
}

template <class IDType>
const char* FSM2<IDType>::FSMV2::State::DNAType()
{
    return "FSM2::FSMV2::State";
}

template <class IDType>
size_t FSM2<IDType>::FSMV2::State::binarySize(size_t __isz) const
{
    __isz += name.size() + 1;
    __isz = __EnumerateSize(__isz, unknown5);
    return __isz + 20;
}

template <class IDType>
void FSM2<IDType>::FSMV2::Unknown1::read(athena::io::IStreamReader& __dna_reader)
{
    /* name */
    name = __dna_reader.readString(-1);
    /* unknown1 */
    unknown1 = __dna_reader.readUint32Big();
    /* unknown2 */
    unknown2 = __dna_reader.readUint32Big();
    /* unknown3 */
    unknown3 = __dna_reader.readUint32Big();
    /* unknown4 */
    unknown4 = __dna_reader.readUint32Big();
    /* unknown5 */
    unknown5 = __dna_reader.readFloatBig();
    /* unknown6Count */
    unknown6Count = __dna_reader.readUint32Big();
    /* unknown6 */
    __dna_reader.enumerate(unknown6, unknown6Count);
    /* unknown7 */
    unknown7 = __dna_reader.readUByte();
}

template <class IDType>
void FSM2<IDType>::FSMV2::Unknown1::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* name */
    __dna_writer.writeString(name, -1);
    /* unknown1 */
    __dna_writer.writeUint32Big(unknown1);
    /* unknown2 */
    __dna_writer.writeUint32Big(unknown2);
    /* unknown3 */
    __dna_writer.writeUint32Big(unknown3);
    /* unknown4 */
    __dna_writer.writeUint32Big(unknown4);
    /* unknown5 */
    __dna_writer.writeUint32Big(unknown5);
    /* unknown6Count */
    __dna_writer.writeFloatBig(unknown6Count);
    /* unknown6 */
    __dna_writer.enumerate(unknown6);
    /* unknown6 */
    __dna_writer.writeUByte(unknown7);
}

template <class IDType>
void FSM2<IDType>::FSMV2::Unknown1::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* name */
    name = __dna_docin.readString("name");
    /* unknown1 */
    unknown1 = __dna_docin.readUint32("unknown1");
    /* unknown2 */
    unknown2 = __dna_docin.readUint32("unknown2");
    /* unknown3 */
    unknown3 = __dna_docin.readUint32("unknown3");
    /* unknown4 */
    unknown4 = __dna_docin.readUint32("unknown4");
    /* unknown5 */
    unknown5 = __dna_docin.readFloat("unknown5");
    /* unknown6Count squelched */
    /* unknown6 */
    unknown6Count = __dna_docin.enumerate("unknown6", unknown6);
    /* unknown7 */
    unknown7 = __dna_docin.readUByte("unknown7");
}

template <class IDType>
void FSM2<IDType>::FSMV2::Unknown1::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* name */
    __dna_docout.writeString("name", name);
    /* unknown1 */
    __dna_docout.writeUint32("unknown1", unknown1);
    /* unknown2 */
    __dna_docout.writeUint32("unknown2", unknown2);
    /* unknown3 */
    __dna_docout.writeUint32("unknown3", unknown3);
    /* unknown4 */
    __dna_docout.writeUint32("unknown4", unknown4);
    /* unknown5 */
    __dna_docout.writeFloat("unknown5", unknown5);
    /* unknown6Count squelched */
    /* unknown6 */
    __dna_docout.enumerate("unknown6", unknown6);
    /* unknown7 */
    __dna_docout.writeUByte("unknown7", unknown7);
}

template <class IDType>
const char* FSM2<IDType>::FSMV2::Unknown1::DNAType()
{
    return "FSM2::FSMV2::Unknown1";
}

template <class IDType>
size_t FSM2<IDType>::FSMV2::Unknown1::binarySize(size_t __isz) const
{
    __isz += name.size() + 1;
    __isz = __EnumerateSize(__isz, unknown6);
    return __isz + 25;
}

template <class IDType>
void FSM2<IDType>::FSMV2::Unknown2::read(athena::io::IStreamReader& __dna_reader)
{
    /* name */
    name = __dna_reader.readString(-1);
    /* unknown1 */
    unknown1 = __dna_reader.readUint32Big();
    /* unknown2 */
    unknown2 = __dna_reader.readUint32Big();
    /* unknown3 */
    unknown3 = __dna_reader.readUint32Big();
    /* unknown4 */
    unknown4 = __dna_reader.readUint32Big();
    /* unknown5Count */
    unknown5Count = __dna_reader.readUint32Big();
    /* unknown5 */
    __dna_reader.enumerate(unknown5, unknown5Count);
}

template <class IDType>
void FSM2<IDType>::FSMV2::Unknown2::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* name */
    __dna_writer.writeString(name, -1);
    /* unknown1 */
    __dna_writer.writeUint32Big(unknown1);
    /* unknown2 */
    __dna_writer.writeUint32Big(unknown2);
    /* unknown3 */
    __dna_writer.writeUint32Big(unknown3);
    /* unknown4 */
    __dna_writer.writeUint32Big(unknown4);
    /* unknown5Count */
    __dna_writer.writeUint32Big(unknown5Count);
    /* unknown5 */
    __dna_writer.enumerate(unknown5);
}

template <class IDType>
void FSM2<IDType>::FSMV2::Unknown2::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* name */
    name = __dna_docin.readString("name");
    /* unknown1 */
    unknown1 = __dna_docin.readUint32("unknown1");
    /* unknown2 */
    unknown2 = __dna_docin.readUint32("unknown2");
    /* unknown3 */
    unknown3 = __dna_docin.readUint32("unknown3");
    /* unknown4 */
    unknown4 = __dna_docin.readUint32("unknown4");
    /* unknown5Count squelched */
    /* unknown5 */
    unknown5Count = __dna_docin.enumerate("unknown5", unknown5);
}

template <class IDType>
void FSM2<IDType>::FSMV2::Unknown2::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* name */
    __dna_docout.writeString("name", name);
    /* unknown1 */
    __dna_docout.writeUint32("unknown1", unknown1);
    /* unknown2 */
    __dna_docout.writeUint32("unknown2", unknown2);
    /* unknown3 */
    __dna_docout.writeUint32("unknown3", unknown3);
    /* unknown4 */
    __dna_docout.writeUint32("unknown4", unknown4);
    /* unknown5Count squelched */
    /* unknown5 */
    __dna_docout.enumerate("unknown5", unknown5);
}

template <class IDType>
const char* FSM2<IDType>::FSMV2::Unknown2::DNAType()
{
    return "FSM2::FSMV2::Unknown2";
}

template <class IDType>
size_t FSM2<IDType>::FSMV2::Unknown2::binarySize(size_t __isz) const
{
    __isz += name.size() + 1;
    __isz = __EnumerateSize(__isz, unknown5);
    return __isz + 20;
}

template <class IDType>
void FSM2<IDType>::FSMV2::Unknown3::read(athena::io::IStreamReader& __dna_reader)
{
    /* name */
    name = __dna_reader.readString(-1);
    /* unknown1 */
    unknown1 = __dna_reader.readUint32Big();
    /* unknown2 */
    unknown2 = __dna_reader.readUint32Big();
    /* unknown3 */
    unknown3 = __dna_reader.readUint32Big();
    /* unknown4 */
    unknown4 = __dna_reader.readUint32Big();
    /* unknown5Count */
    unknown5Count = __dna_reader.readUint32Big();
    /* unknown5 */
    __dna_reader.enumerate(unknown5, unknown5Count);
    /* fsmId */
    fsmId.read(__dna_reader);
}

template <class IDType>
void FSM2<IDType>::FSMV2::Unknown3::write(athena::io::IStreamWriter& __dna_writer) const
{
    /* name */
    __dna_writer.writeString(name, -1);
    /* unknown1 */
    __dna_writer.writeUint32Big(unknown1);
    /* unknown2 */
    __dna_writer.writeUint32Big(unknown2);
    /* unknown3 */
    __dna_writer.writeUint32Big(unknown3);
    /* unknown4 */
    __dna_writer.writeUint32Big(unknown4);
    /* unknown5Count */
    __dna_writer.writeUint32Big(unknown5Count);
    /* unknown5 */
    __dna_writer.enumerate(unknown5);
    /* fsmId */
    fsmId.write(__dna_writer);
}

template <class IDType>
void FSM2<IDType>::FSMV2::Unknown3::read(athena::io::YAMLDocReader& __dna_docin)
{
    /* name */
    name = __dna_docin.readString("name");
    /* unknown1 */
    unknown1 = __dna_docin.readUint32("unknown1");
    /* unknown2 */
    unknown2 = __dna_docin.readUint32("unknown2");
    /* unknown3 */
    unknown3 = __dna_docin.readUint32("unknown3");
    /* unknown4 */
    unknown4 = __dna_docin.readUint32("unknown4");
    /* unknown5Count squelched */
    /* unknown5 */
    unknown5Count = __dna_docin.enumerate("unknown5", unknown5);
    /* fsmId */
    __dna_docin.enumerate("fsmId", fsmId);
}

template <class IDType>
void FSM2<IDType>::FSMV2::Unknown3::write(athena::io::YAMLDocWriter& __dna_docout) const
{
    /* name */
    __dna_docout.writeString("name", name);
    /* unknown1 */
    __dna_docout.writeUint32("unknown1", unknown1);
    /* unknown2 */
    __dna_docout.writeUint32("unknown2", unknown2);
    /* unknown3 */
    __dna_docout.writeUint32("unknown3", unknown3);
    /* unknown4 */
    __dna_docout.writeUint32("unknown4", unknown4);
    /* unknown5Count squelched */
    /* unknown5 */
    __dna_docout.enumerate("unknown5", unknown5);
    /* fsmId */
    __dna_docout.enumerate("fsmId", fsmId);
}

template <class IDType>
const char* FSM2<IDType>::FSMV2::Unknown3::DNAType()
{
    return "FSM2::FSMV2::Unknown3";
}

template <class IDType>
size_t FSM2<IDType>::FSMV2::Unknown3::binarySize(size_t __isz) const
{
    __isz += name.size() + 1;
    __isz = __EnumerateSize(__isz, unknown5);
    __isz = fsmId.binarySize(__isz);
    return __isz + 20;
}


template <class IDType>
void FSM2<IDType>::read(athena::io::IStreamReader& in)
{
    header.read(in);
    if (header.magic != SBIG('FSM2'))
    {
        LogDNAFSM2.report(logvisor::Fatal, "Invalid FSM2 magic '%.4s' expected 'FSM2'", header.magic.toString().c_str());
        return;
    }

    if (header.version == 1)
        detail.reset(new FSMV1);
    else if (header.version == 2)
        detail.reset(new FSMV2);
    else
    {
        LogDNAFSM2.report(logvisor::Fatal, "Invalid FSM2 version '%i'", header.version);
        return;
    }

    detail->read(in);
}

template <class IDType>
void FSM2<IDType>::write(athena::io::IStreamWriter& out) const
{
    header.write(out);
    detail->write(out);
}

template <class IDType>
void FSM2<IDType>::read(athena::io::YAMLDocReader& in)
{
    header.read(in);
    if (header.magic != SBIG('FSM2'))
    {
        LogDNAFSM2.report(logvisor::Fatal, "Invalid FSM2 magic '%.4s' expected 'FSM2'", header.magic.toString().c_str());
        return;
    }

    if (header.version == 1)
        detail.reset(new FSMV1);
    else if (header.version == 2)
        detail.reset(new FSMV2);
    else
    {
        LogDNAFSM2.report(logvisor::Fatal, "Invalid FSM2 version '%i'", header.version);
        return;
    }

    detail->read(in);
}

template <class IDType>
void FSM2<IDType>::write(athena::io::YAMLDocWriter& out) const
{
    header.write(out);
    detail->write(out);
}

template <class IDType>
size_t FSM2<IDType>::binarySize(size_t __isz) const
{
    __isz = header.binarySize(__isz);
    return detail->binarySize(__isz);
}

template <class IDType>
const char* FSM2<IDType>::DNAType() { return "urde::FSM2"; }

template <class IDType>
const char* FSM2<IDType>::DNATypeV() const { return FSM2<IDType>::DNAType(); }

template struct FSM2<UniqueID32>;
template struct FSM2<UniqueID64>;

template <class IDType>
bool ExtractFSM2(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter writer(outPath.getAbsolutePath());
    if (writer.isOpen())
    {
        FSM2<IDType> fsm2;
        fsm2.read(rs);
        fsm2.toYAMLStream(writer);
        return true;
    }
    return false;
}
template bool ExtractFSM2<UniqueID32>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);
template bool ExtractFSM2<UniqueID64>(PAKEntryReadStream& rs, const hecl::ProjectPath& outPath);

template <class IDType>
bool WriteFSM2(const FSM2<IDType>& fsm2, const hecl::ProjectPath& outPath)
{
    athena::io::FileWriter w(outPath.getAbsolutePath(), true, false);
    if (w.hasError())
        return false;
    fsm2.write(w);
    int64_t rem = w.position() % 32;
    if (rem)
        for (int64_t i=0 ; i<32-rem ; ++i)
            w.writeUByte(0xff);
    return true;
}
template bool WriteFSM2<UniqueID32>(const FSM2<UniqueID32>& fsm2, const hecl::ProjectPath& outPath);
template bool WriteFSM2<UniqueID64>(const FSM2<UniqueID64>& fsm2, const hecl::ProjectPath& outPath);

}
}
