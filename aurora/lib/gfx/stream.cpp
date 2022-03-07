#include "stream/shader.hpp"

#include "../gpu.hpp"
#include "common.hpp"
#include "gx.hpp"

#include <unordered_map>

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
  const auto vertRange = push_verts(sStreamState->vertexBuffer.data(), sStreamState->vertexBuffer.size());

  stream::PipelineConfig config{};
  config.shaderConfig.denormalizedVertexAttributes = true;
  const auto info = populate_pipeline_config(config, sStreamState->primitive);
  const auto pipeline = pipeline_ref(config);

  push_draw_command(stream::DrawData{
      .pipeline = pipeline,
      .vertRange = vertRange,
      .uniformRange = build_uniform(info),
      .vertexCount = sStreamState->vertexCount,
      .bindGroups = build_bind_groups(info),
  });

  sStreamState.reset();
}
} // namespace aurora::gfx
