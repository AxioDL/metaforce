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
  mMagic = reader.Read< uint >();
  mVersion = reader.Read< uint >();
  REQUIRE(mMagic == 0xdeadd00d, "Invalid MAPA magic {:#x}", mMagic);
  REQUIRE(mVersion == 2, "Unsupported MAPA version {}", mVersion);
  x8_ = reader.Read< uint >();
  const uint visibility = reader.Read< uint >();
  REQUIRE(visibility <= kVM_Never, "Invalid MAPA visibility {}", visibility);
  mVisibilityMode = static_cast< EVisMode >(visibility);
  mBox = reader.ReadAABox();
  mMappableObjCount = reader.Read< int >();
  mVertexCount = reader.Read< int >();
  mSurfaceCount = reader.Read< int >();
  mSize = size - 52;
  REQUIRE(mMappableObjCount >= 0 && mVertexCount >= 0 && mSurfaceCount >= 0,
          "Negative MAPA record count");
  REQUIRE(static_cast< uint >(mMappableObjCount) <= mSize / 80 &&
              static_cast< uint >(mVertexCount) <= mSize / 12 &&
              static_cast< uint >(mSurfaceCount) <= mSize / 32,
          "MAPA record count exceeds resource");
  const size_t objectBytes = static_cast< size_t >(mMappableObjCount) * 80;
  const size_t vertexBytes = static_cast< size_t >(mVertexCount) * 12;
  const size_t surfaceBytes = static_cast< size_t >(mSurfaceCount) * 32;
  REQUIRE(objectBytes <= mSize && vertexBytes <= mSize - objectBytes &&
              surfaceBytes <= mSize - objectBytes - vertexBytes,
          "MAPA records exceed resource");
  mBuf = rs_new uchar[mSize];
  REQUIRE(in.ReadBytes(mBuf.get(), mSize) == mSize, "Truncated MAPA body");

  CResourceReader body({mBuf.get(), mSize}, Log.name);
  mObjects.reserve(mMappableObjCount);
  for (int i = 0; i < mMappableObjCount; ++i) {
    mObjects.push_back(CMappableObject(body));
  }
  mMoStart = mObjects.data();
  body.Take(vertexBytes);
  uchar* vertices = mBuf.get() + objectBytes;
  for (size_t offset = 0; offset < vertexBytes; offset += sizeof(float)) {
    const float value = read_bits< float >(vertices + offset);
    std::memcpy(vertices + offset, &value, sizeof(value));
  }
  mVertexStart = reinterpret_cast< CVector3f* >(vertices);
  const size_t commandStart = objectBytes + vertexBytes + surfaceBytes;
  mSurfaces.reserve(mSurfaceCount);
  for (int i = 0; i < mSurfaceCount; ++i) {
    mSurfaces.push_back(CMapAreaSurface(body, *this, commandStart));
  }
  mSurfaceStart = mSurfaces.data();
}

CMappableObject::CMappableObject(CResourceReader& in)
: mType(ReadObjectType(in))
, mVisibilityMode(ReadObjectVisibility(in))
, mObjId(in.Read< uint >())
, xc_(in.Read< uint >())
, mTransform(in.ReadTransform()) {
  const auto padding = in.Take(sizeof(mPad));
  std::memcpy(mPad, padding.data(), padding.size());
  mTransform = AdjustTransformForType();
}

CMapArea::CMapAreaSurface::CMapAreaSurface(CResourceReader& in, const CMapArea& area,
                                           size_t commandStart)
: mNormal(in.ReadVector3f()), mCentroid(in.ReadVector3f()) {
  const auto data = std::span< const uchar >(area.mBuf.get(), area.mSize);
  mSurfOffset = ReadCommands(data, in.Read< uint >(), area.mVertexCount, commandStart, false);
  mOutlineOffset =
      ReadCommands(data, in.Read< uint >(), area.mVertexCount, commandStart, true);
}
