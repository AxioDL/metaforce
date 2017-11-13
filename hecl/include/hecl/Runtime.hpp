#ifndef HECLRUNTIME_HPP
#define HECLRUNTIME_HPP

#include "hecl.hpp"
#include "Frontend.hpp"
#include "Backend/Backend.hpp"
#include <boo/graphicsdev/IGraphicsDataFactory.hpp>
#include <athena/DNA.hpp>
#include <athena/FileReader.hpp>
#include <unordered_map>

namespace hecl
{
struct HMDLMeta;

namespace Runtime
{

/**
 * @brief Per-platform file store resolution
 */
class FileStoreManager
{
    SystemString m_domain;
    SystemString m_storeRoot;
public:
    FileStoreManager(SystemStringView domain);
    SystemStringView getDomain() const {return m_domain;}
    /**
     * @brief Returns the full path to the file store, including domain
     * @return Full path to store e.g /home/foo/.hecl/bar
     */
    SystemStringView getStoreRoot() const {return m_storeRoot;}
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
            uint8_t m_texMtxCount;
            uint8_t m_primitiveType;
            uint8_t m_reflectionType;
            bool m_depthTest:1;
            bool m_depthWrite:1;
            bool m_backfaceCulling:1;
        };
    };
public:
    ShaderTag() = default;
    ShaderTag(std::string_view source, uint8_t c, uint8_t u, uint8_t w, uint8_t s, uint8_t t, boo::Primitive pt,
              Backend::ReflectionType reflectionType, bool depthTest, bool depthWrite, bool backfaceCulling)
    : Hash(source), m_colorCount(c), m_uvCount(u), m_weightCount(w), m_skinSlotCount(s), m_texMtxCount(t),
      m_primitiveType(uint8_t(pt)), m_reflectionType(uint8_t(reflectionType)),
      m_depthTest(depthTest), m_depthWrite(depthWrite), m_backfaceCulling(backfaceCulling)
    {hash ^= m_meta;}
    ShaderTag(const hecl::Frontend::IR& ir, uint8_t c, uint8_t u, uint8_t w, uint8_t s, uint8_t t, boo::Primitive pt,
              Backend::ReflectionType reflectionType, bool depthTest, bool depthWrite, bool backfaceCulling)
    : Hash(ir.m_hash), m_colorCount(c), m_uvCount(u), m_weightCount(w), m_skinSlotCount(s), m_texMtxCount(t),
      m_primitiveType(uint8_t(pt)), m_reflectionType(uint8_t(reflectionType)),
      m_depthTest(depthTest), m_depthWrite(depthWrite), m_backfaceCulling(backfaceCulling)
    {hash ^= m_meta;}
    ShaderTag(uint64_t hashin, uint8_t c, uint8_t u, uint8_t w, uint8_t s, uint8_t t, boo::Primitive pt,
              Backend::ReflectionType reflectionType, bool depthTest, bool depthWrite, bool backfaceCulling)
    : Hash(hashin), m_colorCount(c), m_uvCount(u), m_weightCount(w), m_skinSlotCount(s), m_texMtxCount(t),
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
    uint8_t getTexMtxCount() const {return m_texMtxCount;}
    boo::Primitive getPrimType() const {return boo::Primitive(m_primitiveType);}
    Backend::ReflectionType getReflectionType() const {return Backend::ReflectionType(m_reflectionType);}
    bool getDepthTest() const {return m_depthTest;}
    bool getDepthWrite() const {return m_depthWrite;}
    bool getBackfaceCulling() const {return m_backfaceCulling;}
    uint64_t getMetaData() const {return m_meta;}

    /* For shader constructors that require vertex format up-front (HLSL) */
    boo::ObjToken<boo::IVertexFormat> newVertexFormat(boo::IGraphicsDataFactory::Context& ctx) const;
};

/**
 * @brief Simple binary data and tag container for cache interaction
 */
class ShaderCachedData
{
    friend class ShaderCacheManager;
    ShaderCachedData() = default;
public:
    ShaderCachedData(const ShaderTag& tag, size_t decompSz)
    : m_tag(tag), m_data(new uint8_t[decompSz]), m_sz(decompSz) {}
    ShaderTag m_tag;
    std::unique_ptr<uint8_t[]> m_data;
    size_t m_sz;
    operator bool() const {return m_tag.operator bool();}
};

/**
 * @brief Optional cache extensions allowing the client to specify shader transformations in bulk
 */
class ShaderCacheExtensions
{
    friend class ShaderCacheManager;
    boo::IGraphicsDataFactory::Platform m_plat;
    ShaderCacheExtensions() : m_plat(boo::IGraphicsDataFactory::Platform::Null) {}

    uint64_t hashExtensions() const;
public:
    struct Function
    {
        const char* m_source = nullptr;
        const char* m_entry = nullptr;
        Function() = default;
        Function(const char* source, const char* entry)
        : m_source(source), m_entry(entry) {}
    };

    struct ExtensionSlot
    {
        Function lighting;
        Function post;
        size_t blockCount = 0;
        const char** blockNames = nullptr;
        size_t texCount = 0;
        const Backend::TextureInfo* texs = nullptr;
        Backend::BlendFactor srcFactor = Backend::BlendFactor::Original;
        Backend::BlendFactor dstFactor = Backend::BlendFactor::Original;
        Backend::ZTest depthTest = Backend::ZTest::Original;
        bool frontfaceCull = false;
        bool noDepthWrite = false;
        bool noColorWrite = false;
        bool noAlphaWrite = false;
        bool noReflection = false;
    };
    std::vector<ExtensionSlot> m_extensionSlots;

    ShaderCacheExtensions(boo::IGraphicsDataFactory::Platform plat) : m_plat(plat)
    {
        /* Index-0 has special default-meaning */
        m_extensionSlots.emplace_back();
    }
    operator bool() const {return m_plat != boo::IGraphicsDataFactory::Platform::Null;}

    /* Strings must remain resident!! (intended to be stored static const) */
    unsigned registerExtensionSlot(Function lighting, Function post,
                                   size_t blockCount, const char** blockNames,
                                   size_t texCount, const Backend::TextureInfo* texs,
                                   Backend::BlendFactor srcFactor, Backend::BlendFactor dstFactor,
                                   Backend::ZTest depthTest = Backend::ZTest::Original,
                                   bool frontfaceCull = false,
                                   bool noDepthWrite = false,
                                   bool noColorWrite = false,
                                   bool noAlphaWrite = false,
                                   bool noReflection = false)
    {
        m_extensionSlots.emplace_back();
        ExtensionSlot& slot = m_extensionSlots.back();
        slot.lighting = lighting;
        slot.post = post;
        slot.blockCount = blockCount;
        slot.blockNames = blockNames;
        slot.texCount = texCount;
        slot.texs = texs;
        slot.srcFactor = srcFactor;
        slot.dstFactor = dstFactor;
        slot.depthTest = depthTest;
        slot.frontfaceCull = frontfaceCull;
        slot.noDepthWrite = noDepthWrite;
        slot.noColorWrite = noColorWrite;
        slot.noAlphaWrite = noAlphaWrite;
        slot.noReflection = noReflection;
        return m_extensionSlots.size() - 1;
    }
};

/**
 * @brief Interface for binding HECL backends to the ShaderCacheManager
 */
class IShaderBackendFactory
{
    friend class ShaderCacheManager;
protected:
    unsigned m_rtHint = 1;
    using FReturnExtensionShader = std::function<void(const boo::ObjToken<boo::IShaderPipeline>&)>;
    virtual ShaderCachedData buildShaderFromIR(const ShaderTag& tag,
                                               const hecl::Frontend::IR& ir,
                                               hecl::Frontend::Diagnostics& diag,
                                               boo::IGraphicsDataFactory::Context& ctx,
                                               boo::ObjToken<boo::IShaderPipeline>& objOut)=0;
    virtual boo::ObjToken<boo::IShaderPipeline> buildShaderFromCache(const ShaderCachedData& data,
                                                                     boo::IGraphicsDataFactory::Context& ctx)=0;
    virtual ShaderCachedData buildExtendedShaderFromIR(const ShaderTag& tag,
                                                       const hecl::Frontend::IR& ir,
                                                       hecl::Frontend::Diagnostics& diag,
                                                       const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                                       boo::IGraphicsDataFactory::Context& ctx,
                                                       FReturnExtensionShader returnFunc)=0;
    virtual bool buildExtendedShaderFromCache(const ShaderCachedData& data,
                                              const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                              boo::IGraphicsDataFactory::Context& ctx,
                                              FReturnExtensionShader returnFunc)=0;
public:
    virtual ~IShaderBackendFactory() = default;
};

/**
 * @brief Stores token and pipeline set for sharing with ref-counting
 */
struct ShaderPipelines
{
    std::vector<boo::ObjToken<boo::IShaderPipeline>> m_pipelines;
};

/**
 * @brief Maintains index/data file pair containing platform-dependent cached shader data
 */
class ShaderCacheManager
{
    const FileStoreManager& m_storeMgr;
    ShaderCacheExtensions m_extensions;
    uint64_t m_extensionsHash = 0;
    std::unique_ptr<IShaderBackendFactory> m_factory;
    athena::io::FileReader m_idxFr;
    athena::io::FileReader m_datFr;
    hecl::Frontend::Frontend FE;
    struct IndexEntry : athena::io::DNA<athena::BigEndian>
    {
        DECL_DNA
        Value<atUint64> m_hash;
        Value<atUint64> m_meta;
        Value<atUint64> m_compOffset;
        Value<atUint32> m_compSize;
        Value<atUint32> m_decompSize;
    };
    std::vector<IndexEntry> m_entries;
    std::unordered_map<Hash, size_t> m_entryLookup;
    std::unordered_map<Hash, std::shared_ptr<ShaderPipelines>> m_pipelineLookup;

    uint64_t m_timeHash = 0;
    void bootstrapIndex();
    ShaderCachedData lookupData(const Hash& hash);
    bool addData(const ShaderCachedData& data);
    boo::ObjToken<boo::IShaderPipeline> buildFromCache(const ShaderCachedData& foundData,
                                                       boo::IGraphicsDataFactory::Context& ctx);
    std::vector<boo::ObjToken<boo::IShaderPipeline>> buildExtendedFromCache(const ShaderCachedData& foundData,
                                                                            boo::IGraphicsDataFactory::Context& ctx);
public:
    ShaderCacheManager(const FileStoreManager& storeMgr,
                       boo::IGraphicsDataFactory* gfxFactory,
                       ShaderCacheExtensions&& extension);
    ShaderCacheManager(const FileStoreManager& storeMgr,
                       boo::IGraphicsDataFactory* gfxFactory)
    : ShaderCacheManager(storeMgr, gfxFactory, ShaderCacheExtensions()) {}
    void reload();
    void clearCachedPipelines() { m_pipelineLookup.clear(); }

    /* Some platforms (like Metal) require information about the render target
     * for encoding the pipeline state. This must be called before building shaders */
    void setRenderTargetSamples(unsigned samps) {m_factory->m_rtHint = samps;}

    std::shared_ptr<ShaderPipelines> buildShader(const ShaderTag& tag, std::string_view source,
                                                 std::string_view diagName,
                                                 boo::IGraphicsDataFactory& factory);
    std::shared_ptr<ShaderPipelines> buildShader(const ShaderTag& tag, const hecl::Frontend::IR& ir,
                                                 std::string_view diagName,
                                                 boo::IGraphicsDataFactory& factory);
    std::shared_ptr<ShaderPipelines> buildExtendedShader(const ShaderTag& tag, std::string_view source,
                                                         std::string_view diagName,
                                                         boo::IGraphicsDataFactory& factory);
    std::shared_ptr<ShaderPipelines> buildExtendedShader(const ShaderTag& tag, const hecl::Frontend::IR& ir,
                                                         std::string_view diagName,
                                                         boo::IGraphicsDataFactory& factory);
};

/**
 * @brief Integrated reader/constructor/container for HMDL data
 */
struct HMDLData
{
    boo::ObjToken<boo::IGraphicsBufferS> m_vbo;
    boo::ObjToken<boo::IGraphicsBufferS> m_ibo;
    boo::ObjToken<boo::IVertexFormat> m_vtxFmt;

    HMDLData(boo::IGraphicsDataFactory::Context& ctx,
             const void* metaData, const void* vbo, const void* ibo);

    /* For binding constructors that require vertex format up front (GLSL) */
    static boo::ObjToken<boo::IVertexFormat>
    NewVertexFormat(boo::IGraphicsDataFactory::Context& ctx, const HMDLMeta& meta,
                    const boo::ObjToken<boo::IGraphicsBuffer>& vbo={},
                    const boo::ObjToken<boo::IGraphicsBuffer>& ibo={});

    boo::ObjToken<boo::IShaderDataBinding>
    newShaderDataBindng(boo::IGraphicsDataFactory::Context& ctx,
                        const boo::ObjToken<boo::IShaderPipeline>& shader,
                        size_t ubufCount, const boo::ObjToken<boo::IGraphicsBuffer>* ubufs,
                        const boo::PipelineStage* ubufStages,
                        size_t texCount, const boo::ObjToken<boo::ITexture>* texs)
    {return ctx.newShaderDataBinding(shader, m_vtxFmt, m_vbo.get(), nullptr, m_ibo.get(),
                                     ubufCount, ubufs, ubufStages, nullptr, nullptr,
                                     texCount, texs, nullptr, nullptr);}
};

}
}

namespace std
{
template <> struct hash<hecl::Runtime::ShaderTag>
{
    size_t operator()(const hecl::Runtime::ShaderTag& val) const NOEXCEPT
    {return val.valSizeT();}
};
}

#endif // HECLRUNTIME_HPP
