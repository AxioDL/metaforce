#include "CMetroidModelInstance.hpp"

#include "Graphics/CCubeSurface.hpp"
#include "Streams/IOStreams.hpp"

namespace metaforce {
CMetroidModelInstance::CMetroidModelInstance(std::span<const u8> modelHeader, const u8* materialData,
                                             std::span<const u8> positions, std::span<const u8> normals,
                                             std::span<const u8> colors, std::span<const u8> texCoords,
                                             std::span<const u8> packedTexCoords, std::vector<CCubeSurface>&& surfaces)
: x4c_materialData(materialData), x50_surfaces(std::move(surfaces)) {
  {
    CMemoryInStream stream{modelHeader.data(), static_cast<u32>(modelHeader.size_bytes())};
    x0_visorFlags = stream.ReadUint32();
    x4_worldXf = stream.Get<zeus::CTransform>();
    x34_worldAABB = stream.Get<zeus::CAABox>();
  }
  {
    u32 numVertices = positions.size_bytes() / 12;
    x60_positions.reserve(numVertices);
    CMemoryInStream stream{positions.data(), static_cast<u32>(positions.size_bytes())};
    for (u32 i = 0; i < numVertices; ++i) {
      x60_positions.push_back(stream.Get<aurora::Vec3<float>>());
    }
  }
  {
    // Always short normals in MREA
    u32 numNormals = normals.size_bytes() / 6;
    x64_normals.reserve(numNormals);
    CMemoryInStream stream{normals.data(), static_cast<u32>(normals.size_bytes())};
    for (u32 i = 0; i < numNormals; ++i) {
      x64_normals.push_back(stream.Get<aurora::Vec3<s16>>());
    }
  }
  {
    u32 numColors = colors.size_bytes() / 4;
    x68_colors.reserve(numColors);
    CMemoryInStream stream{colors.data(), static_cast<u32>(colors.size_bytes())};
    for (u32 i = 0; i < numColors; ++i) {
      x68_colors.push_back(stream.ReadUint32());
    }
  }
  {
    u32 numTexCoords = texCoords.size_bytes() / 8;
    x6c_texCoords.reserve(numTexCoords);
    CMemoryInStream stream{texCoords.data(), static_cast<u32>(texCoords.size_bytes())};
    for (u32 i = 0; i < numTexCoords; ++i) {
      x6c_texCoords.push_back(stream.Get<aurora::Vec2<float>>());
    }
  }
  {
    u32 numPackedTexCoords = packedTexCoords.size_bytes() / 4;
    x70_packedTexCoords.reserve(numPackedTexCoords);
    CMemoryInStream stream{packedTexCoords.data(), static_cast<u32>(packedTexCoords.size_bytes())};
    for (u32 i = 0; i < numPackedTexCoords; ++i) {
      x70_packedTexCoords.push_back(stream.Get<aurora::Vec2<u16>>());
    }
  }
}
} // namespace metaforce
