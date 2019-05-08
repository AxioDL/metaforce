#include "GX.hpp"

namespace GX {

template <>
void Color::Enumerate<athena::io::DNA<athena::Big>::Read>(typename Read::StreamT& reader) {
  reader.readUBytesToBuf(&num, 4);
}
template <>
void Color::Enumerate<athena::io::DNA<athena::Big>::Write>(typename Write::StreamT& writer) {
  writer.writeUBytes(reinterpret_cast<const atUint8*>(&num), 4);
}
template <>
void Color::Enumerate<athena::io::DNA<athena::Big>::BinarySize>(typename BinarySize::StreamT& s) {
  s += 4;
}

}