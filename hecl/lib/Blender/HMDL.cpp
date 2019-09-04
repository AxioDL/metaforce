#include "hecl/Blender/Connection.hpp"

#include <cfloat>
#include <cmath>
#include <cstddef>
#include <vector>

#include <athena/MemoryWriter.hpp>

#undef min
#undef max

namespace hecl::blender {

atVec3f MtxVecMul4RM(const Matrix4f& mtx, const Vector3f& vec) {
  atVec3f res;
  athena::simd_floats resf;
  athena::simd_floats mtxf[3];
  for (int i = 0; i < 3; ++i)
    mtx[i].simd.copy_to(mtxf[i]);
  athena::simd_floats vecf(vec.val.simd);
  resf[0] = mtxf[0][0] * vecf[0] + mtxf[0][1] * vecf[1] + mtxf[0][2] * vecf[2] + mtxf[0][3];
  resf[1] = mtxf[1][0] * vecf[0] + mtxf[1][1] * vecf[1] + mtxf[1][2] * vecf[2] + mtxf[1][3];
  resf[2] = mtxf[2][0] * vecf[0] + mtxf[2][1] * vecf[1] + mtxf[2][2] * vecf[2] + mtxf[2][3];
  res.simd.copy_from(resf);
  return res;
}

atVec3f MtxVecMul3RM(const Matrix4f& mtx, const Vector3f& vec) {
  atVec3f res;
  athena::simd_floats resf;
  athena::simd_floats mtxf[3];
  for (int i = 0; i < 3; ++i)
    mtx[i].simd.copy_to(mtxf[i]);
  athena::simd_floats vecf(vec.val.simd);
  resf[0] = mtxf[0][0] * vecf[0] + mtxf[0][1] * vecf[1] + mtxf[0][2] * vecf[2];
  resf[1] = mtxf[1][0] * vecf[0] + mtxf[1][1] * vecf[1] + mtxf[1][2] * vecf[2];
  resf[2] = mtxf[2][0] * vecf[0] + mtxf[2][1] * vecf[1] + mtxf[2][2] * vecf[2];
  res.simd.copy_from(resf);
  return res;
}

HMDLBuffers Mesh::getHMDLBuffers(bool absoluteCoords, PoolSkinIndex& poolSkinIndex) const {
  /* If skinned, compute max weight vec count */
  size_t weightCount = 0;
  for (const SkinBanks::Bank& bank : skinBanks.banks)
    weightCount = std::max(weightCount, bank.m_boneIdxs.size());
  size_t weightVecCount = weightCount / 4;
  if (weightCount % 4)
    ++weightVecCount;

  /* Prepare HMDL meta */
  HMDLMeta metaOut;
  metaOut.topology = topology;
  metaOut.vertStride = (3 + 3 + colorLayerCount + uvLayerCount * 2 + weightVecCount * 4) * 4;
  metaOut.colorCount = colorLayerCount;
  metaOut.uvCount = uvLayerCount;
  metaOut.weightCount = weightVecCount;
  metaOut.bankCount = skinBanks.banks.size();

  /* Total all verts from all surfaces (for ibo length) */
  size_t boundVerts = 0;
  for (const Surface& surf : surfaces)
    boundVerts += surf.verts.size();

  /* Maintain unique vert pool for VBO */
  std::vector<std::pair<const Surface*, const Surface::Vert*>> vertPool;
  vertPool.reserve(boundVerts);

  /* Target surfaces representation */
  std::vector<HMDLBuffers::Surface> outSurfaces;
  outSurfaces.reserve(surfaces.size());

  /* Index buffer */
  std::vector<atUint32> iboData;
  iboData.reserve(boundVerts);

  for (const Surface& surf : surfaces) {
    size_t iboStart = iboData.size();
    for (const Surface::Vert& v : surf.verts) {
      if (v.iPos == 0xffffffff) {
        iboData.push_back(0xffffffff);
        continue;
      }

      size_t ti = 0;
      bool found = false;
      for (const std::pair<const Surface*, const Surface::Vert*>& tv : vertPool) {
        if (v == *tv.second && surf.skinBankIdx == tv.first->skinBankIdx) {
          iboData.push_back(ti);
          found = true;
          break;
        }
        ++ti;
      }
      if (!found) {
        iboData.push_back(vertPool.size());
        vertPool.emplace_back(&surf, &v);
      }
    }
    outSurfaces.emplace_back(surf, iboStart, iboData.size() - iboStart);
  }

  metaOut.vertCount = vertPool.size();
  metaOut.indexCount = iboData.size();

  size_t vboSz = metaOut.vertCount * metaOut.vertStride;
  poolSkinIndex.allocate(vertPool.size());
  HMDLBuffers ret(std::move(metaOut), vboSz, iboData, std::move(outSurfaces), skinBanks);
  athena::io::MemoryWriter vboW(ret.m_vboData.get(), vboSz);
  uint32_t curPoolIdx = 0;
  for (const std::pair<const Surface*, const Surface::Vert*>& sv : vertPool) {
    const Surface& s = *sv.first;
    const Surface::Vert& v = *sv.second;

    if (absoluteCoords) {
      atVec3f preXfPos = MtxVecMul4RM(sceneXf, pos[v.iPos]);
      vboW.writeVec3fLittle(preXfPos);

      atVec3f preXfNorm = MtxVecMul3RM(sceneXf, norm[v.iNorm]);
      athena::simd_floats f(preXfNorm.simd * preXfNorm.simd);
      float mag = f[0] + f[1] + f[2];
      if (mag > FLT_EPSILON)
        mag = 1.f / std::sqrt(mag);
      preXfNorm.simd *= mag;
      vboW.writeVec3fLittle(preXfNorm);
    } else {
      vboW.writeVec3fLittle(pos[v.iPos]);
      vboW.writeVec3fLittle(norm[v.iNorm]);
    }

    for (size_t i = 0; i < colorLayerCount; ++i) {
      const Vector3f& c = color[v.iColor[i]];
      athena::simd_floats f(c.val.simd);
      vboW.writeUByte(std::max(0, std::min(255, int(f[0] * 255))));
      vboW.writeUByte(std::max(0, std::min(255, int(f[1] * 255))));
      vboW.writeUByte(std::max(0, std::min(255, int(f[2] * 255))));
      vboW.writeUByte(255);
    }

    for (size_t i = 0; i < uvLayerCount; ++i)
      vboW.writeVec2fLittle(uv[v.iUv[i]]);

    if (weightVecCount) {
      const SkinBanks::Bank& bank = skinBanks.banks[s.skinBankIdx];
      const auto& binds = skins[v.iSkin];

      auto it = bank.m_boneIdxs.cbegin();
      for (size_t i = 0; i < weightVecCount; ++i) {
        atVec4f vec = {};
        for (size_t j = 0; j < 4; ++j) {
          if (it == bank.m_boneIdxs.cend())
            break;
          for (const SkinBind& bind : binds) {
            if (!bind.valid())
              break;
            if (bind.vg_idx == *it) {
              vec.simd[j] = bind.weight;
              break;
            }
          }
          ++it;
        }
        vboW.writeVec4fLittle(vec);
      }
    }

    /* mapping pool verts to skin indices */
    poolSkinIndex.m_poolToSkinIndex[curPoolIdx] = sv.second->iSkin;
    ++curPoolIdx;
  }

  return ret;
}

} // namespace hecl::blender
