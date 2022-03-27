#include "shader.hpp"

#include "../../gpu.hpp"
#include "../common.hpp"

#include <absl/container/flat_hash_map.h>
#include <aurora/model.hpp>

namespace aurora::gfx::model {
static logvisor::Module Log("aurora::gfx::model");

static const std::vector<zeus::CVector3f>* vtxData;
static const std::vector<zeus::CVector3f>* nrmData;
static const std::vector<Vec2<float>>* tex0TcData;
static const std::vector<Vec2<float>>* tcData;
static std::optional<Range> cachedVtxRange;
static std::optional<Range> cachedNrmRange;
static std::optional<Range> cachedPackedTcRange;
static std::optional<Range> cachedTcRange;

static inline void read_vert(ByteBuffer& out, const u8* data) noexcept {
  size_t offset = 0;
  for (const auto& type : gx::g_gxState.vtxDesc) {
    if (type == GX::INDEX8) {
      const auto v = static_cast<s16>(data[offset]); // expand to s16
      out.append(&v, 2);
      ++offset;
    } else if (type == GX::INDEX16) {
      const s16 v = metaforce::SBig(*reinterpret_cast<const s16*>(data + offset));
      out.append(&v, 2);
      offset += 2;
    }
  }
  constexpr size_t align = 4; // Sint16x2
  if (offset % align != 0) {
    out.append_zeroes(align - (offset % align));
  }
}

static absl::flat_hash_map<XXH64_hash_t, std::pair<ByteBuffer, ByteBuffer>> sCachedDisplayLists;

void queue_surface(const u8* dlStart, u32 dlSize) noexcept {
  const auto hash = xxh3_hash(dlStart, dlSize, 0);
  Range vertRange, idxRange;
  u32 numIndices = 0;
  auto it = sCachedDisplayLists.find(hash);
  if (it != sCachedDisplayLists.end()) {
    const auto& [verts, indices] = it->second;
    numIndices = indices.size() / 2;
    vertRange = push_verts(verts.data(), verts.size());
    idxRange = push_indices(indices.data(), indices.size());
  } else {
    ByteBuffer vtxBuf;
    ByteBuffer idxBuf;
    u8 inVtxSize = 0;
    u8 outVtxSize = 0;
    for (const auto& type : gx::g_gxState.vtxDesc) {
      if (type == GX::NONE || type == GX::DIRECT) {
        continue;
      }
      if (type == GX::INDEX8) {
        ++inVtxSize;
        outVtxSize += 2;
      } else if (type == GX::INDEX16) {
        inVtxSize += 2;
        outVtxSize += 2;
      } else {
        Log.report(logvisor::Fatal, FMT_STRING("unexpected vtx type {}"), type);
        unreachable();
      }
    }
    outVtxSize = ALIGN(outVtxSize, 4);

    u16 vtxStart = 0;
    size_t offset = 0;
    while (offset < dlSize - 6) {
      const auto header = dlStart[offset];
      const auto primitive = static_cast<GX::Primitive>(header & 0xF8);
      const auto dlVtxCount = metaforce::SBig(*reinterpret_cast<const u16*>(dlStart + offset + 1));
      offset += 3;

      if (primitive == 0) {
        break;
      }
      if (primitive != GX::TRIANGLES && primitive != GX::TRIANGLESTRIP && primitive != GX::TRIANGLEFAN) {
        Log.report(logvisor::Fatal, FMT_STRING("queue_surface: unsupported primitive type {}"), primitive);
        unreachable();
      }

      vtxBuf.reserve_extra(dlVtxCount * outVtxSize);
      if (dlVtxCount > 3 && (primitive == GX::TRIANGLEFAN || primitive == GX::TRIANGLESTRIP)) {
        idxBuf.reserve_extra(((u32(dlVtxCount) - 3) * 3 + 3) * 2);
      } else {
        idxBuf.reserve_extra(dlVtxCount * 2);
      }
      u16 curVert = vtxStart;
      for (u16 v = 0; v < dlVtxCount; ++v) {
        read_vert(vtxBuf, dlStart + offset);
        offset += inVtxSize;
        if (primitive == GX::TRIANGLES || v < 3) {
          idxBuf.append(&curVert, 2);
          ++numIndices;
        } else if (primitive == GX::TRIANGLEFAN) {
          const std::array<u16, 3> idxs{
              vtxStart,
              u16(curVert - 1),
              curVert,
          };
          idxBuf.append(idxs.data(), 6);
          numIndices += 3;
        } else if (primitive == GX::TRIANGLESTRIP) {
          if ((v & 1) == 0) {
            const std::array<u16, 3> idxs{
                u16(curVert - 2),
                u16(curVert - 1),
                curVert,
            };
            idxBuf.append(idxs.data(), 6);
          } else {
            const std::array<u16, 3> idxs{
                u16(curVert - 1),
                u16(curVert - 2),
                curVert,
            };
            idxBuf.append(idxs.data(), 6);
          }
          numIndices += 3;
        }
        ++curVert;
      }
      vtxStart += dlVtxCount;
    }

    vertRange = push_verts(vtxBuf.data(), vtxBuf.size());
    idxRange = push_indices(idxBuf.data(), idxBuf.size());
    sCachedDisplayLists.try_emplace(hash, std::move(vtxBuf), std::move(idxBuf));
  }

  Range sVtxRange, sNrmRange, sTcRange, sPackedTcRange;
  if (cachedVtxRange) {
    sVtxRange = *cachedVtxRange;
  } else {
    sVtxRange = push_storage(reinterpret_cast<const uint8_t*>(vtxData->data()), vtxData->size() * 16);
    cachedVtxRange = sVtxRange;
  }
  if (cachedNrmRange) {
    sNrmRange = *cachedNrmRange;
  } else {
    sNrmRange = push_storage(reinterpret_cast<const uint8_t*>(nrmData->data()), nrmData->size() * 16);
    cachedNrmRange = sNrmRange;
  }
  if (cachedTcRange) {
    sTcRange = *cachedTcRange;
  } else {
    sTcRange = push_storage(reinterpret_cast<const uint8_t*>(tcData->data()), tcData->size() * 8);
    cachedTcRange = sTcRange;
  }
  if (cachedPackedTcRange) {
    sPackedTcRange = *cachedPackedTcRange;
  } else if (tcData == tex0TcData) {
    sPackedTcRange = sTcRange;
  } else {
    sPackedTcRange = push_storage(reinterpret_cast<const uint8_t*>(tex0TcData->data()), tex0TcData->size() * 8);
    cachedPackedTcRange = sPackedTcRange;
  }

  model::PipelineConfig config{};
  const gx::BindGroupRanges ranges{
      .vtxDataRange = sVtxRange,
      .nrmDataRange = sNrmRange,
      .tcDataRange = sTcRange,
      .packedTcDataRange = sPackedTcRange,
  };
  const auto info = populate_pipeline_config(config, GX::TRIANGLES, ranges);
  const auto pipeline = pipeline_ref(config);

  push_draw_command(model::DrawData{
      .pipeline = pipeline,
      .vertRange = vertRange,
      .idxRange = idxRange,
      .dataRanges = ranges,
      .uniformRange = build_uniform(info),
      .indexCount = numIndices,
      .bindGroups = info.bindGroups,
      .dstAlpha = gx::g_gxState.dstAlpha,
  });
}

State construct_state() { return {}; }

wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] PipelineConfig config) {
  const auto [shader, info] = build_shader(config.shaderConfig);

  std::array<wgpu::VertexAttribute, gx::MaxVtxAttr> vtxAttrs;
  auto [num4xAttr, rem] = std::div(config.shaderConfig.indexedAttributeCount, 4);
  u32 num2xAttr = 0;
  if (rem > 2) {
    ++num4xAttr;
  } else if (rem > 0) {
    ++num2xAttr;
  }
  u32 offset = 0;
  for (u32 i = 0; i < num4xAttr; ++i) {
    vtxAttrs[i] = {
        .format = wgpu::VertexFormat::Sint16x4,
        .offset = offset,
        .shaderLocation = i,
    };
    offset += 8;
  }
  for (u32 i = 0; i < num2xAttr; ++i) {
    const u32 idx = num4xAttr + i;
    vtxAttrs[idx] = {
        .format = wgpu::VertexFormat::Sint16x2,
        .offset = offset,
        .shaderLocation = idx,
    };
    offset += 4;
  }
  const std::array vtxBuffers{wgpu::VertexBufferLayout{
      .arrayStride = offset,
      .stepMode = wgpu::VertexStepMode::Vertex,
      .attributeCount = num4xAttr + num2xAttr,
      .attributes = vtxAttrs.data(),
  }};

  return build_pipeline(config, info, vtxBuffers, shader, "Model Pipeline");
}

void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass) {
  if (!bind_pipeline(data.pipeline, pass)) {
    return;
  }

  const std::array offsets{
      data.uniformRange.offset,
      storage_offset(data.dataRanges.vtxDataRange),
      storage_offset(data.dataRanges.nrmDataRange),
      storage_offset(data.dataRanges.packedTcDataRange),
      storage_offset(data.dataRanges.tcDataRange),
  };
  pass.SetBindGroup(0, find_bind_group(data.bindGroups.uniformBindGroup), offsets.size(), offsets.data());
  if (data.bindGroups.samplerBindGroup && data.bindGroups.textureBindGroup) {
    pass.SetBindGroup(1, find_bind_group(data.bindGroups.samplerBindGroup));
    pass.SetBindGroup(2, find_bind_group(data.bindGroups.textureBindGroup));
  }
  pass.SetVertexBuffer(0, g_vertexBuffer, data.vertRange.offset, data.vertRange.size);
  pass.SetIndexBuffer(g_indexBuffer, wgpu::IndexFormat::Uint16, data.idxRange.offset, data.idxRange.size);
  if (data.dstAlpha) {
    const wgpu::Color color{0.f, 0.f, 0.f, *data.dstAlpha};
    pass.SetBlendConstant(&color);
  }
  pass.DrawIndexed(data.indexCount);
}
} // namespace aurora::gfx::model

static absl::flat_hash_map<XXH64_hash_t, aurora::gfx::Range> sCachedRanges;
template <typename Vec>
static inline void cache_array(const void* data, Vec*& outPtr, std::optional<aurora::gfx::Range>& outRange, u8 stride) {
  Vec* vecPtr = static_cast<Vec*>(data);
  outPtr = vecPtr;
  outRange.reset();
}

void GXSetArray(GX::Attr attr, const void* data, u8 stride) noexcept {
  using namespace aurora::gfx::model;
  switch (attr) {
  case GX::VA_POS:
    cache_array(data, vtxData, cachedVtxRange, stride);
    break;
  case GX::VA_NRM:
    cache_array(data, nrmData, cachedNrmRange, stride);
    break;
  case GX::VA_TEX0:
    cache_array(data, tex0TcData, cachedPackedTcRange, stride);
    break;
  case GX::VA_TEX1:
    cache_array(data, tcData, cachedTcRange, stride);
    break;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("GXSetArray: invalid attr {}"), attr);
    unreachable();
  }
}

void GXCallDisplayList(const void* data, u32 nbytes) noexcept {
  aurora::gfx::model::queue_surface(static_cast<const u8*>(data), nbytes);
}
