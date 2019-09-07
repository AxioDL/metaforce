#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <vector>

#include <boo/graphicsdev/IGraphicsDataFactory.hpp>

#include "hecl.hpp"
#include "../extern/boo/xxhash/xxhash.h"

namespace hecl::Backend {
struct ExtensionSlot;
using namespace std::literals;

enum class TexCoordSource : uint8_t {
  Invalid = 0xff,
  Position = 0,
  Normal = 1,
  Tex0 = 2,
  Tex1 = 3,
  Tex2 = 4,
  Tex3 = 5,
  Tex4 = 6,
  Tex5 = 7,
  Tex6 = 8,
  Tex7 = 9,
};

enum class BlendFactor : uint8_t {
  Zero,
  One,
  SrcColor,
  InvSrcColor,
  DstColor,
  InvDstColor,
  SrcAlpha,
  InvSrcAlpha,
  DstAlpha,
  InvDstAlpha,
  SrcColor1,
  InvSrcColor1,
  Original = 0xff
};

constexpr std::string_view BlendFactorToDefine(BlendFactor factor, BlendFactor defaultFactor) {
  switch (factor) {
  case BlendFactor::Zero:
    return "ZERO"sv;
  case BlendFactor::One:
    return "ONE"sv;
  case BlendFactor::SrcColor:
    return "SRCCOLOR"sv;
  case BlendFactor::InvSrcColor:
    return "INVSRCCOLOR"sv;
  case BlendFactor::DstColor:
    return "DSTCOLOR"sv;
  case BlendFactor::InvDstColor:
    return "INVDSTCOLOR"sv;
  case BlendFactor::SrcAlpha:
    return "SRCALPHA"sv;
  case BlendFactor::InvSrcAlpha:
    return "INVSRCALPHA"sv;
  case BlendFactor::DstAlpha:
    return "DSTALPHA"sv;
  case BlendFactor::InvDstAlpha:
    return "INVDSTALPHA"sv;
  case BlendFactor::SrcColor1:
    return "SRCCOLOR1"sv;
  case BlendFactor::InvSrcColor1:
    return "INVSRCCOLOR1"sv;
  default:
    return BlendFactorToDefine(defaultFactor, BlendFactor::Zero);
  }
}

enum class ZTest : uint8_t { None, LEqual, Greater, Equal, GEqual, Original = 0xff };

enum class CullMode : uint8_t { None, Backface, Frontface, Original = 0xff };

struct TextureInfo {
  TexCoordSource src;
  uint8_t mtxIdx;
  bool normalize;
};

enum class ReflectionType { None, Simple, Indirect };

/**
 * @brief Hash subclass for identifying shaders and their metadata
 */
class ShaderTag : public Hash {
  union {
    uint64_t m_meta = 0;
    struct {
      uint8_t m_colorCount;
      uint8_t m_uvCount;
      uint8_t m_weightCount;
      uint8_t m_skinSlotCount;
      uint8_t m_primitiveType;
      uint8_t m_reflectionType;
      bool m_depthTest : 1;
      bool m_depthWrite : 1;
      bool m_backfaceCulling : 1;
      bool m_alphaTest : 1;
    };
  };

public:
  ShaderTag() = default;
  ShaderTag(std::string_view source, uint8_t c, uint8_t u, uint8_t w, uint8_t s, boo::Primitive pt,
            Backend::ReflectionType reflectionType, bool depthTest, bool depthWrite, bool backfaceCulling,
            bool alphaTest)
  : Hash(source) {
    m_colorCount = c;
    m_uvCount = u;
    m_weightCount = w;
    m_skinSlotCount = s;
    m_primitiveType = uint8_t(pt);
    m_reflectionType = uint8_t(reflectionType);
    m_depthTest = depthTest;
    m_depthWrite = depthWrite;
    m_backfaceCulling = backfaceCulling;
    m_alphaTest = alphaTest;
    hash ^= m_meta;
  }
  ShaderTag(uint64_t hashin, uint8_t c, uint8_t u, uint8_t w, uint8_t s, boo::Primitive pt,
            Backend::ReflectionType reflectionType, bool depthTest, bool depthWrite, bool backfaceCulling,
            bool alphaTest)
  : Hash(hashin) {
    m_colorCount = c;
    m_uvCount = u;
    m_weightCount = w;
    m_skinSlotCount = s;
    m_primitiveType = uint8_t(pt);
    m_reflectionType = uint8_t(reflectionType);
    m_depthTest = depthTest;
    m_depthWrite = depthWrite;
    m_backfaceCulling = backfaceCulling;
    m_alphaTest = alphaTest;
    hash ^= m_meta;
  }
  ShaderTag(uint64_t comphashin, uint64_t meta) : Hash(comphashin), m_meta(meta) {}
  ShaderTag(const ShaderTag& other) : Hash(other), m_meta(other.m_meta) {}
  uint8_t getColorCount() const { return m_colorCount; }
  uint8_t getUvCount() const { return m_uvCount; }
  uint8_t getWeightCount() const { return m_weightCount; }
  uint8_t getSkinSlotCount() const { return m_skinSlotCount; }
  boo::Primitive getPrimType() const { return boo::Primitive(m_primitiveType); }
  Backend::ReflectionType getReflectionType() const { return Backend::ReflectionType(m_reflectionType); }
  bool getDepthTest() const { return m_depthTest; }
  bool getDepthWrite() const { return m_depthWrite; }
  bool getBackfaceCulling() const { return m_backfaceCulling; }
  bool getAlphaTest() const { return m_alphaTest; }
  uint64_t getMetaData() const { return m_meta; }

  std::vector<boo::VertexElementDescriptor> vertexFormat() const {
    std::vector<boo::VertexElementDescriptor> ret;
    size_t elemCount = 2 + m_colorCount + m_uvCount + m_weightCount;
    ret.resize(elemCount);

    ret[0].semantic = boo::VertexSemantic::Position3;
    ret[1].semantic = boo::VertexSemantic::Normal3;
    size_t e = 2;

    for (size_t i = 0; i < m_colorCount; ++i, ++e) {
      ret[e].semantic = boo::VertexSemantic::ColorUNorm;
      ret[e].semanticIdx = i;
    }

    for (size_t i = 0; i < m_uvCount; ++i, ++e) {
      ret[e].semantic = boo::VertexSemantic::UV2;
      ret[e].semanticIdx = i;
    }

    for (size_t i = 0; i < m_weightCount; ++i, ++e) {
      ret[e].semantic = boo::VertexSemantic::Weight;
      ret[e].semanticIdx = i;
    }

    return ret;
  }

  boo::AdditionalPipelineInfo additionalInfo(const ExtensionSlot& ext,
                                             std::pair<BlendFactor, BlendFactor> blendFactors) const;
};

struct Function {
  std::string_view m_source;
  std::string_view m_entry;
  Function() = default;
  Function(std::string_view source, std::string_view entry) : m_source(source), m_entry(entry) {}
};

struct ExtensionSlot {
  const char* shaderMacro;
  size_t texCount = 0;
  const Backend::TextureInfo* texs = nullptr;
  Backend::BlendFactor srcFactor = Backend::BlendFactor::Original;
  Backend::BlendFactor dstFactor = Backend::BlendFactor::Original;
  Backend::ZTest depthTest = Backend::ZTest::Original;
  Backend::CullMode cullMode = Backend::CullMode::Backface;
  bool noDepthWrite = false;
  bool noColorWrite = false;
  bool noAlphaWrite = false;
  bool noAlphaOverwrite = false;
  bool noReflection = false;
  bool forceAlphaTest = false;
  bool diffuseOnly = false;

  ExtensionSlot(size_t texCount = 0,
                const Backend::TextureInfo* texs = nullptr,
                Backend::BlendFactor srcFactor = Backend::BlendFactor::Original,
                Backend::BlendFactor dstFactor = Backend::BlendFactor::Original,
                Backend::ZTest depthTest = Backend::ZTest::Original,
                Backend::CullMode cullMode = Backend::CullMode::Backface, bool noDepthWrite = false,
                bool noColorWrite = false, bool noAlphaWrite = false, bool noAlphaOverwrite = false,
                bool noReflection = false, bool forceAlphaTest = false, bool diffuseOnly = false)
  : texCount(texCount)
  , texs(texs)
  , srcFactor(srcFactor)
  , dstFactor(dstFactor)
  , depthTest(depthTest)
  , cullMode(cullMode)
  , noDepthWrite(noDepthWrite)
  , noColorWrite(noColorWrite)
  , noAlphaWrite(noAlphaWrite)
  , noAlphaOverwrite(noAlphaOverwrite)
  , noReflection(noReflection)
  , forceAlphaTest(forceAlphaTest)
  , diffuseOnly(diffuseOnly) {}

  mutable uint64_t m_hash = 0;
  void calculateHash() const {
    XXH64_state_t st;
    XXH64_reset(&st, 0);
    XXH64_update(&st, shaderMacro, strlen(shaderMacro));
    for (size_t i = 0; i < texCount; ++i) {
      const Backend::TextureInfo& tinfo = texs[i];
      XXH64_update(&st, &tinfo, sizeof(tinfo));
    }
    XXH64_update(&st, &srcFactor, offsetof(ExtensionSlot, m_hash) - offsetof(ExtensionSlot, srcFactor));
    m_hash = XXH64_digest(&st);
  }
  uint64_t hash() const {
    if (m_hash == 0)
      calculateHash();
    return m_hash;
  }
};

inline boo::AdditionalPipelineInfo ShaderTag::additionalInfo(const ExtensionSlot& ext,
                                                             std::pair<BlendFactor, BlendFactor> blendFactors) const {
  boo::ZTest zTest;
  switch (ext.depthTest) {
  case hecl::Backend::ZTest::Original:
  default:
    zTest = getDepthTest() ? boo::ZTest::LEqual : boo::ZTest::None;
    break;
  case hecl::Backend::ZTest::None:
    zTest = boo::ZTest::None;
    break;
  case hecl::Backend::ZTest::LEqual:
    zTest = boo::ZTest::LEqual;
    break;
  case hecl::Backend::ZTest::Greater:
    zTest = boo::ZTest::Greater;
    break;
  case hecl::Backend::ZTest::Equal:
    zTest = boo::ZTest::Equal;
    break;
  case hecl::Backend::ZTest::GEqual:
    zTest = boo::ZTest::GEqual;
    break;
  }

  BlendFactor srcFactor = m_alphaTest ? BlendFactor::One : blendFactors.first;
  BlendFactor dstFactor = m_alphaTest ? BlendFactor::Zero : blendFactors.second;
  return {boo::BlendFactor((ext.srcFactor == BlendFactor::Original) ? srcFactor : ext.srcFactor),
          boo::BlendFactor((ext.dstFactor == BlendFactor::Original) ? dstFactor : ext.dstFactor),
          getPrimType(),
          zTest,
          ext.noDepthWrite ? false : getDepthWrite(),
          !ext.noColorWrite,
          !ext.noAlphaWrite,
          (ext.cullMode == hecl::Backend::CullMode::Original)
              ? (getBackfaceCulling() ? boo::CullMode::Backface : boo::CullMode::None)
              : boo::CullMode(ext.cullMode),
          !ext.noAlphaOverwrite};
}

} // namespace hecl::Backend

namespace std {
template <>
struct hash<hecl::Backend::ShaderTag> {
  size_t operator()(const hecl::Backend::ShaderTag& val) const noexcept { return val.valSizeT(); }
};
} // namespace std
