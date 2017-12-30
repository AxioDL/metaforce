#include "hecl/Runtime.hpp"
#include <athena/FileReader.hpp>
#include <athena/FileWriter.hpp>
#include <zlib.h>
#include <algorithm>
#include <ctime>

#include "hecl/Backend/GLSL.hpp"
#include "hecl/Backend/Metal.hpp"

#undef min
#undef max

namespace hecl::Runtime
{
#if BOO_HAS_GL
IShaderBackendFactory* _NewGLSLBackendFactory();
#endif
#if _WIN32
IShaderBackendFactory* _NewHLSLBackendFactory();
#endif
#if BOO_HAS_METAL
IShaderBackendFactory* _NewMetalBackendFactory();
#endif
#if BOO_HAS_VULKAN
IShaderBackendFactory* _NewSPIRVBackendFactory();
#endif

static logvisor::Module SCM_Log("ShaderCacheManager");
static uint64_t IDX_MAGIC = SBig(uint64_t(0xDEADFEEDC001D00D));
static uint64_t DAT_MAGIC = SBig(uint64_t(0xC001D00DDEADBABE));
static uint64_t ZERO64 = 0;

static uint64_t timeHash()
{
    char buf[80];
    time_t now;
    struct tm* timeinfo;

    time(&now);
    timeinfo = localtime(&now);
    strftime(buf, 80, "%Y-%m-%dT%H:%M:%S+%H:%M", timeinfo);
    Hash tmp(buf, 80);
    return tmp.val64();
}

static void UpdateFunctionHash(XXH64_state_t* st, const ShaderCacheExtensions::Function& func)
{
    if (func.m_source)
        XXH64_update(st, func.m_source, strlen(func.m_source));
    if (func.m_entry)
        XXH64_update(st, func.m_entry, strlen(func.m_entry));
}
template<typename T>
static void UpdateFieldHash(XXH64_state_t* st, T field)
{
    XXH64_update(st, &field, sizeof(field));
}
uint64_t ShaderCacheExtensions::hashExtensions() const
{
    XXH64_state_t st;
    XXH64_reset(&st, 0);
    for (const ExtensionSlot& slot : m_extensionSlots)
    {
        UpdateFunctionHash(&st, slot.lighting);
        UpdateFunctionHash(&st, slot.post);
        UpdateFieldHash(&st, slot.srcFactor);
        UpdateFieldHash(&st, slot.dstFactor);
        UpdateFieldHash(&st, slot.depthTest);
        UpdateFieldHash(&st, slot.cullMode);
        UpdateFieldHash(&st, slot.noDepthWrite);
        UpdateFieldHash(&st, slot.noColorWrite);
        UpdateFieldHash(&st, slot.noAlphaWrite);
    }
    return XXH64_digest(&st);
}

void ShaderCacheManager::bootstrapIndex()
{
    m_timeHash = timeHash();
    m_idxFr.close();
    m_datFr.close();

#if _WIN32
    SystemString idxFilename = m_idxFr.wfilename();
#else
    SystemString idxFilename = m_idxFr.filename();
#endif

    FILE* idxFp = hecl::Fopen(idxFilename.c_str(), _S("wb"));
    if (!idxFp)
        SCM_Log.report(logvisor::Fatal, _S("unable to write shader cache index at %s"),
                       idxFilename.c_str());
    fwrite(&IDX_MAGIC, 1, 8, idxFp);
    fwrite(&m_timeHash, 1, 8, idxFp);
    fwrite(&m_extensionsHash, 1, 8, idxFp);
    fwrite(&ZERO64, 1, 8, idxFp);
    fclose(idxFp);
#if _WIN32
    SystemString datFilename = m_datFr.wfilename();
#else
    SystemString datFilename = m_datFr.filename();
#endif

    FILE* datFp = hecl::Fopen(datFilename.c_str(), _S("wb"));
    if (!datFp)
        SCM_Log.report(logvisor::Fatal, _S("unable to write shader cache data at %s"),
                       datFilename.c_str());
    fwrite(&DAT_MAGIC, 1, 8, datFp);
    fwrite(&m_timeHash, 1, 8, datFp);
    fclose(datFp);

    m_idxFr.open();
    m_datFr.open();
}

ShaderCacheManager::ShaderCacheManager(const FileStoreManager& storeMgr,
                                       boo::IGraphicsDataFactory* gfxFactory,
                                       ShaderCacheExtensions&& extension)
: m_storeMgr(storeMgr),
  m_extensions(std::move(extension)),
  m_idxFr(SystemString(storeMgr.getStoreRoot()) + _S("/shadercache") +
              gfxFactory->platformName() + _S(".idx"), 32*1024, false),
  m_datFr(SystemString(storeMgr.getStoreRoot()) + _S("/shadercache") +
              gfxFactory->platformName() + _S(".dat"), 32*1024, false)
{
    boo::IGraphicsDataFactory::Platform plat = gfxFactory->platform();
    if (m_extensions && m_extensions.m_plat != plat)
        SCM_Log.report(logvisor::Fatal, "ShaderCacheExtension backend mismatch (should be %s)",
                       gfxFactory->platformName());
    m_extensionsHash = m_extensions.hashExtensions();

    switch (plat)
    {
#if BOO_HAS_GL
    case boo::IGraphicsDataFactory::Platform::OpenGL:
        m_factory.reset(_NewGLSLBackendFactory());
        break;
#endif
#if _WIN32
    case boo::IGraphicsDataFactory::Platform::D3D11:
    case boo::IGraphicsDataFactory::Platform::D3D12:
        m_factory.reset(_NewHLSLBackendFactory());
        break;
#endif
#if BOO_HAS_METAL
    case boo::IGraphicsDataFactory::Platform::Metal:
        m_factory.reset(_NewMetalBackendFactory());
        break;
#endif
#if BOO_HAS_VULKAN
    case boo::IGraphicsDataFactory::Platform::Vulkan:
        m_factory.reset(_NewSPIRVBackendFactory());
        break;
#endif
    default:
        SCM_Log.report(logvisor::Fatal, _S("unsupported backend %s"), gfxFactory->platformName());
    }

    reload();
}

void ShaderCacheManager::reload()
{
    m_entries.clear();
    m_entryLookup.clear();
    m_timeHash = 0;

    /* Attempt to open existing index */
    m_idxFr.seek(0, athena::Begin);
    m_datFr.seek(0, athena::Begin);
    if (m_idxFr.hasError() || m_datFr.hasError())
    {
        bootstrapIndex();
        return;
    }
    else
    {
        uint64_t idxMagic;
        size_t rb = m_idxFr.readUBytesToBuf(&idxMagic, 8);
        if (rb != 8 || idxMagic != IDX_MAGIC)
        {
            bootstrapIndex();
            return;
        }

        uint64_t datMagic;
        rb = m_datFr.readUBytesToBuf(&datMagic, 8);
        if (rb != 8 || datMagic != DAT_MAGIC)
        {
            bootstrapIndex();
            return;
        }

        uint64_t idxRand, datRand;
        rb = m_idxFr.readUBytesToBuf(&idxRand, 8);
        size_t rb2 = m_datFr.readUBytesToBuf(&datRand, 8);
        if (rb != 8 || rb2 != 8 || idxRand != datRand)
        {
            bootstrapIndex();
            return;
        }
        m_timeHash = idxRand;
    }

    atUint64 extensionsHash;
    size_t rb = m_idxFr.readUBytesToBuf(&extensionsHash, 8);
    if (rb != 8 || extensionsHash != m_extensionsHash)
    {
        bootstrapIndex();
        return;
    }

    atUint64 idxCount = m_idxFr.readUint64Big();
    if (m_idxFr.position() != 32)
    {
        bootstrapIndex();
        return;
    }

    /* Read existing entries */
    if (idxCount)
    {
        m_entries.reserve(idxCount);
        m_entryLookup.reserve(idxCount);
        for (atUint64 i=0 ; i<idxCount ; ++i)
        {
            m_entries.emplace_back();
            IndexEntry& ent = m_entries.back();
            ent.read(m_idxFr);
            m_entryLookup[ent.m_hash] = m_entries.size() - 1;
        }
    }
}

ShaderCachedData ShaderCacheManager::lookupData(const Hash& hash)
{
    auto search = m_entryLookup.find(hash);
    if (search == m_entryLookup.cend())
        return {};

    const IndexEntry& ent = m_entries[search->second];
    if (ent.m_compOffset + ent.m_compSize > m_datFr.length())
    {
        SCM_Log.report(logvisor::Warning, "shader cache not long enough to read entry, might be corrupt");
        return {};
    }

    /* File-streamed decompression */
    m_datFr.seek(ent.m_compOffset, athena::Begin);
    ShaderCachedData ret(ShaderTag(ent.m_hash, ent.m_meta), ent.m_decompSize);
    uint8_t compDat[2048];
    z_stream z = {};
    inflateInit(&z);
    z.avail_out = ent.m_decompSize;
    z.next_out = ret.m_data.get();
    while (z.avail_out)
    {
        z.avail_in = std::min(size_t(2048), size_t(ent.m_compSize - z.total_in));
        m_datFr.readUBytesToBuf(compDat, z.avail_in);
        z.next_in = compDat;
        int ret = inflate(&z, Z_NO_FLUSH);
        if (ret == Z_STREAM_END)
            break;
        if (ret != Z_OK)
        {
            inflateEnd(&z);
            return {};
        }
    }
    inflateEnd(&z);
    return ret;
}

bool ShaderCacheManager::addData(const ShaderCachedData& data)
{
    m_idxFr.close();
    m_datFr.close();

    /* Perform one-shot buffer compression */
    uLong cBound = compressBound(data.m_sz);
    void* compBuf = malloc(cBound);
    if (compress((Bytef*)compBuf, &cBound, (Bytef*)data.m_data.get(), data.m_sz) != Z_OK)
        SCM_Log.report(logvisor::Fatal, "unable to deflate data");

    /* Open index for writing (non overwriting) */
    athena::io::FileWriter idxFw(m_idxFr.filename(), false);
    if (idxFw.hasError())
        SCM_Log.report(logvisor::Fatal, _S("unable to append shader cache index at %s"),
                       m_idxFr.filename().c_str());

    /* Open data for writing (non overwriting) */
    athena::io::FileWriter datFw(m_datFr.filename(), false);
    if (datFw.hasError())
        SCM_Log.report(logvisor::Fatal, _S("unable to append shader cache data at %s"),
                       m_datFr.filename().c_str());

    size_t targetOffset = 0;
    auto search = m_entryLookup.find(data.m_tag);
    if (search != m_entryLookup.cend())
    {
        /* Hash already present, attempt to replace data */
        IndexEntry& ent = m_entries[search->second];
        if (search->second == m_entries.size() - 1)
        {
            /* Replacing final entry; simply write-over */
            ent.m_meta = data.m_tag.getMetaData();
            ent.m_compSize = cBound;
            ent.m_decompSize = data.m_sz;
            targetOffset = ent.m_compOffset;
            idxFw.seek(search->second * 32 + 32);
            ent.write(idxFw);
        }
        else
        {
            /* Replacing non-final entry; write into available space */
            IndexEntry& nent = m_entries[search->second+1];
            size_t space = nent.m_compOffset - ent.m_compOffset;
            if (cBound <= space)
            {
                ent.m_meta = data.m_tag.getMetaData();
                ent.m_compSize = cBound;
                ent.m_decompSize = data.m_sz;
                targetOffset = ent.m_compOffset;
                idxFw.seek(search->second * 32 + 32);
                ent.write(idxFw);
            }
            else
            {
                /* Not enough space; null-entry and add to end */
                ent.m_hash = 0;
                ent.m_meta = 0;
                ent.m_compOffset = 0;
                ent.m_compSize = 0;
                ent.m_decompSize = 0;
                idxFw.seek(search->second * 32 + 32);
                ent.write(idxFw);
            }
        }
    }

    if (!targetOffset)
    {
        /* New index entry at end */
        idxFw.seek(24, athena::Begin);
        idxFw.writeUint64Big(m_entries.size() + 1);
        idxFw.seek(m_entries.size() * 32 + 32, athena::Begin);
        datFw.seek(0, athena::End);
        m_entryLookup[data.m_tag] = m_entries.size();
        m_entries.emplace_back();

        IndexEntry& ent = m_entries.back();
        ent.m_hash = data.m_tag.val64();
        ent.m_meta = data.m_tag.getMetaData();
        ent.m_compOffset = datFw.position();
        ent.m_compSize = cBound;
        ent.m_decompSize = data.m_sz;
        ent.write(idxFw);

        datFw.writeUBytes((atUint8*)compBuf, cBound);
    }
    else
    {
        /* Reusing index entry and data space */
        datFw.seek(targetOffset, athena::Begin);
        datFw.writeUBytes((atUint8*)compBuf, cBound);
    }

    free(compBuf);

    idxFw.close();
    datFw.close();

    m_idxFr.open();
    m_datFr.open();

    return true;
}

boo::ObjToken<boo::IShaderPipeline>
ShaderCacheManager::buildFromCache(const ShaderCachedData& foundData,
                                   boo::IGraphicsDataFactory::Context& ctx)
{
    return m_factory->buildShaderFromCache(foundData, ctx);
}

std::shared_ptr<ShaderPipelines>
ShaderCacheManager::buildShader(const ShaderTag& tag, std::string_view source,
                                std::string_view diagName,
                                boo::IGraphicsDataFactory& factory)
{
    auto search = m_pipelineLookup.find(tag);
    if (search != m_pipelineLookup.cend())
        return search->second;

    std::shared_ptr<ShaderPipelines> ret = std::make_shared<ShaderPipelines>();
    ShaderCachedData foundData = lookupData(tag);
    if (foundData)
    {
        factory.commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
        {
            SCM_Log.report(logvisor::Info, "building cached shader '%s' %016llX", diagName.data(), tag.val64());
            boo::ObjToken<boo::IShaderPipeline> build = buildFromCache(foundData, ctx);
            if (build)
            {
                ret->m_pipelines.push_back(build);
                return true;
            }
            return false;
        });

        if (ret->m_pipelines.size())
        {
            m_pipelineLookup[tag] = ret;
            return ret;
        }
        SCM_Log.report(logvisor::Warning, "invalid cache read, rebuilding shader '%s'", diagName.data());
    }

    factory.commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        hecl::Frontend::IR ir = FE.compileSource(source, diagName);
        SCM_Log.report(logvisor::Info, "building shader '%s' %016llX", diagName.data(), tag.val64());
        FE.getDiagnostics().reset(diagName);
        boo::ObjToken<boo::IShaderPipeline> build;
        addData(m_factory->buildShaderFromIR(tag, ir, FE.getDiagnostics(), ctx, build));
        ret->m_pipelines.push_back(build);
        return true;
    });
    m_pipelineLookup[tag] = ret;
    return ret;
}

std::shared_ptr<ShaderPipelines>
ShaderCacheManager::buildShader(const ShaderTag& tag, const hecl::Frontend::IR& ir,
                                std::string_view diagName,
                                boo::IGraphicsDataFactory& factory)
{
    auto search = m_pipelineLookup.find(tag);
    if (search != m_pipelineLookup.cend())
        return search->second;

    std::shared_ptr<ShaderPipelines> ret = std::make_shared<ShaderPipelines>();
    ShaderCachedData foundData = lookupData(tag);
    if (foundData)
    {
        factory.commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
        {
            SCM_Log.report(logvisor::Info, "building cached shader '%s' %016llX", diagName.data(), tag.val64());
            boo::ObjToken<boo::IShaderPipeline> build = buildFromCache(foundData, ctx);
            if (build)
            {
                ret->m_pipelines.push_back(build);
                return true;
            }
            return false;
        });

        if (ret->m_pipelines.size())
        {
            m_pipelineLookup[tag] = ret;
            return ret;
        }
        SCM_Log.report(logvisor::Warning, "invalid cache read, rebuilding shader '%s'", diagName.data());
    }

    factory.commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        SCM_Log.report(logvisor::Info, "building shader '%s' %016llX", diagName.data(), tag.val64());
        FE.getDiagnostics().reset(diagName);
        boo::ObjToken<boo::IShaderPipeline> build;
        addData(m_factory->buildShaderFromIR(tag, ir, FE.getDiagnostics(), ctx, build));
        ret->m_pipelines.push_back(build);
        return true;
    });
    m_pipelineLookup[tag] = ret;
    return ret;
}

std::vector<boo::ObjToken<boo::IShaderPipeline>>
ShaderCacheManager::buildExtendedFromCache(const ShaderCachedData& foundData,
                                           boo::IGraphicsDataFactory::Context& ctx)
{
    std::vector<boo::ObjToken<boo::IShaderPipeline>> shaders;
    shaders.reserve(m_extensions.m_extensionSlots.size());
    if (!m_factory->buildExtendedShaderFromCache(foundData, m_extensions.m_extensionSlots, ctx,
    [&](const boo::ObjToken<boo::IShaderPipeline>& shader){shaders.push_back(shader);}))
        return {};
    if (shaders.size() != m_extensions.m_extensionSlots.size())
        SCM_Log.report(logvisor::Fatal, "buildShaderFromCache returned %" PRISize " times, expected %" PRISize,
                       shaders.size(), m_extensions.m_extensionSlots.size());
    return shaders;
}

std::shared_ptr<ShaderPipelines>
ShaderCacheManager::buildExtendedShader(const ShaderTag& tag, std::string_view source,
                                        std::string_view diagName,
                                        boo::IGraphicsDataFactory& factory)
{
    auto search = m_pipelineLookup.find(tag);
    if (search != m_pipelineLookup.cend())
        return search->second;

    std::shared_ptr<ShaderPipelines> ret = std::make_shared<ShaderPipelines>();
    ShaderCachedData foundData = lookupData(tag);
    if (foundData)
    {
        factory.commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
        {
            SCM_Log.report(logvisor::Info, "building cached shader '%s' %016llX", diagName.data(), tag.val64());
            ret->m_pipelines = buildExtendedFromCache(foundData, ctx);
            return ret->m_pipelines.size() != 0;
        });

        if (ret->m_pipelines.size())
        {
            m_pipelineLookup[tag] = ret;
            return ret;
        }
        SCM_Log.report(logvisor::Warning, "invalid cache read, rebuilding shader '%s'", diagName.data());
    }

    hecl::Frontend::IR ir = FE.compileSource(source, diagName);

    factory.commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        ret->m_pipelines.reserve(m_extensions.m_extensionSlots.size());
        FE.getDiagnostics().reset(diagName);
        SCM_Log.report(logvisor::Info, "building shader '%s' %016llX", diagName.data(), tag.val64());
        ShaderCachedData data =
        m_factory->buildExtendedShaderFromIR(tag, ir, FE.getDiagnostics(), m_extensions.m_extensionSlots, ctx,
        [&](const boo::ObjToken<boo::IShaderPipeline>& shader){ret->m_pipelines.push_back(shader);});
        if (ret->m_pipelines.size() != m_extensions.m_extensionSlots.size())
            SCM_Log.report(logvisor::Fatal, "buildShaderFromIR returned %" PRISize " times, expected %" PRISize,
                           ret->m_pipelines.size(), m_extensions.m_extensionSlots.size());
        addData(data);
        return true;
    });
    m_pipelineLookup[tag] = ret;
    return ret;
}

std::shared_ptr<ShaderPipelines>
ShaderCacheManager::buildExtendedShader(const ShaderTag& tag, const hecl::Frontend::IR& ir,
                                        std::string_view diagName,
                                        boo::IGraphicsDataFactory& factory)
{
    auto search = m_pipelineLookup.find(tag);
    if (search != m_pipelineLookup.cend())
        return search->second;

    std::shared_ptr<ShaderPipelines> ret = std::make_shared<ShaderPipelines>();
    ShaderCachedData foundData = lookupData(tag);
    if (foundData)
    {
        factory.commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
        {
            SCM_Log.report(logvisor::Info, "building cached shader '%s' %016llX", diagName.data(), tag.val64());
            ret->m_pipelines = buildExtendedFromCache(foundData, ctx);
            return ret->m_pipelines.size() != 0;
        });

        if (ret->m_pipelines.size() != 0)
        {
            m_pipelineLookup[tag] = ret;
            return ret;
        }
        SCM_Log.report(logvisor::Warning, "invalid cache read, rebuilding shader '%s'", diagName.data());
    }

    factory.commitTransaction([&](boo::IGraphicsDataFactory::Context& ctx) -> bool
    {
        ret->m_pipelines.reserve(m_extensions.m_extensionSlots.size());
        FE.getDiagnostics().reset(diagName);
        SCM_Log.report(logvisor::Info, "building shader '%s' %016llX", diagName.data(), tag.val64());
        ShaderCachedData data =
        m_factory->buildExtendedShaderFromIR(tag, ir, FE.getDiagnostics(), m_extensions.m_extensionSlots, ctx,
        [&](const boo::ObjToken<boo::IShaderPipeline>& shader){ret->m_pipelines.push_back(shader);});
        if (ret->m_pipelines.size() != m_extensions.m_extensionSlots.size())
            SCM_Log.report(logvisor::Fatal, "buildShaderFromIR returned %" PRISize " times, expected %" PRISize,
                           ret->m_pipelines.size(), m_extensions.m_extensionSlots.size());
        addData(data);
        return true;
    });
    m_pipelineLookup[tag] = ret;
    return ret;
}

}
