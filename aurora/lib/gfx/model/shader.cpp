#include "shader.hpp"

#include "../../gpu.hpp"
#include "../common.hpp"

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

void set_vertex_buffer(const std::vector<zeus::CVector3f>* data) noexcept { vtxData = data; }
void set_normal_buffer(const std::vector<zeus::CVector3f>* norm) noexcept { nrmData = norm; }
void set_tex0_tc_buffer(const std::vector<Vec2<float>>* tcs) noexcept { tex0TcData = tcs; }
void set_tc_buffer(const std::vector<Vec2<float>>* tcs) noexcept { tcData = tcs; }

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

void queue_surface(const u8* dlStart, u32 dlSize) noexcept {
  //  Log.report(logvisor::Info, FMT_STRING("DL size {}"), dlSize);
  std::vector<gx::DlVert> verts;
  std::vector<u32> indices;

  size_t offset = 0;
  while (offset < dlSize - 6) {
    const auto header = dlStart[offset];
    const auto primitive = static_cast<GX::Primitive>(header & 0xF8);
    const auto vtxFmt = static_cast<VertexFormat>(header & 0x3);
    const auto vtxCount = metaforce::SBig(*reinterpret_cast<const u16*>(dlStart + offset + 1));
    //    Log.report(logvisor::Info, FMT_STRING("DL header prim {}, fmt {}, vtx count {}"), primitive,
    //               magic_enum::enum_name(vtxFmt), vtxCount);
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

  //  Log.report(logvisor::Info, FMT_STRING("Read {} verts, {} indices"), verts.size(), indices.size());
  const auto vertRange = push_verts(ArrayRef{verts});
  const auto idxRange = push_indices(ArrayRef{indices});
  const auto sVtxRange = push_storage(reinterpret_cast<const uint8_t*>(vtxData->data()), vtxData->size() * 16);
  const auto sNrmRange = push_storage(reinterpret_cast<const uint8_t*>(nrmData->data()), nrmData->size() * 16);
  const auto sTcRange = push_storage(reinterpret_cast<const uint8_t*>(tcData->data()), tcData->size() * 8);
  Range sPackedTcRange;
  if (tcData == tex0TcData) {
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
      .sVtxRange = sVtxRange,
      .sNrmRange = sNrmRange,
      .sTcRange = sTcRange,
      .sPackedTcRange = sPackedTcRange,
      .uniformRange = build_uniform(info),
      .indexCount = static_cast<uint32_t>(indices.size()),
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
      data.uniformRange.first, data.sVtxRange.first,      data.sNrmRange.first,
      data.sTcRange.first,     data.sPackedTcRange.first,
  };
  pass.SetBindGroup(0, find_bind_group(data.bindGroups.uniformBindGroup), offsets.size(), offsets.data());
  if (data.bindGroups.samplerBindGroup && data.bindGroups.textureBindGroup) {
    pass.SetBindGroup(1, find_bind_group(data.bindGroups.samplerBindGroup));
    pass.SetBindGroup(2, find_bind_group(data.bindGroups.textureBindGroup));
  }
  pass.SetVertexBuffer(0, g_vertexBuffer, data.vertRange.first, data.vertRange.second);
  pass.SetIndexBuffer(g_indexBuffer, wgpu::IndexFormat::Uint32, data.idxRange.first, data.idxRange.second);
  pass.DrawIndexed(data.indexCount);
}
} // namespace aurora::gfx::model
