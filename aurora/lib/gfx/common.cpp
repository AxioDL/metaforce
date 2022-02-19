#include "common.hpp"

#include "../gpu.hpp"
#include "movie_player/shader.hpp"

#include <logvisor/logvisor.hpp>
#include <unordered_map>

namespace aurora::gfx {
static logvisor::Module Log("aurora::gfx");

using gpu::g_device;
using gpu::g_queue;

struct ShaderState {
  movie_player::State moviePlayer;
};
struct ShaderDrawCommand {
  ShaderType type;
  union {
    movie_player::DrawData moviePlayer;
  };
};
struct PipelineCreateCommand {
  ShaderType type;
  union {
    movie_player::PipelineConfig moviePlayer;
  };
};
enum class CommandType {
  SetViewport,
  SetScissor,
  Draw,
};
struct Command {
  CommandType type;
  union Data {
    struct SetViewportCommand {
      zeus::CRectangle rect;
      float znear;
      float zfar;
    } setViewport;
    struct SetScissorCommand {
      uint32_t x;
      uint32_t y;
      uint32_t w;
      uint32_t h;
    } setScissor;
    ShaderDrawCommand draw;
  } data;
};

zeus::CMatrix4f g_mv;
zeus::CMatrix4f g_mvInv;
zeus::CMatrix4f g_proj;
metaforce::CFogState g_fogState;

static std::mutex g_pipelineMutex;
static std::unordered_map<uint64_t, wgpu::RenderPipeline> g_pipelines;
static std::vector<PipelineCreateCommand> g_queuedPipelines;
static std::unordered_map<BindGroupRef, wgpu::BindGroup> g_cachedBindGroups;

static ByteBuffer g_verts;
static ByteBuffer g_uniforms;
static ByteBuffer g_indices;
wgpu::Buffer g_vertexBuffer;
wgpu::Buffer g_uniformBuffer;
wgpu::Buffer g_indexBuffer;

static ShaderState g_state;
static PipelineRef g_currentPipeline;

static std::vector<Command> g_commands;

using NewPipelineCallback = std::function<wgpu::RenderPipeline()>;
static PipelineRef find_pipeline(PipelineCreateCommand command, NewPipelineCallback cb) {
  const auto hash = xxh3_hash(command);
  bool found;
  {
    std::lock_guard guard{g_pipelineMutex};
    const auto ref = g_pipelines.find(hash);
    found = ref != g_pipelines.end();
  }
  if (!found) {
    // TODO another thread
    wgpu::RenderPipeline pipeline = cb();
    {
      std::lock_guard guard{g_pipelineMutex};
      g_pipelines[hash] = std::move(pipeline);
    }
  }
  return hash;
}

static void push_draw_command(ShaderDrawCommand data) { g_commands.push_back({CommandType::Draw, {.draw = data}}); }

bool get_dxt_compression_supported() noexcept { return g_device.HasFeature(wgpu::FeatureName::TextureCompressionBC); }

void update_model_view(const zeus::CMatrix4f& mv, const zeus::CMatrix4f& mv_inv) noexcept {
  g_mv = mv;
  g_mvInv = mv_inv;
}
void update_projection(const zeus::CMatrix4f& proj) noexcept { g_proj = proj; }
void update_fog_state(const metaforce::CFogState& state) noexcept { g_fogState = state; }
void set_viewport(const zeus::CRectangle& rect, float znear, float zfar) noexcept {
  g_commands.push_back({CommandType::SetViewport, {.setViewport = {rect, znear, zfar}}});
}
void set_scissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h) noexcept {
  g_commands.push_back({CommandType::SetScissor, {.setScissor = {x, y, w, h}}});
}

void resolve_color(const ClipRect& rect, uint32_t bind, bool clear_depth) noexcept {
  // TODO
}
void resolve_depth(const ClipRect& rect, uint32_t bind) noexcept {
  // TODO
}

void add_material_set(/* TODO */) noexcept {}
void add_model(/* TODO */) noexcept {}

void queue_aabb(const zeus::CAABox& aabb, const zeus::CColor& color, bool z_only) noexcept {
  // TODO
}
void queue_fog_volume_plane(const ArrayRef<zeus::CVector4f>& verts, uint8_t pass) {
  // TODO
}
void queue_fog_volume_filter(const zeus::CColor& color, bool two_way) noexcept {
  // TODO
}
void queue_textured_quad_verts(CameraFilterType filter_type, const TextureHandle& texture, ZTest z_comparison,
                               bool z_test, const zeus::CColor& color, const ArrayRef<zeus::CVector3f>& pos,
                               const ArrayRef<zeus::CVector2f>& uvs, float lod) noexcept {
  // TODO
}
void queue_textured_quad(CameraFilterType filter_type, const TextureHandle& texture, ZTest z_comparison, bool z_test,
                         const zeus::CColor& color, float uv_scale, const zeus::CRectangle& rect, float z) noexcept {
  // TODO
}
void queue_colored_quad_verts(CameraFilterType filter_type, ZTest z_comparison, bool z_test, const zeus::CColor& color,
                              const ArrayRef<zeus::CVector3f>& pos) noexcept {
  // TODO
}
void queue_colored_quad(CameraFilterType filter_type, ZTest z_comparison, bool z_test, const zeus::CColor& color,
                        const zeus::CRectangle& rect, float z) noexcept {
  // TODO
}

void queue_movie_player(const TextureHandle& tex_y, const TextureHandle& tex_u, const TextureHandle& tex_v,
                        const zeus::CColor& color, float h_pad, float v_pad) noexcept {
  auto data = movie_player::make_draw_data(g_state.moviePlayer, tex_y, tex_u, tex_v, color, h_pad, v_pad);
  push_draw_command({.type = ShaderType::MoviePlayer, .moviePlayer = data});
}
template <>
PipelineRef pipeline_ref(movie_player::PipelineConfig config) {
  return find_pipeline({.type = ShaderType::MoviePlayer, .moviePlayer = config},
                       [=]() { return create_pipeline(g_state.moviePlayer, config); });
}

void construct_state() {
  {
    const auto uniformDescriptor = wgpu::BufferDescriptor{
        .label = "Shared Uniform Buffer",
        .usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
        .size = 134217728, // 128mb
    };
    g_uniformBuffer = g_device.CreateBuffer(&uniformDescriptor);
  }
  {
    const auto vertexDescriptor = wgpu::BufferDescriptor{
        .label = "Shared Vertex Buffer",
        .usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst,
        .size = 16777216, // 16mb
    };
    g_vertexBuffer = g_device.CreateBuffer(&vertexDescriptor);
  }
  {
    const auto vertexDescriptor = wgpu::BufferDescriptor{
        .label = "Shared Index Buffer",
        .usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst,
        .size = 4194304, // 4mb
    };
    g_indexBuffer = g_device.CreateBuffer(&vertexDescriptor);
  }

  g_state.moviePlayer = movie_player::construct_state();
}

void render(const wgpu::RenderPassEncoder& pass) {
  {
    g_queue.WriteBuffer(g_vertexBuffer, 0, g_verts.data(), g_verts.size());
    g_queue.WriteBuffer(g_uniformBuffer, 0, g_uniforms.data(), g_uniforms.size());
    g_queue.WriteBuffer(g_indexBuffer, 0, g_indices.data(), g_indices.size());
    g_verts.clear();
    g_uniforms.clear();
    g_indices.clear();
  }

  g_currentPipeline = UINT64_MAX;

  for (const auto& cmd : g_commands) {
    switch (cmd.type) {
    case CommandType::SetViewport: {
      const auto& vp = cmd.data.setViewport;
      pass.SetViewport(vp.rect.position.x(), vp.rect.position.y(), vp.rect.size.x(), vp.rect.size.y(), vp.znear,
                       vp.zfar);
    } break;
    case CommandType::SetScissor: {
      const auto& sc = cmd.data.setScissor;
      pass.SetScissorRect(sc.x, sc.y, sc.w, sc.h);
    } break;
    case CommandType::Draw: {
      const auto& draw = cmd.data.draw;
      switch (draw.type) {
      case ShaderType::Aabb:
        // TODO
        break;
      case ShaderType::TexturedQuad:
        // TODO
        break;
      case ShaderType::MoviePlayer:
        movie_player::render(g_state.moviePlayer, draw.moviePlayer, pass);
        break;
      }
    } break;
    }
  }

  g_commands.clear();
}

bool bind_pipeline(PipelineRef ref, const wgpu::RenderPassEncoder& pass) {
  if (ref == g_currentPipeline) {
    return true;
  }
  std::lock_guard guard{g_pipelineMutex};
  if (!g_pipelines.contains(ref)) {
    return false;
  }
  pass.SetPipeline(g_pipelines[ref]);
  return true;
}

static inline Range push(ByteBuffer& target, const uint8_t* data, size_t length, size_t alignment) {
  size_t padding = 0;
  if (alignment != 0) {
    padding = alignment - length % alignment;
  }
  auto begin = target.size();
  target.append(data, length);
  if (padding > 0) {
    target.append_zeroes(padding);
  }
  return {begin, begin + length};
}
Range push_verts(const uint8_t* data, size_t length) { return push(g_verts, data, length, 0 /* TODO? */); }
Range push_indices(const uint8_t* data, size_t length) { return push(g_indices, data, length, 0 /* TODO? */); }
Range push_uniform(const uint8_t* data, size_t length) {
  wgpu::SupportedLimits limits;
  g_device.GetLimits(&limits);
  return push(g_uniforms, data, length, limits.limits.minUniformBufferOffsetAlignment);
}

BindGroupRef bind_group_ref(const wgpu::BindGroupDescriptor& descriptor) {
  const auto id =
      xxh3_hash(descriptor.entries, descriptor.entryCount * sizeof(wgpu::BindGroupEntry), xxh3_hash(descriptor));
  if (!g_cachedBindGroups.contains(id)) {
    g_cachedBindGroups[id] = g_device.CreateBindGroup(&descriptor);
  }
  return id;
}
const wgpu::BindGroup& find_bind_group(BindGroupRef id) {
  if (!g_cachedBindGroups.contains(id)) {
    Log.report(logvisor::Fatal, FMT_STRING("get_bind_group: failed to locate {}"), id);
  }
  return g_cachedBindGroups[id];
}
} // namespace aurora::gfx
