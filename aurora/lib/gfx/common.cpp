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
#include <absl/container/flat_hash_map.h>

namespace aurora::gfx {
static logvisor::Module Log("aurora::gfx");

using gpu::g_device;
using gpu::g_queue;

constexpr uint64_t UniformBufferSize = 5242880;   // 5mb
constexpr uint64_t VertexBufferSize = 5242880;    // 5mb
constexpr uint64_t IndexBufferSize = 2097152;     // 2mb
constexpr uint64_t StorageBufferSize = 134217728; // 128mb

constexpr uint64_t StagingBufferSize = UniformBufferSize + VertexBufferSize + IndexBufferSize + StorageBufferSize;

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
      float left;
      float top;
      float width;
      float height;
      float znear;
      float zfar;
      bool operator==(const SetViewportCommand& rhs) const = default;
    } setViewport;
    struct SetScissorCommand {
      uint32_t x;
      uint32_t y;
      uint32_t w;
      uint32_t h;
      auto operator<=>(const SetScissorCommand&) const = default;
    } setScissor;
    ShaderDrawCommand draw;
  } data;
};
} // namespace aurora::gfx

namespace aurora {
template <>
inline XXH64_hash_t xxh3_hash(const gfx::PipelineCreateCommand& input, XXH64_hash_t seed) {
  constexpr auto typeSize = sizeof(gfx::PipelineCreateCommand::type);
  switch (input.type) {
  case gfx::ShaderType::Aabb:
    // TODO
    break;
  case gfx::ShaderType::ColoredQuad:
    return xxh3_hash_s(&input, typeSize + sizeof(gfx::PipelineCreateCommand::coloredQuad), seed);
  case gfx::ShaderType::TexturedQuad:
    return xxh3_hash_s(&input, typeSize + sizeof(gfx::PipelineCreateCommand::texturedQuad), seed);
  case gfx::ShaderType::MoviePlayer:
    return xxh3_hash_s(&input, typeSize + sizeof(gfx::PipelineCreateCommand::moviePlayer), seed);
  case gfx::ShaderType::Stream:
    return xxh3_hash_s(&input, typeSize + sizeof(gfx::PipelineCreateCommand::stream), seed);
  case gfx::ShaderType::Model:
    return xxh3_hash_s(&input, typeSize + sizeof(gfx::PipelineCreateCommand::model), seed);
  }
  return 0;
}
template <>
inline XXH64_hash_t xxh3_hash(const wgpu::BindGroupDescriptor& input, XXH64_hash_t seed) {
  constexpr auto offset = sizeof(void*) * 2; // skip nextInChain, label
  const auto hash = xxh3_hash_s(reinterpret_cast<const u8*>(&input) + offset,
                                sizeof(wgpu::BindGroupDescriptor) - offset - sizeof(void*) /* skip entries */, seed);
  //  // TODO ensure size is zeroed elsewhere
  //  for (int i = 0; i < input.entryCount; ++i) {
  //    const wgpu::BindGroupEntry& entry = input.entries[i];
  //    if (!entry.buffer && entry.size != 0) {
  //      gfx::Log.report(logvisor::Fatal, FMT_STRING("Size != 0! {}"), entry.size);
  //    }
  //  }
  return xxh3_hash_s(input.entries, sizeof(wgpu::BindGroupEntry) * input.entryCount, hash);
}
template <>
inline XXH64_hash_t xxh3_hash(const wgpu::SamplerDescriptor& input, XXH64_hash_t seed) {
  constexpr auto offset = sizeof(void*) * 2; // skip nextInChain, label
  return xxh3_hash_s(reinterpret_cast<const u8*>(&input) + offset,
                     sizeof(wgpu::SamplerDescriptor) - offset - 2 /* skip padding */, seed);
}
} // namespace aurora

namespace aurora::gfx {
using NewPipelineCallback = std::function<wgpu::RenderPipeline()>;
std::mutex g_pipelineMutex;
static std::thread g_pipelineThread;
static std::atomic_bool g_pipelineThreadEnd;
static std::condition_variable g_pipelineCv;
static absl::flat_hash_map<PipelineRef, wgpu::RenderPipeline> g_pipelines;
static std::deque<std::pair<PipelineRef, NewPipelineCallback>> g_queuedPipelines;
static absl::flat_hash_map<BindGroupRef, wgpu::BindGroup> g_cachedBindGroups;
static absl::flat_hash_map<SamplerRef, wgpu::Sampler> g_cachedSamplers;
std::atomic_uint32_t queuedPipelines;
std::atomic_uint32_t createdPipelines;

static ByteBuffer g_verts;
static ByteBuffer g_uniforms;
static ByteBuffer g_indices;
static ByteBuffer g_storage;
static ByteBuffer g_staticStorage;
wgpu::Buffer g_vertexBuffer;
wgpu::Buffer g_uniformBuffer;
wgpu::Buffer g_indexBuffer;
wgpu::Buffer g_storageBuffer;
size_t g_staticStorageLastSize = 0;
static std::array<wgpu::Buffer, 3> g_stagingBuffers;

static ShaderState g_state;
static PipelineRef g_currentPipeline;

static std::vector<Command> g_commands;

static PipelineRef find_pipeline(PipelineCreateCommand command, NewPipelineCallback&& cb) {
  OPTICK_EVENT();
  const auto hash = xxh3_hash(command);
  bool found = false;
  {
    std::scoped_lock guard{g_pipelineMutex};
    found = g_pipelines.contains(hash);
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

static Command::Data::SetViewportCommand g_cachedViewport;
void set_viewport(float left, float top, float width, float height, float znear, float zfar) noexcept {
  Command::Data::SetViewportCommand cmd{left, top, width, height, znear, zfar};
  if (cmd != g_cachedViewport) {
    g_commands.push_back({CommandType::SetViewport, {.setViewport = cmd}});
    g_cachedViewport = cmd;
  }
}
static Command::Data::SetScissorCommand g_cachedScissor;
void set_scissor(uint32_t x, uint32_t y, uint32_t w, uint32_t h) noexcept {
  Command::Data::SetScissorCommand cmd{x, y, w, h};
  if (cmd != g_cachedScissor) {
    g_commands.push_back({CommandType::SetScissor, {.setScissor = cmd}});
    g_cachedScissor = cmd;
  }
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
  OPTICK_THREAD("Pipeline Worker");
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
      if (!g_pipelines.try_emplace(cb.first, std::move(result)).second) {
        Log.report(logvisor::Fatal, FMT_STRING("Duplicate pipeline {}"), cb.first);
        unreachable();
      }
      g_queuedPipelines.pop_front();
      hasMore = !g_queuedPipelines.empty();
    }
    createdPipelines++;
    queuedPipelines--;
  }
}

void initialize() {
  gx::initialize();

  g_pipelineThread = std::thread(pipeline_worker);

  const auto createBuffer = [](wgpu::Buffer& out, wgpu::BufferUsage usage, uint64_t size, const char* label) {
    const wgpu::BufferDescriptor descriptor{
        .label = label,
        .usage = usage,
        .size = size,
    };
    out = g_device.CreateBuffer(&descriptor);
  };
  createBuffer(g_uniformBuffer, wgpu::BufferUsage::Uniform | wgpu::BufferUsage::CopyDst, UniformBufferSize,
               "Shared Uniform Buffer");
  createBuffer(g_vertexBuffer, wgpu::BufferUsage::Vertex | wgpu::BufferUsage::CopyDst, VertexBufferSize,
               "Shared Vertex Buffer");
  createBuffer(g_indexBuffer, wgpu::BufferUsage::Index | wgpu::BufferUsage::CopyDst, IndexBufferSize,
               "Shared Index Buffer");
  createBuffer(g_storageBuffer, wgpu::BufferUsage::Storage | wgpu::BufferUsage::CopyDst, StorageBufferSize,
               "Shared Storage Buffer");
  for (int i = 0; i < g_stagingBuffers.size(); ++i) {
    createBuffer(g_stagingBuffers[i], wgpu::BufferUsage::MapWrite | wgpu::BufferUsage::CopySrc, StagingBufferSize,
                 "Staging Buffer");
  }
  map_staging_buffer();

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
  g_stagingBuffers.fill({});

  g_state = {};
}

static size_t currentStagingBuffer = 0;
static bool bufferMapped = false;
void map_staging_buffer() {
  OPTICK_EVENT();
  bufferMapped = false;
  g_stagingBuffers[currentStagingBuffer].MapAsync(
      wgpu::MapMode::Write, 0, StagingBufferSize,
      [](WGPUBufferMapAsyncStatus status, void* userdata) {
        if (status == WGPUBufferMapAsyncStatus_DestroyedBeforeCallback) {
          return;
        } else if (status != WGPUBufferMapAsyncStatus_Success) {
          Log.report(logvisor::Fatal, FMT_STRING("Buffer mapping failed: {}"), status);
          unreachable();
        }
        *static_cast<bool*>(userdata) = true;
      },
      &bufferMapped);
}

void begin_frame() {
  {
    OPTICK_EVENT("Wait for buffer mapping");
    while (!bufferMapped) {
      g_device.Tick();
    }
  }
  OPTICK_EVENT();
  size_t bufferOffset = 0;
  auto& stagingBuf = g_stagingBuffers[currentStagingBuffer];
  const auto mapBuffer = [&](ByteBuffer& buf, uint64_t size) {
    buf = ByteBuffer{static_cast<u8*>(stagingBuf.GetMappedRange(bufferOffset, size)), size};
    bufferOffset += size;
  };
  mapBuffer(g_verts, VertexBufferSize);
  mapBuffer(g_uniforms, UniformBufferSize);
  mapBuffer(g_indices, IndexBufferSize);
  mapBuffer(g_storage, StorageBufferSize);
}

void end_frame(const wgpu::CommandEncoder& cmd) {
  OPTICK_EVENT();
  uint64_t bufferOffset = 0;
  const auto writeBuffer = [&](ByteBuffer& buf, wgpu::Buffer& out, uint64_t size, std::string_view label) {
    const auto writeSize = buf.size(); // Only need to copy this many bytes
    if (writeSize > 0) {
      cmd.CopyBufferToBuffer(g_stagingBuffers[currentStagingBuffer], bufferOffset, out, 0, writeSize);
      buf.clear();
    }
    bufferOffset += size;
  };
  g_stagingBuffers[currentStagingBuffer].Unmap();
  writeBuffer(g_verts, g_vertexBuffer, VertexBufferSize, "Vertex");
  writeBuffer(g_uniforms, g_uniformBuffer, UniformBufferSize, "Uniform");
  writeBuffer(g_indices, g_indexBuffer, IndexBufferSize, "Index");
  writeBuffer(g_storage, g_storageBuffer, StorageBufferSize, "Storage");
  currentStagingBuffer = (currentStagingBuffer + 1) % g_stagingBuffers.size();
  map_staging_buffer();
}

void render(const wgpu::RenderPassEncoder& pass) {
  OPTICK_EVENT();
  g_currentPipeline = UINT64_MAX;

  for (const auto& cmd : g_commands) {
    switch (cmd.type) {
    case CommandType::SetViewport: {
      const auto& vp = cmd.data.setViewport;
      pass.SetViewport(vp.left, vp.top, vp.width, vp.height, vp.znear, vp.zfar);
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
  OPTICK_EVENT();
  if (ref == g_currentPipeline) {
    return true;
  }
  std::lock_guard guard{g_pipelineMutex};
  const auto it = g_pipelines.find(ref);
  if (it == g_pipelines.end()) {
    return false;
  }
  pass.SetPipeline(it->second);
  g_currentPipeline = ref;
  return true;
}

static inline Range push(ByteBuffer& target, const uint8_t* data, size_t length, size_t alignment) {
  OPTICK_EVENT();
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
  return {static_cast<uint32_t>(begin), static_cast<uint32_t>(length + padding)};
}
static inline Range map(ByteBuffer& target, size_t length, size_t alignment) {
  OPTICK_EVENT();
  size_t padding = 0;
  if (alignment != 0) {
    padding = alignment - length % alignment;
  }
  if (length == 0) {
    length = alignment;
  }
  auto begin = target.size();
  target.append_zeroes(length + padding);
  return {static_cast<uint32_t>(begin), static_cast<uint32_t>(length + padding)};
}
Range push_verts(const uint8_t* data, size_t length) {
  OPTICK_EVENT();
  return push(g_verts, data, length, 0 /* TODO? */);
}
Range push_indices(const uint8_t* data, size_t length) {
  OPTICK_EVENT();
  return push(g_indices, data, length, 0 /* TODO? */);
}
Range push_uniform(const uint8_t* data, size_t length) {
  OPTICK_EVENT();
  wgpu::SupportedLimits limits;
  g_device.GetLimits(&limits);
  return push(g_uniforms, data, length, limits.limits.minUniformBufferOffsetAlignment);
}
Range push_storage(const uint8_t* data, size_t length) {
  OPTICK_EVENT();
  wgpu::SupportedLimits limits;
  g_device.GetLimits(&limits);
  return push(g_storage, data, length, limits.limits.minStorageBufferOffsetAlignment);
}
Range push_static_storage(const uint8_t* data, size_t length) {
  OPTICK_EVENT();
  wgpu::SupportedLimits limits;
  g_device.GetLimits(&limits);
  auto range = push(g_staticStorage, data, length, limits.limits.minStorageBufferOffsetAlignment);
  range.isStatic = true;
  return range;
}
std::pair<ByteBuffer, Range> map_verts(size_t length) {
  OPTICK_EVENT();
  const auto range = map(g_verts, length, 0 /* TODO? */);
  return {ByteBuffer{g_verts.data() + range.offset, range.size}, range};
}
std::pair<ByteBuffer, Range> map_indices(size_t length) {
  OPTICK_EVENT();
  const auto range = map(g_indices, length, 0 /* TODO? */);
  return {ByteBuffer{g_indices.data() + range.offset, range.size}, range};
}
std::pair<ByteBuffer, Range> map_uniform(size_t length) {
  OPTICK_EVENT();
  wgpu::SupportedLimits limits;
  g_device.GetLimits(&limits);
  const auto range = map(g_uniforms, length, limits.limits.minUniformBufferOffsetAlignment);
  return {ByteBuffer{g_uniforms.data() + range.offset, range.size}, range};
}
std::pair<ByteBuffer, Range> map_storage(size_t length) {
  OPTICK_EVENT();
  wgpu::SupportedLimits limits;
  g_device.GetLimits(&limits);
  const auto range = map(g_storage, length, limits.limits.minStorageBufferOffsetAlignment);
  return {ByteBuffer{g_storage.data() + range.offset, range.size}, range};
}

BindGroupRef bind_group_ref(const wgpu::BindGroupDescriptor& descriptor) {
  OPTICK_EVENT();
  const auto id = xxh3_hash(descriptor);
  if (!g_cachedBindGroups.contains(id)) {
    g_cachedBindGroups.try_emplace(id, g_device.CreateBindGroup(&descriptor));
  }
  return id;
}
const wgpu::BindGroup& find_bind_group(BindGroupRef id) {
  OPTICK_EVENT();
  const auto it = g_cachedBindGroups.find(id);
  if (it == g_cachedBindGroups.end()) {
    Log.report(logvisor::Fatal, FMT_STRING("get_bind_group: failed to locate {}"), id);
    unreachable();
  }
  return it->second;
}

const wgpu::Sampler& sampler_ref(const wgpu::SamplerDescriptor& descriptor) {
  OPTICK_EVENT();
  const auto id = xxh3_hash(descriptor);
  auto it = g_cachedSamplers.find(id);
  if (it == g_cachedSamplers.end()) {
    it = g_cachedSamplers.try_emplace(id, g_device.CreateSampler(&descriptor)).first;
  }
  return it->second;
}

uint32_t align_uniform(uint32_t value) {
  OPTICK_EVENT();
  wgpu::SupportedLimits limits;
  g_device.GetLimits(&limits); // TODO cache
  const auto uniform_alignment = limits.limits.minUniformBufferOffsetAlignment;
  return ALIGN(value, uniform_alignment);
}
} // namespace aurora::gfx
