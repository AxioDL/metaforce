#include "common.hpp"

#include "../gpu.hpp"
#include "colored_quad/shader.hpp"
#include "movie_player/shader.hpp"
#include "stream/shader.hpp"
#include "textured_quad/shader.hpp"

#include <condition_variable>
#include <deque>
#include <logvisor/logvisor.hpp>
#include <thread>
#include <unordered_map>

namespace aurora::gfx {
static logvisor::Module Log("aurora::gfx");

using gpu::g_device;
using gpu::g_queue;

struct ShaderState {
  movie_player::State moviePlayer;
  colored_quad::State coloredQuad;
  textured_quad::State texturedQuad;
  stream::State stream;
};
struct ShaderDrawCommand {
  ShaderType type;
  union {
    movie_player::DrawData moviePlayer;
    colored_quad::DrawData coloredQuad;
    textured_quad::DrawData texturedQuad;
    stream::DrawData stream;
  };
};
struct PipelineCreateCommand {
  ShaderType type;
  union {
    movie_player::PipelineConfig moviePlayer;
    colored_quad::PipelineConfig coloredQuad;
    textured_quad::PipelineConfig texturedQuad;
    stream::PipelineConfig stream;
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
// GX state
metaforce::ERglCullMode g_cullMode;
metaforce::ERglBlendMode g_blendMode;
metaforce::ERglBlendFactor g_blendFacSrc;
metaforce::ERglBlendFactor g_blendFacDst;
metaforce::ERglLogicOp g_blendOp;
bool g_depthCompare;
bool g_depthUpdate;
metaforce::ERglEnum g_depthFunc;
std::array<zeus::CColor, 4> g_colorRegs;
bool g_alphaUpdate;
std::optional<float> g_dstAlpha;
zeus::CColor g_clearColor;

using NewPipelineCallback = std::function<wgpu::RenderPipeline()>;
static std::mutex g_pipelineMutex;
static std::thread g_pipelineThread;
static std::atomic_bool g_pipelineThreadEnd;
static std::condition_variable g_pipelineCv;
static std::unordered_map<PipelineRef, wgpu::RenderPipeline> g_pipelines;
static std::deque<std::pair<PipelineRef, NewPipelineCallback>> g_queuedPipelines;
static std::unordered_map<BindGroupRef, wgpu::BindGroup> g_cachedBindGroups;
static std::unordered_map<SamplerRef, wgpu::Sampler> g_cachedSamplers;
std::atomic_uint32_t queuedPipelines;
std::atomic_uint32_t createdPipelines;

static ByteBuffer g_verts;
static ByteBuffer g_uniforms;
static ByteBuffer g_indices;
wgpu::Buffer g_vertexBuffer;
wgpu::Buffer g_uniformBuffer;
wgpu::Buffer g_indexBuffer;

static ShaderState g_state;
static PipelineRef g_currentPipeline;

static std::vector<Command> g_commands;

static PipelineRef find_pipeline(PipelineCreateCommand command, NewPipelineCallback&& cb) {
  const auto hash = xxh3_hash(command);
  bool found = false;
  {
    std::scoped_lock guard{g_pipelineMutex};
    found = g_pipelines.find(hash) != g_pipelines.end();
    if (!found) {
      const auto ref =
          std::find_if(g_queuedPipelines.begin(), g_queuedPipelines.end(), [=](auto v) { return v.first == hash; });
      if (ref != g_queuedPipelines.end()) {
        found = true;
      }
    }
    if (!found) {
      g_queuedPipelines.emplace_back(std::pair{hash, std::move(cb)});
    }
  }
  if (!found) {
    g_pipelineCv.notify_one();
    queuedPipelines++;
  }
  return hash;
}

static void push_draw_command(ShaderDrawCommand data) { g_commands.push_back({CommandType::Draw, {.draw = data}}); }

bool get_dxt_compression_supported() noexcept { return g_device.HasFeature(wgpu::FeatureName::TextureCompressionBC); }

// GX state
void set_cull_mode(metaforce::ERglCullMode mode) noexcept { g_cullMode = mode; }
void set_blend_mode(metaforce::ERglBlendMode mode, metaforce::ERglBlendFactor src, metaforce::ERglBlendFactor dst,
                    metaforce::ERglLogicOp op) noexcept {
  g_blendMode = mode;
  g_blendFacSrc = src;
  g_blendFacDst = dst;
  g_blendOp = op;
}
void set_depth_mode(bool compare_enable, metaforce::ERglEnum func, bool update_enable) noexcept {
  g_depthCompare = compare_enable;
  g_depthFunc = func;
  g_depthUpdate = update_enable;
}
void set_gx_reg1_color(const zeus::CColor& color) noexcept { g_colorRegs[1] = color; }
void set_alpha_update(bool enabled) noexcept { g_alphaUpdate = enabled; }
void set_dst_alpha(bool enabled, float value) noexcept {
  if (enabled) {
    g_dstAlpha = value;
  } else {
    g_dstAlpha.reset();
  }
}
void set_clear_color(const zeus::CColor& color) noexcept { g_clearColor = color; }

// Model state
void set_alpha_discard(bool v) {}

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

void queue_textured_quad_verts(CameraFilterType filter_type, const TextureHandle& texture, ZComp z_comparison,
                               bool z_test, const zeus::CColor& color, const ArrayRef<zeus::CVector3f>& pos,
                               const ArrayRef<zeus::CVector2f>& uvs, float lod) noexcept {
  auto data = textured_quad::make_draw_data_verts(g_state.texturedQuad, filter_type, texture, z_comparison, z_test,
                                                  color, pos, uvs, lod);
  push_draw_command({.type = ShaderType::TexturedQuad, .texturedQuad = data});
}
void queue_textured_quad(CameraFilterType filter_type, const TextureHandle& texture, ZComp z_comparison, bool z_test,
                         const zeus::CColor& color, float uv_scale, const zeus::CRectangle& rect, float z,
                         float lod) noexcept {
  auto data = textured_quad::make_draw_data(g_state.texturedQuad, filter_type, texture, z_comparison, z_test, color,
                                            uv_scale, rect, z, lod);
  push_draw_command({.type = ShaderType::TexturedQuad, .texturedQuad = data});
}
template <>
PipelineRef pipeline_ref(textured_quad::PipelineConfig config) {
  return find_pipeline({.type = ShaderType::TexturedQuad, .texturedQuad = config},
                       [=]() { return create_pipeline(g_state.texturedQuad, config); });
}

void queue_colored_quad_verts(CameraFilterType filter_type, ZComp z_comparison, bool z_test, const zeus::CColor& color,
                              const ArrayRef<zeus::CVector3f>& pos) noexcept {
  auto data = colored_quad::make_draw_data_verts(g_state.coloredQuad, filter_type, z_comparison, z_test, color, pos);
  push_draw_command({.type = ShaderType::ColoredQuad, .coloredQuad = data});
}
void queue_colored_quad(CameraFilterType filter_type, ZComp z_comparison, bool z_test, const zeus::CColor& color,
                        const zeus::CRectangle& rect, float z) noexcept {
  auto data = colored_quad::make_draw_data(g_state.coloredQuad, filter_type, z_comparison, z_test, color, rect, z);
  push_draw_command({.type = ShaderType::ColoredQuad, .coloredQuad = data});
}
template <>
PipelineRef pipeline_ref(colored_quad::PipelineConfig config) {
  return find_pipeline({.type = ShaderType::ColoredQuad, .coloredQuad = config},
                       [=]() { return create_pipeline(g_state.coloredQuad, config); });
}

void queue_movie_player(const TextureHandle& tex_y, const TextureHandle& tex_u, const TextureHandle& tex_v,
                        const zeus::CVector3f& v1, const zeus::CVector3f& v2, const zeus::CVector3f& v3,
                        const zeus::CVector3f& v4) noexcept {
  auto data = movie_player::make_draw_data(g_state.moviePlayer, tex_y, tex_u, tex_v, v1, v2, v3, v4);
  push_draw_command({.type = ShaderType::MoviePlayer, .moviePlayer = data});
}
template <>
PipelineRef pipeline_ref(movie_player::PipelineConfig config) {
  return find_pipeline({.type = ShaderType::MoviePlayer, .moviePlayer = config},
                       [=]() { return create_pipeline(g_state.moviePlayer, config); });
}

template <>
const stream::State& get_state() {
  return g_state.stream;
}
template <>
void push_draw_command(stream::DrawData data) {
  push_draw_command({.type = ShaderType::Stream, .stream = data});
}
template <>
PipelineRef pipeline_ref(stream::PipelineConfig config) {
  return find_pipeline({.type = ShaderType::Stream, .stream = config},
                       [=]() { return create_pipeline(g_state.stream, config); });
}

static void pipeline_worker() {
  bool hasMore = false;
  while (true) {
    std::pair<PipelineRef, NewPipelineCallback> cb;
    {
      std::unique_lock lock{g_pipelineMutex};
      if (!hasMore) {
        g_pipelineCv.wait(lock, [] { return !g_queuedPipelines.empty() || g_pipelineThreadEnd; });
      }
      if (g_pipelineThreadEnd) {
        break;
      }
      cb = std::move(g_queuedPipelines.front());
    }
    auto result = cb.second();
    // std::this_thread::sleep_for(std::chrono::milliseconds{1500});
    {
      std::scoped_lock lock{g_pipelineMutex};
      if (g_pipelines.contains(cb.first)) {
        Log.report(logvisor::Fatal, FMT_STRING("Duplicate pipeline {}"), cb.first);
        unreachable();
      }
      g_pipelines[cb.first] = result;
      g_queuedPipelines.pop_front();
      hasMore = !g_queuedPipelines.empty();
    }
    createdPipelines++;
    queuedPipelines--;
  }
}

void initialize() {
  g_pipelineThread = std::thread(pipeline_worker);

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
  g_state.coloredQuad = colored_quad::construct_state();
  g_state.texturedQuad = textured_quad::construct_state();
  g_state.stream = stream::construct_state();
}

void shutdown() {
  g_pipelineThreadEnd = true;
  g_pipelineCv.notify_all();
  g_pipelineThread.join();

  g_cachedBindGroups.clear();
  g_pipelines.clear();
  g_vertexBuffer = {};
  g_uniformBuffer = {};
  g_indexBuffer = {};

  g_state = {};
}

void render(const wgpu::RenderPassEncoder& pass) {
  {
    if (g_verts.size() > 0) {
      g_queue.WriteBuffer(g_vertexBuffer, 0, g_verts.data(), g_verts.size());
      g_verts.clear();
    }
    if (g_uniforms.size() > 0) {
      g_queue.WriteBuffer(g_uniformBuffer, 0, g_uniforms.data(), g_uniforms.size());
      g_uniforms.clear();
    }
    if (g_indices.size() > 0) {
      g_queue.WriteBuffer(g_indexBuffer, 0, g_indices.data(), g_indices.size());
      g_indices.clear();
    }
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
      case ShaderType::ColoredQuad:
        colored_quad::render(g_state.coloredQuad, draw.coloredQuad, pass);
        break;
      case ShaderType::TexturedQuad:
        textured_quad::render(g_state.texturedQuad, draw.texturedQuad, pass);
        break;
      case ShaderType::MoviePlayer:
        movie_player::render(g_state.moviePlayer, draw.moviePlayer, pass);
        break;
      case ShaderType::Stream:
        stream::render(g_state.stream, draw.stream, pass);
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
  g_currentPipeline = ref;
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
    unreachable();
  }
  return g_cachedBindGroups[id];
}

const wgpu::Sampler& sampler_ref(const wgpu::SamplerDescriptor& descriptor) {
  const auto id = xxh3_hash(descriptor);
  if (!g_cachedSamplers.contains(id)) {
    g_cachedSamplers[id] = g_device.CreateSampler(&descriptor);
  }
  return g_cachedSamplers[id];
}

uint32_t align_uniform(uint32_t value) {
  wgpu::SupportedLimits limits;
  g_device.GetLimits(&limits); // TODO cache
  const auto uniform_alignment = limits.limits.minUniformBufferOffsetAlignment;
  return ALIGN(value, uniform_alignment);
}
} // namespace aurora::gfx
