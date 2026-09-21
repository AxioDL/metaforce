#include "MetroidPrime/CMapArea.hpp"

#include "Kyoto/Streams/CInputStream.hpp"
#include "Metaforce/CResourceReader.hpp"
#include "Metaforce/Common.hpp"
#include "MetroidPrime/CMemoryDrawEnum.hpp"

#include <array>
#include <dolphin/gx/GXEnum.h>

namespace {
constexpr borealis::Log Log{"CMapArea"};

const int* ReadCommands(std::span< const uchar > data, uint offset, uint vertexCount,
                        size_t commandStart, bool outlines) {
  REQUIRE(offset >= commandStart && offset <= data.size() && offset % 4 == 0,
          "Invalid MAPA command offset {}", offset);
  CResourceReader in(data.subspan(offset), Log.name);
  const uint count = in.ReadCount(outlines ? 4 : 8);
  for (uint i = 0; i < count; ++i) {
    if (!outlines) {
      const uint primitive = in.Read< uint >();
      REQUIRE(primitive == GX_QUADS || primitive == GX_TRIANGLES || primitive == GX_TRIANGLESTRIP ||
                  primitive == GX_TRIANGLEFAN || primitive == GX_LINES ||
                  primitive == GX_LINESTRIP || primitive == GX_POINTS,
              "Invalid MAPA primitive {}", primitive);
    }
    const uint vertices = in.Read< uint >();
    REQUIRE(vertices <= UINT16_MAX, "MAPA primitive has too many vertices: {}", vertices);
    const auto indices = in.Take((vertices + 3) & ~size_t{3}).first(vertices);
    for (uchar index : indices) {
      REQUIRE(index < vertexCount, "MAPA vertex index {} exceeds {} vertices", index, vertexCount);
    }
  }
  return reinterpret_cast< const int* >(data.data() + offset);
}

CMappableObject::EMappableObjectType ReadObjectType(CResourceReader& in) {
  const uint type = in.Read< uint >();
  REQUIRE(type <= CMappableObject::kMOT_MissileStation, "Invalid MAPA object type {}", type);
  return static_cast< CMappableObject::EMappableObjectType >(type);
}

CMappableObject::EVisMode ReadObjectVisibility(CResourceReader& in) {
  const uint visibility = in.Read< uint >();
  REQUIRE(visibility <= CMappableObject::kVM_MapStationOrVisit2,
          "Invalid MAPA object visibility {}", visibility);
  return static_cast< CMappableObject::EVisMode >(visibility);
}
} // namespace

CMapArea::CMapArea(CInputStream& in, uint size) {
  REQUIRE(size >= 52 && size <= INT32_MAX, "Invalid MAPA resource size {}", size);
  std::array< uchar, 52 > header;
  REQUIRE(in.ReadBytes(header.data(), header.size()) == header.size(), "Truncated MAPA header");
  CResourceReader reader(header, Log.name);
  x0_magic = reader.Read< uint >();
  x4_version = reader.Read< uint >();
  REQUIRE(x0_magic == 0xdeadd00d, "Invalid MAPA magic {:#x}", x0_magic);
  REQUIRE(x4_version == 2, "Unsupported MAPA version {}", x4_version);
  x8_ = reader.Read< uint >();
  const uint visibility = reader.Read< uint >();
  REQUIRE(visibility <= kVM_Never, "Invalid MAPA visibility {}", visibility);
  xc_visibilityMode = static_cast< EVisMode >(visibility);
  x10_box = reader.ReadAABox();
  x28_mappableObjCount = reader.Read< int >();
  x2c_vertexCount = reader.Read< int >();
  x30_surfaceCount = reader.Read< int >();
  x34_size = size - 52;
  REQUIRE(x28_mappableObjCount >= 0 && x2c_vertexCount >= 0 && x30_surfaceCount >= 0,
          "Negative MAPA record count");
  REQUIRE(static_cast< uint >(x28_mappableObjCount) <= x34_size / 80 &&
              static_cast< uint >(x2c_vertexCount) <= x34_size / 12 &&
              static_cast< uint >(x30_surfaceCount) <= x34_size / 32,
          "MAPA record count exceeds resource");
  const size_t objectBytes = static_cast< size_t >(x28_mappableObjCount) * 80;
  const size_t vertexBytes = static_cast< size_t >(x2c_vertexCount) * 12;
  const size_t surfaceBytes = static_cast< size_t >(x30_surfaceCount) * 32;
  REQUIRE(objectBytes <= x34_size && vertexBytes <= x34_size - objectBytes &&
              surfaceBytes <= x34_size - objectBytes - vertexBytes,
          "MAPA records exceed resource");
  x44_buf = rs_new uchar[x34_size];
  REQUIRE(in.ReadBytes(x44_buf.get(), x34_size) == x34_size, "Truncated MAPA body");

  CResourceReader body({x44_buf.get(), x34_size}, Log.name);
  mObjects.reserve(x28_mappableObjCount);
  for (int i = 0; i < x28_mappableObjCount; ++i) {
    mObjects.push_back(CMappableObject(body));
  }
  x38_moStart = mObjects.data();
  body.Take(vertexBytes);
  uchar* vertices = x44_buf.get() + objectBytes;
  for (size_t offset = 0; offset < vertexBytes; offset += sizeof(float)) {
    const float value = read_bits< float >(vertices + offset);
    std::memcpy(vertices + offset, &value, sizeof(value));
  }
  x3c_vertexStart = reinterpret_cast< CVector3f* >(vertices);
  const size_t commandStart = objectBytes + vertexBytes + surfaceBytes;
  mSurfaces.reserve(x30_surfaceCount);
  for (int i = 0; i < x30_surfaceCount; ++i) {
    mSurfaces.push_back(CMapAreaSurface(body, *this, commandStart));
  }
  x40_surfaceStart = mSurfaces.data();
}

CMappableObject::CMappableObject(CResourceReader& in)
: x0_type(ReadObjectType(in))
, x4_visibilityMode(ReadObjectVisibility(in))
, x8_objId(in.Read< uint >())
, xc_(in.Read< uint >())
, x10_transform(in.ReadTransform()) {
  const auto padding = in.Take(sizeof(x40_pad));
  std::memcpy(x40_pad, padding.data(), padding.size());
  x10_transform = AdjustTransformForType();
}

CMapArea::CMapAreaSurface::CMapAreaSurface(CResourceReader& in, const CMapArea& area,
                                           size_t commandStart)
: x0_normal(in.ReadVector3f()), xc_centroid(in.ReadVector3f()) {
  const auto data = std::span< const uchar >(area.x44_buf.get(), area.x34_size);
  x18_surfOffset = ReadCommands(data, in.Read< uint >(), area.x2c_vertexCount, commandStart, false);
  x1c_outlineOffset =
      ReadCommands(data, in.Read< uint >(), area.x2c_vertexCount, commandStart, true);
}
