#ifndef HECLRUNTIME_HPP
#define HECLRUNTIME_HPP

#include "HECL.hpp"
#include "Frontend.hpp"
#include <boo/graphicsdev/IGraphicsDataFactory.hpp>
#include <Athena/DNA.hpp>
#include <Athena/FileReader.hpp>
#include <unordered_map>

namespace HECL
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
    FileStoreManager(const SystemString& domain);
    const SystemString& getDomain() const {return m_domain;}
    /**
     * @brief Returns the full path to the file store, including domain
     * @return Full path to store e.g /home/foo/.hecl/bar
     */
    const SystemString& getStoreRoot() const {return m_storeRoot;}
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
            bool m_depthTest:1;
            bool m_depthWrite:1;
            bool m_backfaceCulling:1;
        };
    };
public:
    ShaderTag() = default;
    ShaderTag(const std::string& source, uint8_t c, uint8_t u, uint8_t w, uint8_t s, uint8_t t,
              bool depthTest, bool depthWrite, bool backfaceCulling)
    : Hash(source), m_colorCount(c), m_uvCount(u), m_weightCount(w), m_skinSlotCount(s), m_texMtxCount(t),
      m_depthTest(depthTest), m_depthWrite(depthWrite), m_backfaceCulling(backfaceCulling)
    {hash ^= m_meta;}
    ShaderTag(const HECL::Frontend::IR& ir, uint8_t c, uint8_t u, uint8_t w, uint8_t s, uint8_t t,
              bool depthTest, bool depthWrite, bool backfaceCulling)
    : Hash(ir.m_hash), m_colorCount(c), m_uvCount(u), m_weightCount(w), m_skinSlotCount(s), m_texMtxCount(t),
      m_depthTest(depthTest), m_depthWrite(depthWrite), m_backfaceCulling(backfaceCulling)
    {hash ^= m_meta;}
    ShaderTag(uint64_t hashin, uint8_t c, uint8_t u, uint8_t w, uint8_t s, uint8_t t,
              bool depthTest, bool depthWrite, bool backfaceCulling)
    : Hash(hashin), m_colorCount(c), m_uvCount(u), m_weightCount(w), m_skinSlotCount(s), m_texMtxCount(t),
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
    bool getDepthTest() const {return m_depthTest;}
    bool getDepthWrite() const {return m_depthWrite;}
    bool getBackfaceCulling() const {return m_backfaceCulling;}
    uint64_t getMetaData() const {return m_meta;}

    /* For shader constructors that require vertex format up-front (HLSL) */
    boo::IVertexFormat* newVertexFormat(boo::IGraphicsDataFactory* factory) const;
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
    };
    std::vector<ExtensionSlot> m_extensionSlots;

    ShaderCacheExtensions(boo::IGraphicsDataFactory::Platform plat) : m_plat(plat)
    {
        /* Index-0 has special default-meaning */
        m_extensionSlots.emplace_back();
    }
    operator bool() const {return m_plat != boo::IGraphicsDataFactory::Platform::Null;}

    /* Strings must remain resident!! (intended to be stored static const) */
    unsigned registerExtensionSlot(Function lighting, Function post)
    {
        m_extensionSlots.emplace_back();
        ExtensionSlot& slot = m_extensionSlots.back();
        slot.lighting = lighting;
        slot.post = post;
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
    using FReturnExtensionShader = std::function<void(boo::IShaderPipeline*)>;
    virtual ShaderCachedData buildShaderFromIR(const ShaderTag& tag,
                                               const HECL::Frontend::IR& ir,
                                               HECL::Frontend::Diagnostics& diag,
                                               boo::IShaderPipeline*& objOut)=0;
    virtual boo::IShaderPipeline* buildShaderFromCache(const ShaderCachedData& data)=0;
    virtual ShaderCachedData buildExtendedShaderFromIR(const ShaderTag& tag,
                                                       const HECL::Frontend::IR& ir,
                                                       HECL::Frontend::Diagnostics& diag,
                                                       const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                                       FReturnExtensionShader returnFunc)=0;
    virtual void buildExtendedShaderFromCache(const ShaderCachedData& data,
                                              const std::vector<ShaderCacheExtensions::ExtensionSlot>& extensionSlots,
                                              FReturnExtensionShader returnFunc)=0;
public:
    virtual ~IShaderBackendFactory() {}
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
    Athena::io::FileReader m_idxFr;
    Athena::io::FileReader m_datFr;
    HECL::Frontend::Frontend FE;
    struct IndexEntry : Athena::io::DNA<Athena::BigEndian>
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
    uint64_t m_timeHash = 0;
    void bootstrapIndex();
    ShaderCachedData lookupData(const Hash& hash);
    bool addData(const ShaderCachedData& data);
    boo::IShaderPipeline* buildFromCache(const ShaderCachedData& foundData);
    std::vector<boo::IShaderPipeline*> buildExtendedFromCache(const ShaderCachedData& foundData);
public:
    ShaderCacheManager(const FileStoreManager& storeMgr,
                       boo::IGraphicsDataFactory* gfxFactory,
                       ShaderCacheExtensions&& extension);
    ShaderCacheManager(const FileStoreManager& storeMgr,
                       boo::IGraphicsDataFactory* gfxFactory)
    : ShaderCacheManager(storeMgr, gfxFactory, ShaderCacheExtensions()) {}
    void reload();

    /* Some platforms (like Metal) require information about the render target
     * for encoding the pipeline state. This must be called before building shaders */
    void setRenderTargetSamples(unsigned samps) {m_factory->m_rtHint = samps;}
    
    boo::IShaderPipeline* buildShader(const ShaderTag& tag, const std::string& source,
                                      const std::string& diagName);
    boo::IShaderPipeline* buildShader(const ShaderTag& tag, const HECL::Frontend::IR& ir,
                                      const std::string& diagName);
    std::vector<boo::IShaderPipeline*> buildExtendedShader(const ShaderTag& tag, const std::string& source,
                                                           const std::string& diagName);
    std::vector<boo::IShaderPipeline*> buildExtendedShader(const ShaderTag& tag, const HECL::Frontend::IR& ir,
                                                           const std::string& diagName);
};

/**
 * @brief Integrated reader/constructor/container for HMDL data
 */
struct HMDLData
{
    boo::IGraphicsBufferS* m_vbo;
    boo::IGraphicsBufferS* m_ibo;
    boo::IVertexFormat* m_vtxFmt = nullptr;

    HMDLData(boo::IGraphicsDataFactory* factory,
             const void* metaData, const void* vbo, const void* ibo);

    /* For binding constructors that require vertex format up front (GLSL) */
    static boo::IVertexFormat* NewVertexFormat(boo::IGraphicsDataFactory* factory, const HMDLMeta& meta,
                                               boo::IGraphicsBuffer* vbo=nullptr, boo::IGraphicsBuffer* ibo=nullptr);

    boo::IShaderDataBinding* newShaderDataBindng(boo::IGraphicsDataFactory* factory,
                                                 boo::IShaderPipeline* shader,
                                                 size_t ubufCount, boo::IGraphicsBuffer** ubufs,
                                                 size_t texCount, boo::ITexture** texs)
    {return factory->newShaderDataBinding(shader, m_vtxFmt, m_vbo, nullptr, m_ibo,
                                          ubufCount, ubufs, texCount, texs);}
};

}
}

namespace std
{
template <> struct hash<HECL::Runtime::ShaderTag>
{
    size_t operator()(const HECL::Runtime::ShaderTag& val) const NOEXCEPT
    {return val.valSizeT();}
};
}

#endif // HECLRUNTIME_HPP
