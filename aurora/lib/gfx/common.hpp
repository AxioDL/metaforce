#pragma once
#include <aurora/gfx.hpp>

#include <utility>

#include <dawn/webgpu_cpp.h>
#include <xxhash_impl.h>

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
  ByteBuffer() noexcept = default;
  explicit ByteBuffer(size_t size) noexcept
  : m_data(static_cast<uint8_t*>(calloc(1, size))), m_length(size), m_capacity(size) {}
  ~ByteBuffer() noexcept {
    if (m_data != nullptr) {
      free(m_data);
    }
  }
  ByteBuffer(ByteBuffer&& rhs) noexcept : m_data(rhs.m_data), m_length(rhs.m_length), m_capacity(rhs.m_capacity) {
    rhs.m_data = nullptr;
    rhs.m_length = 0;
    rhs.m_capacity = 0;
  }
  ByteBuffer& operator=(ByteBuffer&& rhs) noexcept {
    if (m_data != nullptr) {
      free(m_data);
    }
    m_data = rhs.m_data;
    m_length = rhs.m_length;
    m_capacity = rhs.m_capacity;
    rhs.m_data = nullptr;
    rhs.m_length = 0;
    rhs.m_capacity = 0;
    return *this;
  }
  ByteBuffer(ByteBuffer const&) = delete;
  ByteBuffer& operator=(ByteBuffer const&) = delete;

  [[nodiscard]] uint8_t* data() noexcept { return m_data; }
  [[nodiscard]] const uint8_t* data() const noexcept { return m_data; }
  [[nodiscard]] size_t size() const noexcept { return m_length; }
  [[nodiscard]] bool empty() const noexcept { return m_length == 0; }

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
// GX state
extern metaforce::ERglCullMode g_cullMode;
extern metaforce::ERglBlendMode g_blendMode;
extern metaforce::ERglBlendFactor g_blendFacSrc;
extern metaforce::ERglBlendFactor g_blendFacDst;
extern metaforce::ERglLogicOp g_blendOp;
extern bool g_depthCompare;
extern bool g_depthUpdate;
extern metaforce::ERglEnum g_depthFunc;
extern std::array<zeus::CColor, 4> g_colorRegs;
extern bool g_alphaUpdate;
extern std::optional<float> g_dstAlpha;
extern zeus::CColor g_clearColor;

extern wgpu::Buffer g_vertexBuffer;
extern wgpu::Buffer g_uniformBuffer;
extern wgpu::Buffer g_indexBuffer;

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
using Range = std::pair<uint32_t, uint32_t>;

enum class ShaderType {
  Aabb,
  ColoredQuad,
  TexturedQuad,
  MoviePlayer,
  Stream,
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

static inline Mat4x4<float> get_combined_matrix() { return g_proj * g_mv; }
} // namespace aurora::gfx
