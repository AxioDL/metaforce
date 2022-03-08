#include "common.hpp"

#include "../gpu.hpp"
#include "colored_quad/shader.hpp"
#include "movie_player/shader.hpp"
#include "stream/shader.hpp"
#include "textured_quad/shader.hpp"
#include "model/shader.hpp"

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
  model::State model;
};
struct ShaderDrawCommand {
  ShaderType type;
  union {
    movie_player::DrawData moviePlayer;
    colored_quad::DrawData coloredQuad;
    textured_quad::DrawData texturedQuad;
    stream::DrawData stream;
    model::DrawData model;
  };
};
struct PipelineCreateCommand {
  ShaderType type;
  union {
    movie_player::PipelineConfig moviePlayer;
    colored_quad::PipelineConfig coloredQuad;
    textured_quad::PipelineConfig texturedQuad;
    stream::PipelineConfig stream;
    model::PipelineConfig model;
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
} // namespace aurora::gfx

namespace aurora {
template <>
inline void xxh3_update(XXH3_state_t& state, const gfx::colored_quad::PipelineConfig& input) {
  XXH3_64bits_update(&state, &input.filterType, sizeof(gfx::colored_quad::PipelineConfig::filterType));
  XXH3_64bits_update(&state, &input.zComparison, sizeof(gfx::colored_quad::PipelineConfig::zComparison));
  XXH3_64bits_update(&state, &input.zTest, sizeof(gfx::colored_quad::PipelineConfig::zTest));
}
template <>
inline void xxh3_update(XXH3_state_t& state, const gfx::textured_quad::PipelineConfig& input) {
  XXH3_64bits_update(&state, &input.filterType, sizeof(gfx::textured_quad::PipelineConfig::filterType));
  XXH3_64bits_update(&state, &input.zComparison, sizeof(gfx::textured_quad::PipelineConfig::zComparison));
  XXH3_64bits_update(&state, &input.zTest, sizeof(gfx::textured_quad::PipelineConfig::zTest));
}
template <>
inline void xxh3_update(XXH3_state_t& state, const gfx::movie_player::PipelineConfig& input) {
  // no-op
}
template <>
inline void xxh3_update(XXH3_state_t& state, const gfx::gx::PipelineConfig& input) {
  xxh3_update(state, input.shaderConfig);
  XXH3_64bits_update(&state, &input.primitive, sizeof(gfx::gx::PipelineConfig::primitive));
  XXH3_64bits_update(&state, &input.depthFunc, sizeof(gfx::gx::PipelineConfig::depthFunc));
  XXH3_64bits_update(&state, &input.cullMode, sizeof(gfx::gx::PipelineConfig::cullMode));
  XXH3_64bits_update(&state, &input.blendMode, sizeof(gfx::gx::PipelineConfig::blendMode));
  XXH3_64bits_update(&state, &input.blendFacSrc, sizeof(gfx::gx::PipelineConfig::blendFacSrc));
  XXH3_64bits_update(&state, &input.blendFacDst, sizeof(gfx::gx::PipelineConfig::blendFacDst));
  XXH3_64bits_update(&state, &input.blendOp, sizeof(gfx::gx::PipelineConfig::blendOp));
  if (input.dstAlpha) {
    XXH3_64bits_update(&state, &*input.dstAlpha, sizeof(float));
  }
  XXH3_64bits_update(&state, &input.depthCompare, sizeof(gfx::gx::PipelineConfig::depthCompare));
  XXH3_64bits_update(&state, &input.depthUpdate, sizeof(gfx::gx::PipelineConfig::depthUpdate));
  XXH3_64bits_update(&state, &input.alphaUpdate, sizeof(gfx::gx::PipelineConfig::alphaUpdate));
}
template <>
inline void xxh3_update(XXH3_state_t& state, const gfx::stream::PipelineConfig& input) {
  xxh3_update<gfx::gx::PipelineConfig>(state, input);
}
template <>
inline void xxh3_update(XXH3_state_t& state, const gfx::model::PipelineConfig& input) {
  xxh3_update<gfx::gx::PipelineConfig>(state, input);
}
template <>
inline void xxh3_update(XXH3_state_t& state, const gfx::PipelineCreateCommand& input) {
  XXH3_64bits_update(&state, &input.type, sizeof(gfx::PipelineCreateCommand::type));
  switch (input.type) {
  case gfx::ShaderType::Aabb:
    // TODO
    break;
  case gfx::ShaderType::ColoredQuad:
    xxh3_update(state, input.coloredQuad);
    break;
  case gfx::ShaderType::TexturedQuad:
    xxh3_update(state, input.texturedQuad);
    break;
  case gfx::ShaderType::MoviePlayer:
    xxh3_update(state, input.moviePlayer);
    break;
  case gfx::ShaderType::Stream:
    xxh3_update(state, input.stream);
    break;
  case gfx::ShaderType::Model:
    xxh3_update(state, input.model);
    break;
  }
}
template <>
inline void xxh3_update(XXH3_state_t& state, const wgpu::BindGroupEntry& input) {
  XXH3_64bits_update(&state, &input.binding, sizeof(wgpu::BindGroupEntry::binding));
  XXH3_64bits_update(&state, &input.buffer, sizeof(wgpu::BindGroupEntry::buffer));
  XXH3_64bits_update(&state, &input.offset, sizeof(wgpu::BindGroupEntry::offset));
  if (input.buffer != nullptr) {
    XXH3_64bits_update(&state, &input.size, sizeof(wgpu::BindGroupEntry::size));
  }
  XXH3_64bits_update(&state, &input.sampler, sizeof(wgpu::BindGroupEntry::sampler));
  XXH3_64bits_update(&state, &input.textureView, sizeof(wgpu::BindGroupEntry::textureView));
}
template <>
inline void xxh3_update(XXH3_state_t& state, const wgpu::BindGroupDescriptor& input) {
  if (input.label != nullptr) {
    XXH3_64bits_update(&state, input.label, strlen(input.label));
  }
  XXH3_64bits_update(&state, &input.layout, sizeof(wgpu::BindGroupDescriptor::layout));
  XXH3_64bits_update(&state, &input.entryCount, sizeof(wgpu::BindGroupDescriptor::entryCount));
  for (int i = 0; i < input.entryCount; ++i) {
    xxh3_update(state, input.entries[i]);
  }
}
template <>
inline void xxh3_update(XXH3_state_t& state, const wgpu::SamplerDescriptor& input) {
  if (input.label != nullptr) {
    XXH3_64bits_update(&state, input.label, strlen(input.label));
  }
  XXH3_64bits_update(&state, &input.addressModeU, sizeof(wgpu::SamplerDescriptor::addressModeU));
  XXH3_64bits_update(&state, &input.addressModeV, sizeof(wgpu::SamplerDescriptor::addressModeV));
  XXH3_64bits_update(&state, &input.addressModeW, sizeof(wgpu::SamplerDescriptor::addressModeW));
  XXH3_64bits_update(&state, &input.magFilter, sizeof(wgpu::SamplerDescriptor::magFilter));
  XXH3_64bits_update(&state, &input.minFilter, sizeof(wgpu::SamplerDescriptor::minFilter));
  XXH3_64bits_update(&state, &input.mipmapFilter, sizeof(wgpu::SamplerDescriptor::mipmapFilter));
  XXH3_64bits_update(&state, &input.lodMinClamp, sizeof(wgpu::SamplerDescriptor::lodMinClamp));
  XXH3_64bits_update(&state, &input.lodMaxClamp, sizeof(wgpu::SamplerDescriptor::lodMaxClamp));
  XXH3_64bits_update(&state, &input.compare, sizeof(wgpu::SamplerDescriptor::compare));
  XXH3_64bits_update(&state, &input.maxAnisotropy, sizeof(wgpu::SamplerDescriptor::maxAnisotropy));
}
} // namespace aurora

namespace aurora::gfx {
using NewPipelineCallback = std::function<wgpu::RenderPipeline()>;
std::mutex g_pipelineMutex;
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
static ByteBuffer g_storage;
wgpu::Buffer g_vertexBuffer;
wgpu::Buffer g_uniformBuffer;
wgpu::Buffer g_indexBuffer;
wgpu::Buffer g_storageBuffer;

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

void queue_movie_player(const TextureHandle& tex_y, const TextureHandle& tex_u, const TextureHandle& tex_v, float h_pad,
                        float v_pad) noexcept {
  auto data = movie_player::make_draw_data(g_state.moviePlayer, tex_y, tex_u, tex_v, h_pad, v_pad);
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

template <>
void push_draw_command(model::DrawData data) {
  push_draw_command({.type = ShaderType::Model, .model = data});
}
template <>
PipelineRef pipeline_ref(model::PipelineConfig config) {
  return find_pipeline({.type = ShaderType::Model, .model = config},
                       [=]() { return create_pipeline(g_state.model, config); });
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
    const wgpu::BufferDescriptor descriptor{
        .label = "Shared Uniform Buffer",
        .usage = wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst,
        .size = 134217728, // 128mb
    };
    g_uniformBuffer = g_device.CreateBuffer(&descriptor);
  }
  {
    const wgpu::BufferDescriptor descriptor{
        .label = "Shared Vertex Buffer",
        .usage = wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst,
        .size = 16777216, // 16mb
    };
    g_vertexBuffer = g_device.CreateBuffer(&descriptor);
  }
  {
    const wgpu::BufferDescriptor descriptor{
        .label = "Shared Index Buffer",
        .usage = wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst,
        .size = 4194304, // 4mb
    };
    g_indexBuffer = g_device.CreateBuffer(&descriptor);
  }
  {
    const wgpu::BufferDescriptor descriptor{
        .label = "Shared Storage Buffer",
        .usage = wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst,
        .size = 134217728, // 128mb
    };
    g_storageBuffer = g_device.CreateBuffer(&descriptor);
  }

  g_state.moviePlayer = movie_player::construct_state();
  g_state.coloredQuad = colored_quad::construct_state();
  g_state.texturedQuad = textured_quad::construct_state();
  g_state.stream = stream::construct_state();
  g_state.model = model::construct_state();
}

void shutdown() {
  g_pipelineThreadEnd = true;
  g_pipelineCv.notify_all();
  g_pipelineThread.join();

  gx::shutdown();

  g_cachedBindGroups.clear();
  g_cachedSamplers.clear();
  g_pipelines.clear();
  g_vertexBuffer = {};
  g_uniformBuffer = {};
  g_indexBuffer = {};
  g_storageBuffer = {};

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
    if (g_storage.size() > 0) {
      g_queue.WriteBuffer(g_storageBuffer, 0, g_storage.data(), g_storage.size());
      g_storage.clear();
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
      case ShaderType::Model:
        model::render(g_state.model, draw.model, pass);
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
  if (length == 0) {
    // TODO shared zero buf?
    length = alignment;
    target.append_zeroes(alignment);
  } else {
    target.append(data, length);
    if (padding > 0) {
      target.append_zeroes(padding);
    }
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
Range push_storage(const uint8_t* data, size_t length) {
  wgpu::SupportedLimits limits;
  g_device.GetLimits(&limits);
  return push(g_storage, data, length, limits.limits.minStorageBufferOffsetAlignment);
}

BindGroupRef bind_group_ref(const wgpu::BindGroupDescriptor& descriptor) {
  const auto id = xxh3_hash(descriptor);
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
