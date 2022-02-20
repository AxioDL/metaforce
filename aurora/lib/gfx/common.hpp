#pragma once
#include <aurora/gfx.hpp>

#include <utility>

#include <dawn/webgpu_cpp.h>
#include <xxhash.h>
#if defined(__x86_64__) || defined(__i386__) || defined(_M_IX86) || defined(_M_X64)
#include <xxh_x86dispatch.h>
#endif

#ifndef ALIGN
#define ALIGN(x, a) (((x) + ((a)-1)) & ~((a)-1))
#endif

namespace aurora {
static inline XXH64_hash_t xxh3_hash(const void* input, size_t len, XXH64_hash_t seed = 0) {
  return XXH3_64bits_withSeed(input, len, seed);
}
template <typename T>
static inline XXH64_hash_t xxh3_hash(const T& input, XXH64_hash_t seed = 0) {
  return xxh3_hash(&input, sizeof(T), seed);
}

class ByteBuffer {
public:
  ByteBuffer() = default;
  explicit ByteBuffer(size_t capacity) : m_data(static_cast<uint8_t*>(calloc(1, capacity))), m_capacity(capacity) {}

  ~ByteBuffer() {
    if (m_data != nullptr) {
      free(m_data);
    }
  }

  uint8_t* data() { return m_data; }
  const uint8_t* data() const { return m_data; }
  size_t size() const { return m_length; }

  void append(const void* data, size_t size) {
    resize(m_length + size);
    memcpy(m_data + m_length, data, size);
    m_length += size;
  }

  void append_zeroes(size_t size) {
    resize(m_length + size);
    memset(m_data + m_length, 0, size);
    m_length += size;
  }

  void clear() {
    if (m_data != nullptr) {
      free(m_data);
    }
    m_data = nullptr;
    m_length = 0;
    m_capacity = 0;
  }

private:
  uint8_t* m_data = nullptr;
  size_t m_length = 0;
  size_t m_capacity = 0;

  void resize(size_t size) {
    if (size == 0) {
      clear();
    } else if (m_data == nullptr) {
      m_data = static_cast<uint8_t*>(malloc(size));
    } else if (size > m_capacity) {
      m_data = static_cast<uint8_t*>(realloc(m_data, size));
    } else {
      return;
    }
    m_capacity = size;
  }
};

template <typename T>
struct Vec2 {
  T x{};
  T y{};

  constexpr Vec2(T x, T y) : x(x), y(y) {}
  constexpr Vec2(const zeus::CVector2f& vec) : x(vec.x()), y(vec.y()) {}
};
template <typename T>
struct Vec3 {
  T x{};
  T y{};
  T z{};

  constexpr Vec3(T x, T y, T z) : x(x), y(y), z(z) {}
  constexpr Vec3(const zeus::CVector3f& vec) : x(vec.x()), y(vec.y()), z(vec.z()) {}
};
template <typename T>
struct Vec4 {
  T x{};
  T y{};
  T z{};
  T w{};

  constexpr Vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}
  constexpr Vec4(const zeus::CVector4f& vec) : x(vec.x()), y(vec.y()), z(vec.z()), w(vec.w()) {}
  constexpr Vec4(const zeus::CColor& color) : x(color.r()), y(color.g()), z(color.b()), w(color.a()) {}
};
template <typename T>
struct Mat4x4 {
  Vec4<T> m0{};
  Vec4<T> m1{};
  Vec4<T> m2{};
  Vec4<T> m3{};

  constexpr Mat4x4(const Vec4<T>& m0, const Vec4<T>& m1, const Vec4<T>& m2, const Vec4<T>& m3)
  : m0(m0), m1(m1), m2(m2), m3(m3) {}
  constexpr Mat4x4(const zeus::CMatrix4f& m) : m0(m[0]), m1(m[1]), m2(m[2]), m3(m[3]) {}
};
constexpr Mat4x4<float> Mat4x4_Identity{
    Vec4<float>{1.f, 0.f, 0.f, 0.f},
    Vec4<float>{0.f, 1.f, 0.f, 0.f},
    Vec4<float>{0.f, 0.f, 1.f, 0.f},
    Vec4<float>{0.f, 0.f, 0.f, 1.f},
};
} // namespace aurora

namespace aurora::gfx {
extern zeus::CMatrix4f g_mv;
extern zeus::CMatrix4f g_mvInv;
extern zeus::CMatrix4f g_proj;
extern metaforce::CFogState g_fogState;

extern wgpu::Buffer g_vertexBuffer;
extern wgpu::Buffer g_uniformBuffer;
extern wgpu::Buffer g_indexBuffer;

struct TextureRef {
  wgpu::Texture texture;
  wgpu::TextureView view;
  wgpu::Extent3D size;
  wgpu::TextureFormat format;

  TextureRef(wgpu::Texture&& texture, wgpu::TextureView&& view, wgpu::Extent3D size, wgpu::TextureFormat format)
  : texture(std::move(texture)), view(std::move(view)), size(size), format(format) {}
};

using PipelineRef = uint64_t;
using BindGroupRef = uint64_t;
using Range = std::pair<uint32_t, uint32_t>;

enum class ShaderType {
  Aabb,
  ColoredQuad,
  TexturedQuad,
  MoviePlayer,
};

void initialize();
void shutdown();

void render(const wgpu::RenderPassEncoder& pass);

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

template <typename PipelineConfig>
PipelineRef pipeline_ref(PipelineConfig config);
bool bind_pipeline(PipelineRef ref, const wgpu::RenderPassEncoder& pass);

BindGroupRef bind_group_ref(const wgpu::BindGroupDescriptor& descriptor);
const wgpu::BindGroup& find_bind_group(BindGroupRef id);

static inline zeus::CMatrix4f get_combined_matrix() { return g_proj * g_mv; }
} // namespace aurora::gfx
