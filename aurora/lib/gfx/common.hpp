#pragma once
#include <aurora/gfx.hpp>

#include <utility>

#include <dawn/webgpu_cpp.h>
#define XXH_INLINE_ALL
#define XXH_STATIC_LINKING_ONLY
#define XXH_IMPLEMENTATION
#include <xxhash.h>
#include <optick.h>

#ifndef ALIGN
#define ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))
#endif

#ifdef __GNUC__
#define PACK(...) __VA_ARGS__  __attribute__((__packed__))
#endif
#ifdef _MSC_VER
#define PACK(...) __pragma(pack(push, 1)); __VA_ARGS__; __pragma(pack(pop))
#endif

namespace aurora {
template <typename T>
static inline void xxh3_update(XXH3_state_t& state, const T& input);
static inline XXH64_hash_t xxh3_hash_s(const void* input, size_t len, XXH64_hash_t seed = 0) {
  return XXH3_64bits_withSeed(input, len, seed);
}
template <typename T>
static inline XXH64_hash_t xxh3_hash(const T& input, XXH64_hash_t seed = 0) {
  OPTICK_EVENT();
  XXH3_state_t state;
  memset(&state, 0, sizeof(XXH3_state_t));
  XXH3_64bits_reset_withSeed(&state, seed);
  xxh3_update(state, input);
  return XXH3_64bits_digest(&state);
}

class ByteBuffer {
public:
  ByteBuffer() noexcept = default;
  explicit ByteBuffer(size_t size) noexcept
  : m_data(static_cast<uint8_t*>(calloc(1, size))), m_length(size), m_capacity(size) {}
  explicit ByteBuffer(uint8_t* data, size_t size) noexcept
  : m_data(data), m_length(0), m_capacity(size), m_owned(false) {}
  ~ByteBuffer() noexcept {
    if (m_data != nullptr && m_owned) {
      free(m_data);
    }
  }
  ByteBuffer(ByteBuffer&& rhs) noexcept
  : m_data(rhs.m_data), m_length(rhs.m_length), m_capacity(rhs.m_capacity), m_owned(rhs.m_owned) {
    rhs.m_data = nullptr;
    rhs.m_length = 0;
    rhs.m_capacity = 0;
    rhs.m_owned = true;
  }
  ByteBuffer& operator=(ByteBuffer&& rhs) noexcept {
    if (m_data != nullptr && m_owned) {
      free(m_data);
    }
    m_data = rhs.m_data;
    m_length = rhs.m_length;
    m_capacity = rhs.m_capacity;
    m_owned = rhs.m_owned;
    rhs.m_data = nullptr;
    rhs.m_length = 0;
    rhs.m_capacity = 0;
    rhs.m_owned = true;
    return *this;
  }
  ByteBuffer(ByteBuffer const&) = delete;
  ByteBuffer& operator=(ByteBuffer const&) = delete;

  [[nodiscard]] uint8_t* data() noexcept { return m_data; }
  [[nodiscard]] const uint8_t* data() const noexcept { return m_data; }
  [[nodiscard]] size_t size() const noexcept { return m_length; }
  [[nodiscard]] bool empty() const noexcept { return m_length == 0; }

  void append(const void* data, size_t size) {
    resize(m_length + size, false);
    memcpy(m_data + m_length, data, size);
    m_length += size;
  }

  void append_zeroes(size_t size) {
    resize(m_length + size, true);
    m_length += size;
  }

  void clear() {
    if (m_data != nullptr && m_owned) {
      free(m_data);
    }
    m_data = nullptr;
    m_length = 0;
    m_capacity = 0;
    m_owned = true;
  }

  void reserve_extra(size_t size) { resize(m_length + size, true); }

private:
  uint8_t* m_data = nullptr;
  size_t m_length = 0;
  size_t m_capacity = 0;
  bool m_owned = true;

  void resize(size_t size, bool zeroed) {
    if (size == 0) {
      clear();
    } else if (m_data == nullptr) {
      if (zeroed) {
        m_data = static_cast<uint8_t*>(calloc(1, size));
      } else {
        m_data = static_cast<uint8_t*>(malloc(size));
      }
      m_owned = true;
    } else if (size > m_capacity) {
      if (!m_owned) {
        abort();
      }
      m_data = static_cast<uint8_t*>(realloc(m_data, size));
      if (zeroed) {
        memset(m_data + m_capacity, 0, size - m_capacity);
      }
    } else {
      return;
    }
    m_capacity = size;
  }
};
} // namespace aurora

namespace aurora::gfx {
extern wgpu::Buffer g_vertexBuffer;
extern wgpu::Buffer g_uniformBuffer;
extern wgpu::Buffer g_indexBuffer;
extern wgpu::Buffer g_storageBuffer;
extern size_t g_staticStorageLastSize;

struct TextureRef {
  wgpu::Texture texture;
  wgpu::TextureView view;
  wgpu::Extent3D size;
  wgpu::TextureFormat format;
  uint32_t mipCount;
  metaforce::ETexelFormat gameFormat;

  TextureRef(wgpu::Texture&& texture, wgpu::TextureView&& view, wgpu::Extent3D size, wgpu::TextureFormat format,
             uint32_t mipCount, metaforce::ETexelFormat gameFormat = metaforce::ETexelFormat::Invalid)
  : texture(std::move(texture))
  , view(std::move(view))
  , size(size)
  , format(format)
  , mipCount(mipCount)
  , gameFormat(gameFormat) {}
};

using BindGroupRef = uint64_t;
using PipelineRef = uint64_t;
using SamplerRef = uint64_t;
using ShaderRef = uint64_t;
struct Range {
  uint32_t offset;
  uint32_t size;
  bool isStatic;
};
static inline uint32_t storage_offset(Range range) {
  return range.isStatic ? range.offset : range.offset + g_staticStorageLastSize;
}

enum class ShaderType {
  Aabb,
  ColoredQuad,
  TexturedQuad,
  MoviePlayer,
  Stream,
  Model,
};

void initialize();
void shutdown();

void begin_frame();
void end_frame(const wgpu::CommandEncoder& cmd);
void render(const wgpu::RenderPassEncoder& pass);
void map_staging_buffer();

Range push_verts(const uint8_t* data, size_t length);
template <typename T>
static inline Range push_verts(ArrayRef<T> data) {
  return push_verts(reinterpret_cast<const uint8_t*>(data.data()), data.size() * sizeof(T));
}
Range push_indices(const uint8_t* data, size_t length);
template <typename T>
static inline Range push_indices(ArrayRef<T> data) {
  return push_indices(reinterpret_cast<const uint8_t*>(data.data()), data.size() * sizeof(T));
}
Range push_uniform(const uint8_t* data, size_t length);
template <typename T>
static inline Range push_uniform(const T& data) {
  return push_uniform(reinterpret_cast<const uint8_t*>(&data), sizeof(T));
}
Range push_storage(const uint8_t* data, size_t length);
template <typename T>
static inline Range push_storage(ArrayRef<T> data) {
  return push_storage(reinterpret_cast<const uint8_t*>(data.data()), data.size() * sizeof(T));
}
template <typename T>
static inline Range push_storage(const T& data) {
  return push_storage(reinterpret_cast<const uint8_t*>(&data), sizeof(T));
}
Range push_static_storage(const uint8_t* data, size_t length);
template <typename T>
static inline Range push_static_storage(ArrayRef<T> data) {
  return push_static_storage(reinterpret_cast<const uint8_t*>(data.data()), data.size() * sizeof(T));
}
template <typename T>
static inline Range push_static_storage(const T& data) {
  return push_static_storage(reinterpret_cast<const uint8_t*>(&data), sizeof(T));
}
std::pair<ByteBuffer, Range> map_verts(size_t length);
std::pair<ByteBuffer, Range> map_indices(size_t length);
std::pair<ByteBuffer, Range> map_uniform(size_t length);
std::pair<ByteBuffer, Range> map_storage(size_t length);

template <typename State>
const State& get_state();
template <typename DrawData>
void push_draw_command(DrawData data);

template <typename PipelineConfig>
PipelineRef pipeline_ref(PipelineConfig config);
bool bind_pipeline(PipelineRef ref, const wgpu::RenderPassEncoder& pass);

BindGroupRef bind_group_ref(const wgpu::BindGroupDescriptor& descriptor);
const wgpu::BindGroup& find_bind_group(BindGroupRef id);

const wgpu::Sampler& sampler_ref(const wgpu::SamplerDescriptor& descriptor);

uint32_t align_uniform(uint32_t value);
} // namespace aurora::gfx
