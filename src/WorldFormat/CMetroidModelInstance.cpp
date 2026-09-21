#include "WorldFormat/CMetroidModelInstance.hpp"

#include "Kyoto/Basics/CBasics.hpp"
#include <string.h>

#if defined(TARGET_PC)
#include "Metaforce/Endian.hpp"
#include <borealis/log.hpp>
#endif

#if TARGET_BIG_ENDIAN
static const CTransform4f& TransformFromData(const void* ptr) {
  return *static_cast< const CTransform4f* >(ptr);
}
#else
static CTransform4f TransformFromData(const void* ptr) {
  float values[12];
  memcpy(values, ptr, sizeof(values));
  for (int i = 0; i < 12; ++i) {
    values[i] = CBasics::SwapBytes(values[i]);
  }
  return CTransform4f(values[0], values[1], values[2], values[3], values[4], values[5], values[6],
                      values[7], values[8], values[9], values[10], values[11]);
}
#endif

static CAABox BoundingBoxFromData(const void* ptr) {
  float out[6];
  const float* tmp = reinterpret_cast< const float* >(ptr);
  for (int i = 0; i < 6; ++i) {
    out[i] = CBasics::SwapBytes(tmp[i]);
  }

#ifdef __MWERKS__
  return *reinterpret_cast< const CAABox* >(out);
#else
  return CAABox(CVector3f(out[0], out[1], out[2]), CVector3f(out[3], out[4], out[5]));
#endif
}

#if defined(TARGET_PC)
namespace {
constexpr borealis::Log Log{"CMetroidModelInstance"};

const uchar* CheckedModelHeader(TModelData header) {
  if (header.size() < 76) {
    Log.fatal("Truncated MREA model header");
  }
  return header.data();
}
} // namespace

CMetroidModelInstance::CMetroidModelInstance(TModelData header, TModelData materialData,
                                             const SModelArrays& arrays,
                                             const std::vector< TModelData >& surfaces)
: x0_visorFlags(read_bits< uint >(CheckedModelHeader(header)))
, x4_worldXf(TransformFromData(header.data() + 4))
, x34_worldAABB(ReadModelBounds(header.subspan(52, 24)))
, x4c_materialData(materialData)
, x50_surfaces(surfaces)
, mArrays(arrays) {}
#else
CMetroidModelInstance::CMetroidModelInstance(const void* header, const void* firstGeom,
                                             const void* positions, const void* normals,
                                             const void* colors, const void* texCoords,
                                             const void* packedTexCoords,
                                             const rstl::vector< void* >& surfaces)
: x0_visorFlags(CBasics::SwapBytes(*reinterpret_cast< const uint* >(header)))
, x4_worldXf(TransformFromData((uchar*)header + sizeof(uint)))
, x34_worldAABB(BoundingBoxFromData((uchar*)header + sizeof(CTransform4f) + sizeof(uint)))
, x4c_materialData(firstGeom)
, x50_surfaces(surfaces)
, x60_positions(positions)
, x64_normals(normals)
, x68_colors(colors)
, x6c_texCoords(texCoords)
, x70_packedTexCoords(packedTexCoords) {}
#endif
