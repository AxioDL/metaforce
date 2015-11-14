#ifndef HECLRUNTIME_HPP
#define HECLRUNTIME_HPP

#include "HECL.hpp"
#include <boo/graphicsdev/IGraphicsDataFactory.hpp>
#include <Athena/DNA.hpp>
#include <Athena/FileReader.hpp>
#include <unordered_map>

namespace HECL
{
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
    const SystemString& getStoreRoot() const {return m_storeRoot;}
};

/**
 * @brief Shader formats that may be identified within ShaderHash
 */
enum ShaderFormat : uint8_t
{
    ShaderFormatNone,
    ShaderFormatGLSL,
    ShaderFormatHLSL,
    ShaderFormatMetal,
    ShaderFormatSpirV
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
            ShaderFormat m_fmt;
            uint8_t m_colorCount;
            uint8_t m_uvCount;
            uint8_t m_weightCount;
        };
    };
public:
    ShaderTag() = default;
    ShaderTag(const void* buf, size_t len, ShaderFormat fmt, uint8_t c, uint8_t u, uint8_t w)
    : Hash(buf, len), m_fmt(fmt), m_colorCount(c), m_uvCount(u), m_weightCount(w) {}
    ShaderTag(unsigned long long hashin, uint64_t meta)
    : Hash(hashin), m_meta(meta) {}
    ShaderTag(const ShaderTag& other) : Hash(other) {}
    ShaderFormat getShaderFormat() const {return m_fmt;}
    uint8_t getColorCount() const {return m_colorCount;}
    uint8_t getUvCount() const {return m_uvCount;}
    uint8_t getWeightCount() const {return m_weightCount;}
    uint64_t getMetaData() const {return m_meta;}
};

/**
 * @brief Maintains index/data file pair containing platform-dependent cached shader data
 */
class ShaderCacheManager
{
    const FileStoreManager& m_storeMgr;
    Athena::io::FileReader m_idxFr;
    Athena::io::FileReader m_datFr;
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
    void BootstrapIndex();
public:
    ShaderCacheManager(const FileStoreManager& storeMgr)
    : m_storeMgr(storeMgr),
      m_idxFr(storeMgr.getStoreRoot() + _S("/shadercache.idx")),
      m_datFr(storeMgr.getStoreRoot() + _S("/shadercache.dat"))
    {reload();}
    void reload();

    class CachedData
    {
        friend class ShaderCacheManager;
        CachedData() = default;
        CachedData(unsigned long long hashin, uint64_t meta, size_t decompSz)
        : m_tag(hashin, meta), m_data(new uint8_t[decompSz]), m_sz(decompSz) {}
    public:
        ShaderTag m_tag;
        std::unique_ptr<uint8_t[]> m_data;
        size_t m_sz;
        operator bool() const {return m_tag.operator bool();}
    };
    CachedData lookupData(const Hash& hash);
    bool addData(const ShaderTag& hash, const void* data, size_t sz);
};

/**
 * @brief Integrated reader/constructor/container for HMDL data
 */
struct HMDLData
{
    boo::IGraphicsBufferS* m_vbo;
    boo::IGraphicsBufferS* m_ibo;
    boo::IVertexFormat* m_vtxFmt;

    HMDLData(boo::IGraphicsDataFactory* factory,
             const void* metaData, const void* vbo, const void* ibo);
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
