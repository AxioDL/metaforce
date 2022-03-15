#include "shader.hpp"

#include "../../gpu.hpp"
#include "../common.hpp"

#include <absl/container/flat_hash_map.h>
#include <aurora/model.hpp>
#include <magic_enum.hpp>

enum class VtxDescAttr : u8 {
  Position = 0,
  Normal = 2,
  Color0 = 4,
  Color1 = 6,
  Tex0 = 8,
  Tex1 = 10,
  Tex2 = 12,
  Tex3 = 14,
  Tex4 = 16,
  Tex5 = 18,
  Tex6 = 20,
  PnMatIdx = 24,
  Tex0MatIdx = 25,
  Tex1MatIdx = 26,
  Tex2MatIdx = 27,
  Tex3MatIdx = 28,
  Tex4MatIdx = 29,
  Tex5MatIdx = 30,
  Tex6MatIdx = 31,
};
enum class VtxDescAttrType : u8 { None = 0, Direct = 1, Index8 = 2, Index16 = 3 };
class VtxDescFlags {
  u32 m_flags = 0;

public:
  constexpr VtxDescFlags() noexcept = default;
  constexpr VtxDescFlags(u32 flags) noexcept : m_flags(flags){};
  [[nodiscard]] constexpr VtxDescAttrType GetAttributeType(VtxDescAttr attribute) const noexcept {
    return VtxDescAttrType((m_flags >> u32(attribute)) & 0x3);
  }
  [[nodiscard]] constexpr VtxDescAttrType GetDirectAttributeType(VtxDescAttr attribute) const noexcept {
    return VtxDescAttrType((m_flags >> u32(attribute)) & 0x1);
  }
  constexpr void SetAttributeType(VtxDescAttr attribute, VtxDescAttrType type) noexcept {
    m_flags &= ~(u32(0x3) << u32(attribute));
    m_flags |= u32(type) << u32(attribute);
  }
};

namespace aurora::gfx::model {
static logvisor::Module Log("aurora::gfx::model");

static const std::vector<zeus::CVector3f>* vtxData;
static const std::vector<zeus::CVector3f>* nrmData;
static const std::vector<Vec2<float>>* tex0TcData;
static const std::vector<Vec2<float>>* tcData;
static std::optional<Range> staticVtxRange;
static std::optional<Range> staticNrmRange;
static std::optional<Range> staticPackedTcRange;
static std::optional<Range> staticTcRange;

enum class VertexFormat : u8 {
  F32F32,
  S16F32,
  S16S16,
};
static VtxDescFlags sVtxDescFlags;
void set_vtx_desc_compressed(u32 vtxDesc) noexcept { sVtxDescFlags = vtxDesc; }

static inline std::pair<gx::DlVert, size_t> readVert(const u8* data) noexcept {
  gx::DlVert out{};
  size_t offset = 0;
  const auto read8 = [data, &offset](VtxDescAttrType type) -> s8 {
    if (type == VtxDescAttrType::Direct) {
      s8 v = static_cast<s8>(data[offset]);
      ++offset;
      return v;
    }
    return 0;
  };
  const auto read16 = [data, &offset](VtxDescAttrType type) -> s16 {
    if (type == VtxDescAttrType::Index16) {
      s16 v = metaforce::SBig(*reinterpret_cast<const u16*>(data + offset));
      offset += 2;
      return v;
    }
    return 0;
  };
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::PnMatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex0MatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex1MatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex2MatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex3MatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex4MatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex5MatIdx));
  read8(sVtxDescFlags.GetDirectAttributeType(VtxDescAttr::Tex6MatIdx));

  out.pos = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Position));
  out.norm = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Normal));
  read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Color0));
  read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Color1));
  out.uvs[0] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex0));
  out.uvs[1] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex1));
  out.uvs[2] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex2));
  out.uvs[3] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex3));
  out.uvs[4] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex4));
  out.uvs[5] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex5));
  out.uvs[6] = read16(sVtxDescFlags.GetAttributeType(VtxDescAttr::Tex6));

  return {out, offset};
}

static absl::flat_hash_map<XXH64_hash_t, std::pair<std::vector<gx::DlVert>, std::vector<u32>>> sCachedDisplayLists;

void queue_surface(const u8* dlStart, u32 dlSize) noexcept {
  const auto hash = xxh3_hash(dlStart, dlSize, 0);
  Range vertRange, idxRange;
  uint32_t numIndices;
  auto it = sCachedDisplayLists.find(hash);
  if (it != sCachedDisplayLists.end()) {
    const auto& [verts, indices] = it->second;
    numIndices = indices.size();
    vertRange = push_verts(ArrayRef{verts});
    idxRange = push_indices(ArrayRef{indices});
  } else {
    std::vector<gx::DlVert> verts;
    std::vector<u32> indices;

    size_t offset = 0;
    while (offset < dlSize - 6) {
      const auto header = dlStart[offset];
      const auto primitive = static_cast<GX::Primitive>(header & 0xF8);
      const auto vtxFmt = static_cast<VertexFormat>(header & 0x3);
      const auto vtxCount = metaforce::SBig(*reinterpret_cast<const u16*>(dlStart + offset + 1));
      offset += 3;

      if (primitive == 0) {
        break;
      }
      if (primitive != GX::TRIANGLES && primitive != GX::TRIANGLESTRIP && primitive != GX::TRIANGLEFAN) {
        Log.report(logvisor::Fatal, FMT_STRING("queue_surface: unsupported primitive type {}"), primitive);
        unreachable();
      }

      const u32 idxStart = indices.size();
      const u16 vertsStart = verts.size();
      verts.reserve(vertsStart + vtxCount);
      if (vtxCount > 3 && (primitive == GX::TRIANGLEFAN || primitive == GX::TRIANGLESTRIP)) {
        indices.reserve(idxStart + (u32(vtxCount) - 3) * 3 + 3);
      } else {
        indices.reserve(idxStart + vtxCount);
      }
      auto curVert = vertsStart;
      for (int v = 0; v < vtxCount; ++v) {
        const auto [vert, read] = readVert(dlStart + offset);
        verts.push_back(vert);
        offset += read;
        if (primitive == GX::TRIANGLES || v < 3) {
          // pass
        } else if (primitive == GX::TRIANGLEFAN) {
          indices.push_back(vertsStart);
          indices.push_back(curVert - 1);
        } else if (primitive == GX::TRIANGLESTRIP) {
          if ((v & 1) == 0) {
            indices.push_back(curVert - 2);
            indices.push_back(curVert - 1);
          } else {
            indices.push_back(curVert - 1);
            indices.push_back(curVert - 2);
          }
        }
        indices.push_back(curVert);
        ++curVert;
      }
    }

    numIndices = indices.size();
    vertRange = push_verts(ArrayRef{verts});
    idxRange = push_indices(ArrayRef{indices});
    sCachedDisplayLists.try_emplace(hash, std::move(verts), std::move(indices));
  }

  Range sVtxRange, sNrmRange, sTcRange, sPackedTcRange;
  if (staticVtxRange) {
    sVtxRange = *staticVtxRange;
  } else {
    sVtxRange = push_storage(reinterpret_cast<const uint8_t*>(vtxData->data()), vtxData->size() * 16);
  }
  if (staticNrmRange) {
    sNrmRange = *staticNrmRange;
  } else {
    sNrmRange = push_storage(reinterpret_cast<const uint8_t*>(nrmData->data()), nrmData->size() * 16);
  }
  if (staticTcRange) {
    sTcRange = *staticTcRange;
  } else {
    sTcRange = push_storage(reinterpret_cast<const uint8_t*>(tcData->data()), tcData->size() * 8);
  }
  if (staticPackedTcRange) {
    sPackedTcRange = *staticPackedTcRange;
  } else if (tcData == tex0TcData) {
    sPackedTcRange = sTcRange;
  } else {
    sPackedTcRange = push_storage(reinterpret_cast<const uint8_t*>(tex0TcData->data()), tex0TcData->size() * 8);
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
  });
}

State construct_state() { return {}; }

wgpu::RenderPipeline create_pipeline(const State& state, [[maybe_unused]] PipelineConfig config) {
  const auto [shader, info] = build_shader(config.shaderConfig);

  const auto attributes = gpu::utils::make_vertex_attributes(
      std::array{wgpu::VertexFormat::Sint16x2, wgpu::VertexFormat::Sint16x4, wgpu::VertexFormat::Sint16x4});
  const std::array vertexBuffers{gpu::utils::make_vertex_buffer_layout(sizeof(gx::DlVert), attributes)};

  return build_pipeline(config, info, vertexBuffers, shader, "Model Pipeline");
}

void render(const State& state, const DrawData& data, const wgpu::RenderPassEncoder& pass) {
  if (!bind_pipeline(data.pipeline, pass)) {
    return;
  }

  const std::array offsets{
      data.uniformRange.offset,
      storage_offset(data.dataRanges.vtxDataRange),
      storage_offset(data.dataRanges.nrmDataRange),
      storage_offset(data.dataRanges.tcDataRange),
      storage_offset(data.dataRanges.packedTcDataRange),
  };
  pass.SetBindGroup(0, find_bind_group(data.bindGroups.uniformBindGroup), offsets.size(), offsets.data());
  if (data.bindGroups.samplerBindGroup && data.bindGroups.textureBindGroup) {
    pass.SetBindGroup(1, find_bind_group(data.bindGroups.samplerBindGroup));
    pass.SetBindGroup(2, find_bind_group(data.bindGroups.textureBindGroup));
  }
  pass.SetVertexBuffer(0, g_vertexBuffer, data.vertRange.offset, data.vertRange.size);
  pass.SetIndexBuffer(g_indexBuffer, wgpu::IndexFormat::Uint32, data.idxRange.offset, data.idxRange.size);
  pass.DrawIndexed(data.indexCount);
}
} // namespace aurora::gfx::model

static absl::flat_hash_map<XXH64_hash_t, aurora::gfx::Range> sCachedRanges;
template <typename Vec>
static inline void cache_array(const void* data, Vec*& outPtr, std::optional<aurora::gfx::Range>& outRange, u8 stride) {
  Vec* vecPtr = static_cast<Vec*>(data);
  outPtr = vecPtr;
  if (stride == 1) {
    const auto hash = aurora::xxh3_hash(vecPtr->data(), vecPtr->size() * sizeof(typename Vec::value_type), 0);
    const auto it = sCachedRanges.find(hash);
    if (it != sCachedRanges.end()) {
      outRange = it->second;
    } else {
      const auto range = aurora::gfx::push_static_storage(aurora::ArrayRef{*vecPtr});
      sCachedRanges.try_emplace(hash, range);
      outRange = range;
    }
  } else {
    outRange.reset();
  }
}

void GXSetArray(GX::Attr attr, const void* data, u8 stride) noexcept {
  using namespace aurora::gfx::model;
  switch (attr) {
  case GX::VA_POS:
    cache_array(data, vtxData, staticVtxRange, stride);
    break;
  case GX::VA_NRM:
    cache_array(data, nrmData, staticNrmRange, stride);
    break;
  case GX::VA_TEX0:
    cache_array(data, tex0TcData, staticPackedTcRange, stride);
    break;
  case GX::VA_TEX1:
    cache_array(data, tcData, staticTcRange, stride);
    break;
  default:
    Log.report(logvisor::Fatal, FMT_STRING("GXSetArray: invalid attr {}"), attr);
    unreachable();
  }
}

void GXCallDisplayList(const void* data, u32 nbytes) noexcept {
  aurora::gfx::model::queue_surface(static_cast<const u8*>(data), nbytes);
}
