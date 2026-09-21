#include "Metaforce/CModelSectionReader.hpp"

#include "Kyoto/Graphics/CCubeSurface.hpp"
#include "Metaforce/Common.hpp"
#include "Metaforce/Endian.hpp"

#include <limits>

namespace {
constexpr borealis::Log Log{"CModelSectionReader"};

template < typename T >
void NormalizeArray(std::span< uchar > data) {
  if constexpr (std::endian::native != std::endian::big) {
    for (size_t offset = 0; offset < data.size(); offset += sizeof(T)) {
      const T value = read_bits< T >(data.data() + offset);
      std::memcpy(data.data() + offset, &value, sizeof(value));
    }
  }
}

void ValidateArray(TModelData data, size_t componentSize) {
  REQUIRE(data.size() % componentSize == 0,
          "Incomplete model array component: {} bytes with {}-byte components", data.size(),
          componentSize);
  REQUIRE(data.empty() || reinterpret_cast< uintptr_t >(data.data()) % componentSize == 0,
          "Misaligned model array");
  REQUIRE(data.size() <= std::numeric_limits< uint32_t >::max(), "Model array is too large");
}

size_t MaterialTableOffset(TModelData data) {
  REQUIRE(data.size() >= 8, "Truncated model material set");
  const uint textureCount = read_bits< uint >(data.data());
  REQUIRE(textureCount <= (data.size() - 8) / 4, "Truncated model texture table");
  return (static_cast< size_t >(textureCount) + 1) * 4;
}
} // namespace

SModelArrays PrepareModelArrays(std::span< uchar > positions, std::span< uchar > normals,
                                std::span< uchar > colors, std::span< uchar > texCoords,
                                std::span< uchar > packedTexCoords, bool shortNormals) {
  ValidateArray(positions, 4);
  ValidateArray(normals, shortNormals ? 2 : 4);
  ValidateArray(colors, 4);
  ValidateArray(texCoords, 4);
  ValidateArray(packedTexCoords, 2);
  NormalizeArray< float >(positions);
  if (shortNormals) {
    NormalizeArray< short >(normals);
  } else {
    NormalizeArray< float >(normals);
  }
  NormalizeArray< float >(texCoords);
  NormalizeArray< ushort >(packedTexCoords);
  return {
      .positions = positions,
      .normals = normals,
      .colors = colors,
      .texCoords = texCoords,
      .packedTexCoords = packedTexCoords,
  };
}

CModelSectionReader::CModelSectionReader(std::span< uchar > data) : mData(data) {
  REQUIRE(data.size() >= 0x28, "Truncated CMDL header");
  REQUIRE(read_bits< uint >(data.data()) == 0xdeadbabe, "Invalid CMDL magic");
  const uint version = read_bits< uint >(data.data() + 4);
  REQUIRE(version == 1 || version == 2, "Unsupported CMDL version {}", version);
  mSizeTable = version == 1 ? 0x28 : 0x2c;
  REQUIRE(data.size() >= mSizeTable, "Truncated CMDL header");
  mFlags = read_bits< uint >(data.data() + 8);
  mMaterialSetCount = version == 1 ? 1 : read_bits< uint >(data.data() + 0x28);
  mSectionCount = read_bits< uint >(data.data() + 0x24);
  REQUIRE(mSectionCount <= (data.size() - mSizeTable) / 4, "Truncated CMDL section table");
  const size_t geometrySections = (mFlags & 4) ? 6 : 5;
  REQUIRE(mMaterialSetCount != 0 && mMaterialSetCount <= mSectionCount &&
              geometrySections <= mSectionCount - mMaterialSetCount,
          "Invalid CMDL material set count");
  mOffset = (mSizeTable + mSectionCount * 4 + 31) & ~size_t{31};
  REQUIRE(mOffset <= data.size(), "Truncated CMDL section alignment");
  size_t end = mOffset;
  for (size_t i = 0; i < mSectionCount; ++i) {
    const size_t size = read_bits< uint >(data.data() + mSizeTable + i * 4);
    REQUIRE(size <= data.size() - end, "CMDL section exceeds resource");
    end += size;
  }
}

std::span< uchar > CModelSectionReader::Next() {
  REQUIRE(mSection < mSectionCount, "Missing CMDL section");
  const size_t size = read_bits< uint >(mData.data() + mSizeTable + mSection++ * 4);
  auto result = size ? mData.subspan(mOffset, size) : std::span< uchar >{};
  mOffset += size;
  return result;
}

CAABox CModelSectionReader::GetBounds() const { return ReadModelBounds(mData.subspan(12, 24)); }

CAABox ReadModelBounds(TModelData data) {
  REQUIRE(data.size() >= 24, "Truncated model bounds");
  return CAABox{
      read_bits< float >(data.data()),      read_bits< float >(data.data() + 4),
      read_bits< float >(data.data() + 8),  read_bits< float >(data.data() + 12),
      read_bits< float >(data.data() + 16), read_bits< float >(data.data() + 20),
  };
}

uint ReadModelSurfaceCount(TModelData data, size_t remainingSections) {
  REQUIRE(data.size() >= 4, "Truncated model surface table");
  const uint count = read_bits< uint >(data.data());
  REQUIRE(count <= remainingSections, "Invalid model surface count");
  REQUIRE(count <= (data.size() - 4) / 4, "Truncated model surface offsets");
  return count;
}

uint ValidateModelMaterials(TModelData data) {
  REQUIRE(reinterpret_cast< uintptr_t >(data.data()) % 4 == 0, "Misaligned model materials");
  const size_t table = MaterialTableOffset(data);
  const uint textureCount = read_bits< uint >(data.data());
  const uint count = read_bits< uint >(data.data() + table);
  REQUIRE(count <= (data.size() - table - 4) / 4, "Truncated model material offsets");
  const size_t start = table + (static_cast< size_t >(count) + 1) * 4;
  size_t previous = 0;
  for (uint i = 0; i < count; ++i) {
    const size_t end = read_bits< uint >(data.data() + table + (static_cast< size_t >(i) + 1) * 4);
    REQUIRE(end >= previous && end <= data.size() - start && previous % 4 == 0,
            "Invalid model material extent");
    const auto material = data.subspan(start + previous, end - previous);
    REQUIRE(material.size() >= 16, "Truncated model material header");
    const uint references = read_bits< uint >(material.data() + 4);
    REQUIRE(references <= (material.size() - 16) / 4, "Truncated material texture references");
    for (uint j = 0; j < references; ++j) {
      REQUIRE(read_bits< uint >(material.data() + 8 + static_cast< size_t >(j) * 4) < textureCount,
              "Invalid material texture index");
    }
    previous = end;
  }
  return count;
}

TModelData GetModelMaterial(TModelData data, uint index) {
  const size_t table = MaterialTableOffset(data);
  const uint count = read_bits< uint >(data.data() + table);
  REQUIRE(count <= (data.size() - table - 4) / 4 && index < count,
          "Invalid surface material index");
  const size_t start = table + (static_cast< size_t >(count) + 1) * 4;
  const size_t offset =
      index ? read_bits< uint >(data.data() + table + static_cast< size_t >(index) * 4) : 0;
  const size_t end =
      read_bits< uint >(data.data() + table + (static_cast< size_t >(index) + 1) * 4);
  REQUIRE(end >= offset && end <= data.size() - start, "Invalid model material extent");
  return data.subspan(start + offset, end - offset);
}

CCubeSurface::SSurfaceData CCubeSurface::ReadData(TModelData data, CCubeModel* parent,
                                                  uint materialCount) {
  REQUIRE(data.size() >= 44, "Truncated model surface header");
  const auto* bytes = data.data();
  const uint material = read_bits< uint >(bytes + 12);
  const uint dlSize = read_bits< uint >(bytes + 16);
  const uint extraSize = read_bits< uint >(bytes + 28);
  REQUIRE(material < materialCount, "Invalid surface material index");
  REQUIRE(extraSize == 0 || extraSize >= 24, "Truncated surface bounds");
  const uint64_t headerSize = (uint64_t{44} + extraSize + 31) & ~uint64_t{31};
  REQUIRE(headerSize <= data.size() && (dlSize & 0x7fffffff) <= data.size() - headerSize,
          "Surface display list exceeds section");
  const CVector3f center(read_bits< float >(bytes), read_bits< float >(bytes + 4),
                         read_bits< float >(bytes + 8));
  return {
      .mCenter = center,
      .mMaterialIndex = material,
      .mDisplayListSizeAndNormalHint = dlSize,
      .mParent = parent,
      .mNextSurface = nullptr,
      .mExtraSize = extraSize,
      .mNormal = CUnitVector3f(read_bits< float >(bytes + 32), read_bits< float >(bytes + 36),
                               read_bits< float >(bytes + 40)),
      .mBounds = extraSize ? ReadModelBounds(data.subspan(44, 24))
                           : CAABox(center.GetX(), center.GetY(), center.GetZ(), center.GetX(),
                                    center.GetY(), center.GetZ()),
      .displayList = bytes + headerSize,
      .headerSize = static_cast< uint >(headerSize),
  };
}
