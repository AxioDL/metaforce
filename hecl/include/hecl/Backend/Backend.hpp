#pragma once

#include "hecl/Frontend.hpp"

namespace hecl::Backend
{

using IR = Frontend::IR;
using Diagnostics = Frontend::Diagnostics;
using SourceLocation = Frontend::SourceLocation;
using ArithmeticOp = IR::Instruction::ArithmeticOpType;

enum class TexGenSrc : uint8_t
{
    Position,
    Normal,
    UV
};

enum class BlendFactor : uint8_t
{
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

enum class ZTest : uint8_t
{
    None,
    LEqual,
    Greater,
    Equal,
    GEqual,
    Original = 0xff
};

enum class CullMode : uint8_t
{
    None,
    Backface,
    Frontface,
    Original = 0xff
};

struct TextureInfo
{
    TexGenSrc src;
    int mapIdx;
    int uvIdx;
    int mtxIdx;
    bool normalize;
};

enum class ReflectionType
{
    None,
    Simple,
    Indirect
};

class IBackend
{
public:
    virtual void reset(const IR& ir, Diagnostics& diag)=0;
};

/**
 * @brief Hash subclass for identifying shaders and their metadata
 */
class ShaderTag : public Hash
{
    union
    {
        uint64_t m_meta = 0;
        struct
        {
            uint8_t m_colorCount;
            uint8_t m_uvCount;
            uint8_t m_weightCount;
            uint8_t m_skinSlotCount;
            uint8_t m_primitiveType;
            uint8_t m_reflectionType;
            bool m_depthTest:1;
            bool m_depthWrite:1;
            bool m_backfaceCulling:1;
        };
    };
public:
    ShaderTag() = default;
    ShaderTag(std::string_view source, uint8_t c, uint8_t u, uint8_t w, uint8_t s, boo::Primitive pt,
              Backend::ReflectionType reflectionType, bool depthTest, bool depthWrite, bool backfaceCulling)
        : Hash(source), m_colorCount(c), m_uvCount(u), m_weightCount(w), m_skinSlotCount(s),
          m_primitiveType(uint8_t(pt)), m_reflectionType(uint8_t(reflectionType)),
          m_depthTest(depthTest), m_depthWrite(depthWrite), m_backfaceCulling(backfaceCulling)
    {hash ^= m_meta;}
    ShaderTag(const hecl::Frontend::IR& ir, uint8_t c, uint8_t u, uint8_t w, uint8_t s, boo::Primitive pt,
              Backend::ReflectionType reflectionType, bool depthTest, bool depthWrite, bool backfaceCulling)
        : Hash(ir.m_hash), m_colorCount(c), m_uvCount(u), m_weightCount(w), m_skinSlotCount(s),
          m_primitiveType(uint8_t(pt)), m_reflectionType(uint8_t(reflectionType)),
          m_depthTest(depthTest), m_depthWrite(depthWrite), m_backfaceCulling(backfaceCulling)
    {hash ^= m_meta;}
    ShaderTag(uint64_t hashin, uint8_t c, uint8_t u, uint8_t w, uint8_t s, boo::Primitive pt,
              Backend::ReflectionType reflectionType, bool depthTest, bool depthWrite, bool backfaceCulling)
        : Hash(hashin), m_colorCount(c), m_uvCount(u), m_weightCount(w), m_skinSlotCount(s),
          m_primitiveType(uint8_t(pt)), m_reflectionType(uint8_t(reflectionType)),
          m_depthTest(depthTest), m_depthWrite(depthWrite), m_backfaceCulling(backfaceCulling)
    {hash ^= m_meta;}
    ShaderTag(uint64_t comphashin, uint64_t meta)
        : Hash(comphashin), m_meta(meta) {}
    ShaderTag(const ShaderTag& other) : Hash(other), m_meta(other.m_meta) {}
    uint8_t getColorCount() const {return m_colorCount;}
    uint8_t getUvCount() const {return m_uvCount;}
    uint8_t getWeightCount() const {return m_weightCount;}
    uint8_t getSkinSlotCount() const {return m_skinSlotCount;}
    boo::Primitive getPrimType() const {return boo::Primitive(m_primitiveType);}
    Backend::ReflectionType getReflectionType() const {return Backend::ReflectionType(m_reflectionType);}
    bool getDepthTest() const {return m_depthTest;}
    bool getDepthWrite() const {return m_depthWrite;}
    bool getBackfaceCulling() const {return m_backfaceCulling;}
    uint64_t getMetaData() const {return m_meta;}

    std::vector<boo::VertexElementDescriptor> vertexFormat() const
    {
        std::vector<boo::VertexElementDescriptor> ret;
        size_t elemCount = 2 + m_colorCount + m_uvCount + m_weightCount;
        ret.resize(elemCount);

        ret[0].semantic = boo::VertexSemantic::Position3;
        ret[1].semantic = boo::VertexSemantic::Normal3;
        size_t e = 2;

        for (size_t i=0 ; i<m_colorCount ; ++i, ++e)
        {
            ret[e].semantic = boo::VertexSemantic::ColorUNorm;
            ret[e].semanticIdx = i;
        }

        for (size_t i=0 ; i<m_uvCount ; ++i, ++e)
        {
            ret[e].semantic = boo::VertexSemantic::UV2;
            ret[e].semanticIdx = i;
        }

        for (size_t i=0 ; i<m_weightCount ; ++i, ++e)
        {
            ret[e].semantic = boo::VertexSemantic::Weight;
            ret[e].semanticIdx = i;
        }

        return ret;
    }
};

struct Function
{
    std::string_view m_source;
    std::string_view m_entry;
    Function() = default;
    Function(std::string_view source, std::string_view entry)
    : m_source(source), m_entry(entry) {}
};

struct ExtensionSlot
{
    Function lighting;
    Function post;
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

    ExtensionSlot(size_t texCount = 0,
                  const Backend::TextureInfo* texs = nullptr,
                  Backend::BlendFactor srcFactor = Backend::BlendFactor::Original,
                  Backend::BlendFactor dstFactor = Backend::BlendFactor::Original,
                  Backend::ZTest depthTest = Backend::ZTest::Original,
                  Backend::CullMode cullMode = Backend::CullMode::Backface,
                  bool noDepthWrite = false,
                  bool noColorWrite = false,
                  bool noAlphaWrite = false,
                  bool noAlphaOverwrite = false,
                  bool noReflection = false)
    : texCount(texCount), texs(texs), srcFactor(srcFactor), dstFactor(dstFactor), depthTest(depthTest),
      cullMode(cullMode), noDepthWrite(noDepthWrite), noColorWrite(noColorWrite), noAlphaWrite(noAlphaWrite),
      noAlphaOverwrite(noAlphaOverwrite), noReflection(noReflection) {}

    mutable uint64_t m_hash = 0;
    void calculateHash() const
    {
        XXH64_state_t st;
        XXH64_reset(&st, 0);
        if (!lighting.m_source.empty())
            XXH64_update(&st, lighting.m_source.data(), lighting.m_source.size());
        if (!post.m_source.empty())
            XXH64_update(&st, post.m_source.data(), post.m_source.size());
        for (size_t i = 0; i < texCount; ++i)
        {
            const Backend::TextureInfo& tinfo = texs[i];
            XXH64_update(&st, &tinfo, sizeof(tinfo));
        }
        XXH64_update(&st, &srcFactor, offsetof(ExtensionSlot, m_hash) - offsetof(ExtensionSlot, srcFactor));
        m_hash = XXH64_digest(&st);
    }
    uint64_t hash() const
    {
        if (m_hash == 0)
            calculateHash();
        return m_hash;
    }
};

}

namespace std
{
template <> struct hash<hecl::Backend::ShaderTag>
{
    size_t operator()(const hecl::Backend::ShaderTag& val) const noexcept
    {return val.valSizeT();}
};
}

