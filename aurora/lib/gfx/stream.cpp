#include "stream/shader.hpp"

#include "../gpu.hpp"
#include "common.hpp"
#include "gx.hpp"

namespace aurora::gfx {
static logvisor::Module Log("aurora::gfx::stream");

struct SStreamState {
  GX::Primitive primitive;
  metaforce::EStreamFlags flags;
  uint32_t vertexCount = 0;
  ByteBuffer vertexBuffer;

  explicit SStreamState(GX::Primitive primitive) noexcept : primitive(primitive) {}
};
static std::optional<SStreamState> sStreamState;

void stream_begin(GX::Primitive primitive) noexcept {
  if (sStreamState) {
    Log.report(logvisor::Fatal, FMT_STRING("Stream began twice!"));
    unreachable();
  }
  sStreamState.emplace(primitive);
}

void stream_vertex(metaforce::EStreamFlags flags, const zeus::CVector3f& pos, const zeus::CVector3f& nrm,
                   const zeus::CColor& color, const zeus::CVector2f& uv) noexcept {
  if (!sStreamState) {
    Log.report(logvisor::Fatal, FMT_STRING("Stream not started!"));
    unreachable();
  }
  if (sStreamState->flags) {
    if (sStreamState->flags != flags) {
      Log.report(logvisor::Fatal, FMT_STRING("Stream changed flags?"));
      unreachable();
    }
  } else {
    sStreamState->flags = flags;
  }
  sStreamState->vertexBuffer.append(&pos, 12);
  if (flags & metaforce::EStreamFlagBits::fHasNormal) {
    sStreamState->vertexBuffer.append(&nrm, 12);
  }
  if (flags & metaforce::EStreamFlagBits::fHasColor) {
    sStreamState->vertexBuffer.append(&color, 16);
  }
  if (flags & metaforce::EStreamFlagBits::fHasTexture) {
    sStreamState->vertexBuffer.append(&uv, 8);
  }
  sStreamState->vertexCount++;
}

void stream_end() noexcept {
  const auto& tex = get_texture(GX::TEXMAP0);
  if (sStreamState->flags & metaforce::EStreamFlagBits::fHasTexture && !tex) {
    Log.report(logvisor::Fatal, FMT_STRING("Stream has texture but no texture bound!"));
    unreachable();
  }
  const auto vertRange = push_verts(sStreamState->vertexBuffer.data(), sStreamState->vertexBuffer.size());

  ByteBuffer uniBuf;
  std::bitset<g_colorRegs.size()> usedColors;
  {
    const auto xf = get_combined_matrix();
    uniBuf.append(&xf, 64);
  }
  if (sStreamState->flags & metaforce::EStreamFlagBits::fHasTexture) {
    uniBuf.append(&tex.lod, 4);
  }
  const auto uniRange = push_uniform(uniBuf.data(), uniBuf.size());

  const auto uniform_size = align_uniform(uniBuf.size());

  stream::PipelineConfig config{
      .uniformSize = uniform_size,
      .flags = sStreamState->flags,
  };
  config.shaderConfig.denormalizedVertexAttributes = true;
  config.shaderConfig.denormalizedNorm = sStreamState->flags.IsSet(metaforce::EStreamFlagBits::fHasNormal);
  config.shaderConfig.denormalizedColor = sStreamState->flags.IsSet(metaforce::EStreamFlagBits::fHasColor);
  populate_gx_pipeline_config(config, sStreamState->primitive, {1});
  const auto pipeline = pipeline_ref(config);

  BindGroupRef samplerBindGroup{};
  BindGroupRef textureBindGroup{};
  if (sStreamState->flags & metaforce::EStreamFlagBits::fHasTexture) {
    const auto& state = get_state<stream::State>();
    {
      const std::array samplerEntries{wgpu::BindGroupEntry{
          .binding = 0,
          .sampler = sampler_ref(tex.get_descriptor()),
      }};
      samplerBindGroup = bind_group_ref(wgpu::BindGroupDescriptor{
          .label = "Stream Sampler Bind Group",
          .layout = state.samplerLayout,
          .entryCount = samplerEntries.size(),
          .entries = samplerEntries.data(),
      });
    }
    {
      const std::array textureEntries{wgpu::BindGroupEntry{
          .binding = 0,
          .textureView = tex.handle.ref->view,
      }};
      textureBindGroup = bind_group_ref(wgpu::BindGroupDescriptor{
          .label = "Stream Texture Bind Group",
          .layout = state.textureLayout,
          .entryCount = textureEntries.size(),
          .entries = textureEntries.data(),
      });
    }
  }

  push_draw_command(stream::DrawData{
      .pipeline = pipeline,
      .vertRange = vertRange,
      .uniformRange = uniRange,
      .vertexCount = sStreamState->vertexCount,
      .uniformSize = uniform_size,
      .samplerBindGroup = samplerBindGroup,
      .textureBindGroup = textureBindGroup,
  });

  sStreamState.reset();
}
} // namespace aurora::gfx
