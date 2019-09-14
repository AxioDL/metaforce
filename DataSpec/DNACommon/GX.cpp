#include "GX.hpp"

namespace GX {

template <>
void Color::Enumerate<athena::io::DNA<athena::Endian::Big>::Read>(Read::StreamT& reader) {
  reader.readUBytesToBuf(&num, 4);
}
template <>
void Color::Enumerate<athena::io::DNA<athena::Endian::Big>::Write>(Write::StreamT& writer) {
  writer.writeUBytes(reinterpret_cast<const atUint8*>(&num), 4);
}
template <>
void Color::Enumerate<athena::io::DNA<athena::Endian::Big>::BinarySize>(BinarySize::StreamT& s) {
  s += 4;
}

}